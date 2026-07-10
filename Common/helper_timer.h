/*
 * Copyright (c) 2023-2026, T-HEAD (SHANGHAI) SEMICONDUCTOR CO., LTD.
 * All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Sample code demonstrating T-HEAD SAIL SDK usage. This code is provided
 * under the Apache License 2.0 for reference and educational purposes.
 *
 * Lightweight wall-clock timer for host-side performance measurement.
 *
 * Usage:
 *     HggcTimer timer;
 *     timer.start();
 *     // ... work ...
 *     timer.stop();
 *     printf("elapsed: %.3f ms\n", timer.elapsed());
 */
#pragma once

#include <chrono>

// =============================================================================
//  HggcTimer — a simple, non-virtual wall-clock timer backed by steady_clock.
// =============================================================================
class HggcTimer {
 public:
    HggcTimer() = default;

    /// Begin (or resume) a timing session.
    void start() {
        anchor_     = std::chrono::steady_clock::now();
        is_running_ = true;
    }

    /// End the current session, accumulate elapsed time.
    void stop() {
        last_ms_     = running_ms();
        total_ms_   += last_ms_;
        is_running_  = false;
        ++sessions_;
    }

    /// Discard all accumulated time. If the timer is running, restart the
    /// anchor without stopping it.
    void reset() {
        last_ms_  = 0.0f;
        total_ms_ = 0.0f;
        sessions_ = 0;
        if (is_running_) {
            anchor_ = std::chrono::steady_clock::now();
        }
    }

    /// Total accumulated time (ms). If running, includes the current in-flight
    /// slice up to this moment.
    float elapsed() const {
        float ms = total_ms_;
        if (is_running_) ms += running_ms();
        return ms;
    }

    /// Average time (ms) per completed start/stop session, or 0 if none.
    float average() const {
        return (sessions_ > 0) ? (total_ms_ / static_cast<float>(sessions_))
                               : 0.0f;
    }

    /// Duration (ms) of the most recently completed session.
    float last_session() const { return last_ms_; }

    bool is_running() const { return is_running_; }

 private:
    float running_ms() const {
        return std::chrono::duration_cast<
                   std::chrono::duration<float, std::milli>>(
                   std::chrono::steady_clock::now() - anchor_).count();
    }

    std::chrono::steady_clock::time_point anchor_{};
    float last_ms_    = 0.0f;
    float total_ms_   = 0.0f;
    int   sessions_   = 0;
    bool  is_running_ = false;
};
