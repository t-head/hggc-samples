/*
 * Copyright (c) 2023-2026, T-HEAD (SHANGHAI) SEMICONDUCTOR CO., LTD.
 * All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Sample code demonstrating T-HEAD SAIL SDK usage. This code is provided
 * under the Apache License 2.0 for reference and educational purposes.
 */
#pragma once

#include <cassert>
#include <cmath>
#include <cstdio>
#include <iostream>

// ===========================================================================
//  Element-wise array comparison routine.
// ===========================================================================
template <class T, class S>
inline bool compareData(const T *reference,
                        const T *data,
                        const unsigned int len,
                        const S epsilon,
                        const float threshold) {
    assert(epsilon >= 0);

    unsigned int mismatches = 0;
    bool exact = true;
    for (unsigned int i = 0; i < len; ++i) {
        const float diff = static_cast<float>(reference[i]) -
                           static_cast<float>(data[i]);
        const bool within = (diff <= static_cast<float>(epsilon)) &&
                            (diff >= -static_cast<float>(epsilon));
        if (!within) {
            ++mismatches;
            exact = false;
        }
    }

    if (threshold == 0.0f) {
        return exact;
    }
    if (mismatches != 0) {
        std::printf("%4.2f(%%) of bytes mismatched (count=%d)\n",
                    static_cast<float>(mismatches) * 100.0f /
                        static_cast<float>(len),
                    mismatches);
    }
    return (static_cast<float>(len) * threshold) >
           static_cast<float>(mismatches);
}

// ===========================================================================
//  L2 relative error comparison.
// ===========================================================================
inline bool compareL2RelError(const float *reference,
                           const float *data,
                           const unsigned int len,
                           const float epsilon) {
    assert(epsilon >= 0);

    float sq_err = 0.0f;
    float sq_ref = 0.0f;
    for (unsigned int i = 0; i < len; ++i) {
        const float d = reference[i] - data[i];
        sq_err += d * d;
        sq_ref += reference[i] * reference[i];
    }

    if (std::fabs(sq_ref) < 1e-7f) {
#ifdef _DEBUG
        std::cerr << "ERROR, reference l2-norm is 0\n";
#endif
        return false;
    }

    const float relative = std::sqrt(sq_err) / std::sqrt(sq_ref);
    const bool ok = relative < epsilon;
#ifdef _DEBUG
    if (!ok) {
        std::cerr << "ERROR, l2-norm error " << relative
                  << " is greater than epsilon " << epsilon << "\n";
    }
#endif
    return ok;
}
