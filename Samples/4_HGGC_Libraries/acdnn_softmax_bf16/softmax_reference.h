/*
 * Copyright (c) 2023-2026, T-HEAD (SHANGHAI) SEMICONDUCTOR CO., LTD.
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <limits>
#include <vector>

// Host-only helper; handle BF16 bit patterns explicitly without type aliases or pointer casts.
namespace softmax_bf16 {
static_assert(sizeof(float) == sizeof(std::uint32_t) &&
              std::numeric_limits<float>::is_iec559, "IEEE 754 binary32 is required");
using Bf16 = std::uint16_t;

inline Bf16 to_bf16(float value) {
    std::uint32_t bits;
    std::memcpy(&bits, &value, sizeof(bits));
    if ((bits & 0x7f800000u) == 0x7f800000u) {
        // Keep infinities; keep the NaN sign and set the quiet bit so low bits cannot truncate into infinity.
        return static_cast<Bf16>((bits >> 16) |
                                 ((bits & 0x007fffffu) ? 0x0040u : 0u));
    }
    bits += 0x7fffu + ((bits >> 16) & 1u); // round-to-nearest, ties-to-even
    return static_cast<Bf16>(bits >> 16);
}

inline float from_bf16(Bf16 value) {
    const std::uint32_t bits = static_cast<std::uint32_t>(value) << 16;
    float result;
    std::memcpy(&result, &bits, sizeof(result));
    return result;
}

struct Shape {
    int n, c, h, w;
    std::size_t count() const { return std::size_t(n) * c * h * w; }
    std::size_t index(int batch, int channel, int row, int col) const {
        return ((std::size_t(batch) * c + channel) * h + row) * w + col;
    }
};

inline std::vector<Bf16> make_input(const Shape &s) {
    std::vector<Bf16> input(s.count());
    for (int n = 0; n < s.n; ++n)
        for (int c = 0; c < s.c; ++c)
            for (int h = 0; h < s.h; ++h)
                for (int w = 0; w < s.w; ++w) {
                    float value = -3.25f;
                    if (s.c == 17)
                        value = ((c * 13 + n * 7 + h * 3 + w) % 37 - 18) *
                                    0.21731f + (n + h + w) * 0.0137f;
                    else if (s.c == 257)
                        value = ((c * 29 + n * 11 + h * 7 + w * 3) % 257 - 128) *
                                    1.25f; // Input range [-160,160]; a direct exp overflows FP32.
                    input[s.index(n, c, h, w)] = to_bf16(value);
                }
    return input;
}

inline std::vector<double> reference(const Shape &s, const std::vector<Bf16> &input) {
    std::vector<double> output(s.count());
    for (int n = 0; n < s.n; ++n)
        for (int h = 0; h < s.h; ++h)
            for (int w = 0; w < s.w; ++w) {
                double maximum = -std::numeric_limits<double>::infinity();
                for (int c = 0; c < s.c; ++c)
                    maximum = std::max(maximum, double(from_bf16(input[s.index(n, c, h, w)])));
                double sum = 0;
                for (int c = 0; c < s.c; ++c) {
                    const auto i = s.index(n, c, h, w);
                    // The reference must use the quantized input actually passed to the library, not the pre-quantization FP32.
                    output[i] = std::exp(double(from_bf16(input[i])) - maximum);
                    sum += output[i];
                }
                for (int c = 0; c < s.c; ++c) output[s.index(n, c, h, w)] /= sum;
            }
    return output;
}

struct Metrics {
    double max_abs_error = 0, max_sum_error = 0;
    std::size_t invalid = 0, mismatches = 0;
    bool pass() const { return invalid == 0 && mismatches == 0 && max_sum_error <= 1e-2; }
};

inline Metrics validate(const Shape &s, const std::vector<Bf16> &output,
                        const std::vector<double> &expected) {
    Metrics metrics;
    if (output.size() != s.count() || expected.size() != s.count()) {
        metrics.invalid = 1;
        return metrics;
    }
    for (std::size_t i = 0; i < output.size(); ++i) {
        const double got = from_bf16(output[i]);
        if (!std::isfinite(got) || got < 0 ||
            !std::isfinite(expected[i]) || expected[i] < 0) {
            ++metrics.invalid;
            continue;
        }
        const double error = std::abs(got - expected[i]);
        metrics.max_abs_error = std::max(metrics.max_abs_error, error);
        if (error > 5e-4 + 1e-2 * std::abs(expected[i])) ++metrics.mismatches;
    }
    for (int n = 0; n < s.n; ++n)
        for (int h = 0; h < s.h; ++h)
            for (int w = 0; w < s.w; ++w) {
                double sum = 0;
                for (int c = 0; c < s.c; ++c) sum += from_bf16(output[s.index(n, c, h, w)]);
                const double error = std::isfinite(sum) ? std::abs(sum - 1) :
                                     std::numeric_limits<double>::infinity();
                metrics.max_sum_error = std::max(metrics.max_sum_error, error);
            }
    return metrics;
}
} // namespace softmax_bf16
