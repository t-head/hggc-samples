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

// ----- Standard library facilities reached for by the helpers ----------------
#include <algorithm>   // generic algorithms used by the helpers
#include <cassert>     // assert() — preferred over <assert.h>
#include <cmath>       // floating-point primitives used in helper_math.h consumers
#include <cstdio>      // printf/fprintf for the diagnostic helpers
#include <cstdlib>     // exit codes, malloc, getenv
#include <fstream>     // file streams used by BMP writers
#include <iostream>    // std::cout / std::cerr in sample drivers
#include <string>      // std::string consumed by helper_string.h
#include <vector>      // std::vector backing for image buffers

// ----- Project-local helper modules ------------------------------------------
#include <helper_timer.h>   ///< HggcTimer wall-clock timer
#include <helper_string.h>  ///< Command-line / path parsing utilities
#include <helper_image.h>   ///< Element-wise & L2 relative error comparison

/// Exit code reserved for "test skipped / waived" outcomes.
#ifndef EXIT_SKIPPED
#  define EXIT_SKIPPED 2
#endif  // !EXIT_SKIPPED
