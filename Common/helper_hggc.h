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
#ifndef COMMON_HELPER_HGGC_H_
#define COMMON_HELPER_HGGC_H_

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <helper_string.h>

#ifndef EXIT_SKIPPED
#define EXIT_SKIPPED 2
#endif

// Compact "case X: return #X;" macro used by every _hggcGetErrorEnum below.
#define HGGC_ERR_CASE(x) case x: return #x;

// ===========================================================================
//  HGGC Runtime
// ===========================================================================
#ifdef __HGGCRT_DRIVER_TYPES_H__
static const char *_hggcGetErrorEnum(hggcError_t error)
{
    return hggcGetErrorName(error);
}
#endif

// ===========================================================================
//  HGGC Driver API
// ===========================================================================
#ifdef HGGC_DRIVER_API
static const char *_hggcGetErrorEnum(HGresult error)
{
    static char unknown[] = "<unknown>";
    const char *ret = NULL;
    hgGetErrorName(error, &ret);
    return ret ? ret : unknown;
}
#endif

// ===========================================================================
//  ACBLAS
// ===========================================================================
#ifdef ACBLAS_API_H_
static const char *_hggcGetErrorEnum(acblasStatus_t error)
{
    switch (error) {
        HGGC_ERR_CASE(ACBLAS_STATUS_SUCCESS)
        HGGC_ERR_CASE(ACBLAS_STATUS_NOT_INITIALIZED)
        HGGC_ERR_CASE(ACBLAS_STATUS_ALLOC_FAILED)
        HGGC_ERR_CASE(ACBLAS_STATUS_INVALID_VALUE)
        HGGC_ERR_CASE(ACBLAS_STATUS_ARCH_MISMATCH)
        HGGC_ERR_CASE(ACBLAS_STATUS_MAPPING_ERROR)
        HGGC_ERR_CASE(ACBLAS_STATUS_EXECUTION_FAILED)
        HGGC_ERR_CASE(ACBLAS_STATUS_INTERNAL_ERROR)
        HGGC_ERR_CASE(ACBLAS_STATUS_NOT_SUPPORTED)
        HGGC_ERR_CASE(ACBLAS_STATUS_LICENSE_ERROR)
    }
    return "<unknown>";
}
#endif

// ===========================================================================
//  ACFFT
// ===========================================================================
#ifdef _ACFFT_H_
static const char *_hggcGetErrorEnum(acfftResult error)
{
    switch (error) {
        HGGC_ERR_CASE(ACFFT_SUCCESS)
        HGGC_ERR_CASE(ACFFT_INVALID_PLAN)
        HGGC_ERR_CASE(ACFFT_ALLOC_FAILED)
        HGGC_ERR_CASE(ACFFT_INVALID_TYPE)
        HGGC_ERR_CASE(ACFFT_INVALID_VALUE)
        HGGC_ERR_CASE(ACFFT_INTERNAL_ERROR)
        HGGC_ERR_CASE(ACFFT_EXEC_FAILED)
        HGGC_ERR_CASE(ACFFT_SETUP_FAILED)
        HGGC_ERR_CASE(ACFFT_INVALID_SIZE)
        HGGC_ERR_CASE(ACFFT_UNALIGNED_DATA)
        HGGC_ERR_CASE(ACFFT_INCOMPLETE_PARAMETER_LIST)
        HGGC_ERR_CASE(ACFFT_INVALID_DEVICE)
        HGGC_ERR_CASE(ACFFT_PARSE_ERROR)
        HGGC_ERR_CASE(ACFFT_NO_WORKSPACE)
        HGGC_ERR_CASE(ACFFT_NOT_IMPLEMENTED)
        HGGC_ERR_CASE(ACFFT_LICENSE_ERROR)
        HGGC_ERR_CASE(ACFFT_NOT_SUPPORTED)
    }
    return "<unknown>";
}
#endif

// ===========================================================================
//  ACSOLVER
// ===========================================================================
#ifdef ACSOLVER_COMMON_H_
static const char *_hggcGetErrorEnum(acsolverStatus_t error)
{
    switch (error) {
        HGGC_ERR_CASE(ACSOLVER_STATUS_SUCCESS)
        HGGC_ERR_CASE(ACSOLVER_STATUS_NOT_INITIALIZED)
        HGGC_ERR_CASE(ACSOLVER_STATUS_ALLOC_FAILED)
        HGGC_ERR_CASE(ACSOLVER_STATUS_INVALID_VALUE)
        HGGC_ERR_CASE(ACSOLVER_STATUS_ARCH_MISMATCH)
        HGGC_ERR_CASE(ACSOLVER_STATUS_MAPPING_ERROR)
        HGGC_ERR_CASE(ACSOLVER_STATUS_EXECUTION_FAILED)
        HGGC_ERR_CASE(ACSOLVER_STATUS_INTERNAL_ERROR)
        HGGC_ERR_CASE(ACSOLVER_STATUS_MATRIX_TYPE_NOT_SUPPORTED)
        // Upstream returns a trailing space on this one -- preserved verbatim.
        case ACSOLVER_STATUS_NOT_SUPPORTED: return "ACSOLVER_STATUS_NOT_SUPPORTED ";
        HGGC_ERR_CASE(ACSOLVER_STATUS_ZERO_PIVOT)
        HGGC_ERR_CASE(ACSOLVER_STATUS_INVALID_LICENSE)
    }
    return "<unknown>";
}
#endif

// ===========================================================================
//  hgJPEG
// ===========================================================================
#ifdef HGJPEGAPI
static const char *_hggcGetErrorEnum(hgjpegStatus_t error)
{
    switch (error) {
        HGGC_ERR_CASE(HGJPEG_STATUS_SUCCESS)
        HGGC_ERR_CASE(HGJPEG_STATUS_NOT_INITIALIZED)
        HGGC_ERR_CASE(HGJPEG_STATUS_INVALID_PARAMETER)
        HGGC_ERR_CASE(HGJPEG_STATUS_BAD_JPEG)
        HGGC_ERR_CASE(HGJPEG_STATUS_JPEG_NOT_SUPPORTED)
        HGGC_ERR_CASE(HGJPEG_STATUS_ALLOCATOR_FAILURE)
        HGGC_ERR_CASE(HGJPEG_STATUS_EXECUTION_FAILED)
        HGGC_ERR_CASE(HGJPEG_STATUS_ARCH_MISMATCH)
        HGGC_ERR_CASE(HGJPEG_STATUS_INTERNAL_ERROR)
    }
    return "<unknown>";
}
#endif

// Stop polluting the including TU with our internal macro.
#undef HGGC_ERR_CASE

// ===========================================================================
//  Generic status checker -- abort the process with a useful diagnostic when
//  a non-success status is returned from any of the libraries above.
// ===========================================================================
template <typename T>
void check(T result, char const *const func,
           const char *const file, int const line)
{
    if (!result) {
        return;
    }
    std::fprintf(stderr,
                 "HGGC error at %s:%d code=%d \"%s\" \n",
                 file, line,
                 static_cast<unsigned int>(result),
                 func);
    std::exit(EXIT_FAILURE);
}

#ifdef __HGGCRT_DRIVER_TYPES_H__

#define checkHggcErrors(val)    check((val), #val, __FILE__, __LINE__)

#define getLastHggcError(msg)   __getLastHggcError((msg), __FILE__, __LINE__)

/// Pull the most recent runtime error, log it, and abort on failure.
inline void __getLastHggcError(const char *errorMessage,
                               const char *file, const int line)
{
    hggcError_t err = hggcGetLastError();
    if (err == hggcSuccess) {
        return;
    }
    std::fprintf(stderr,
                 "%s(%i) : getLastHggcError() HGGC error :"
                 " %s : (%d) %s.\n",
                 file, line, errorMessage,
                 static_cast<int>(err),
                 hggcGetErrorString(err));
    std::exit(EXIT_FAILURE);
}
#endif  // __HGGCRT_DRIVER_TYPES_H__

#ifndef MAX
#define MAX(a, b) (a > b ? a : b)
#endif

// ===========================================================================
//  PPU architecture lookup tables (Arch ID = 0xMm where M=major, m=minor)
// ===========================================================================

/// Number of HGGC cores per compute unit for a given (major, minor) architecture.
inline int ppuCoresPerUnit(int major, int minor)
{
    typedef struct {
        int ArchID;
        int Cores;
    } sArchToCores;

    sArchToCores nPpuArchCoresPerUnit[] = {
        {0x80,  64},
        {0x89, 128},
        {-1,   -1}
    };

    int index = 0;
    while (nPpuArchCoresPerUnit[index].ArchID != -1) {
        if (nPpuArchCoresPerUnit[index].ArchID == ((major << 4) + minor)) {
            return nPpuArchCoresPerUnit[index].Cores;
        }
        ++index;
    }

    std::printf(
        "Map to cores for version %d.%d is undefined."
        "  Default to use %d cores per unit\n",
        major, minor, nPpuArchCoresPerUnit[index - 1].Cores);
    return nPpuArchCoresPerUnit[index - 1].Cores;
}

// ===========================================================================
//  Runtime-API device-pick helpers.
//  Only available once the runtime header has been pulled in.
// ===========================================================================
#ifdef ___HGGC_RUNTIME_H___

/// Bind the calling thread to device `devID` after validating it.
inline int ppuDeviceInit(int devID)
{
    int device_count = 0;
    checkHggcErrors(hggcGetDeviceCount(&device_count));

    if (device_count == 0) {
        std::fprintf(stderr,
                     "ppuDeviceInit() HGGC error: "
                     "no devices supporting HGGC.\n");
        std::exit(EXIT_FAILURE);
    }

    if (devID < 0) {
        devID = 0;
    }

    if (devID > device_count - 1) {
        std::fprintf(stderr, "\n");
        std::fprintf(stderr, ">> %d HGGC capable PPU device(s) detected. <<\n", device_count);
        std::fprintf(stderr,
                     ">> ppuDeviceInit (-device=%d) is not a valid"
                     " PPU device. <<\n",
                     devID);
        std::fprintf(stderr, "\n");
        return -devID;
    }

    int computeMode = -1;
    int major       =  0;
    int minor       =  0;
    checkHggcErrors(hggcDeviceGetAttribute(&computeMode, hggcDevAttrComputeMode, devID));
    checkHggcErrors(hggcDeviceGetAttribute(&major,       hggcDevAttrComputeCapabilityMajor, devID));
    checkHggcErrors(hggcDeviceGetAttribute(&minor,       hggcDevAttrComputeCapabilityMinor, devID));

    if (computeMode == hggcComputeModeProhibited) {
        std::fprintf(stderr,
                     "Error: device is running in <Compute Mode "
                     "Prohibited>, no threads can use hggcSetDevice().\n");
        return -1;
    }

    if (major < 1) {
        std::fprintf(stderr, "ppuDeviceInit(): PPU device does not support HGGC.\n");
        std::exit(EXIT_FAILURE);
    }

    checkHggcErrors(hggcSetDevice(devID));
    std::printf("ppuDeviceInit() HGGC Device [%d]\n", devID);
    return devID;
}

/// Walk every visible device and return the one with the highest
/// (#compute-units x cores/unit x clock) figure-of-merit.
inline int ppuGetMaxPerfDeviceId()
{
    int device_count       = 0;
    int devices_prohibited = 0;
    checkHggcErrors(hggcGetDeviceCount(&device_count));

    if (device_count == 0) {
        std::fprintf(stderr,
                     "ppuGetMaxPerfDeviceId() HGGC error:"
                     " no devices supporting HGGC.\n");
        std::exit(EXIT_FAILURE);
    }

    int      max_perf_device = 0;
    uint64_t max_compute_perf = 0;

    for (int current_device = 0; current_device < device_count; ++current_device) {
        int computeMode = -1;
        int major       =  0;
        int minor       =  0;

        checkHggcErrors(hggcDeviceGetAttribute(&computeMode, hggcDevAttrComputeMode, current_device));
        checkHggcErrors(hggcDeviceGetAttribute(&major,       hggcDevAttrComputeCapabilityMajor, current_device));
        checkHggcErrors(hggcDeviceGetAttribute(&minor,       hggcDevAttrComputeCapabilityMinor, current_device));

        if (computeMode == hggcComputeModeProhibited) {
            ++devices_prohibited;
            continue;
        }

        const int cores_per_unit = (major == 9999 && minor == 9999)
                                       ? 1
                                       : ppuCoresPerUnit(major, minor);

        int computeUnitCount = 0;
        int clockRate        = 0;
        checkHggcErrors(hggcDeviceGetAttribute(&computeUnitCount,
                        hggcDevAttrMultiProcessorCount, current_device));

        const hggcError_t rc = hggcDeviceGetAttribute(&clockRate,
                                                      hggcDevAttrClockRate,
                                                      current_device);
        if (rc != hggcSuccess) {
            if (rc == hggcErrorInvalidValue) {
                // Some embedded devices don't expose a clock-rate attribute.
                // Pretend they tick at 1 to keep them rankable by core-count.
                clockRate = 1;
            } else {
                std::fprintf(stderr, "HGGC error at %s:%d code=%d(%s) \n",
                             __FILE__, __LINE__,
                             static_cast<unsigned int>(rc),
                             _hggcGetErrorEnum(rc));
                std::exit(EXIT_FAILURE);
            }
        }

        const uint64_t perf = static_cast<uint64_t>(computeUnitCount)
                            * cores_per_unit * clockRate;
        if (perf > max_compute_perf) {
            max_compute_perf = perf;
            max_perf_device  = current_device;
        }
    }

    if (devices_prohibited == device_count) {
        std::fprintf(stderr,
                     "ppuGetMaxPerfDeviceId() HGGC error:"
                     " all devices have compute mode prohibited.\n");
        std::exit(EXIT_FAILURE);
    }

    return max_perf_device;
}

/// `-device=N` honoured if present; otherwise pick the best PPU.
inline int findHggcDevice(int argc, const char **argv)
{
    int devID = 0;

    if (hasArg(argc, argv, "device")) {
        devID = getArgInt(argc, argv, "device=");

        if (devID < 0) {
            std::printf("Invalid command line parameter\n ");
            std::exit(EXIT_FAILURE);
        }
        devID = ppuDeviceInit(devID);
        if (devID < 0) {
            std::printf("exiting...\n");
            std::exit(EXIT_FAILURE);
        }
    } else {
        devID = ppuGetMaxPerfDeviceId();
        checkHggcErrors(hggcSetDevice(devID));
        std::printf("PPU Device %d\n\n", devID);
    }
    return devID;
}

#endif  // ___HGGC_RUNTIME_H___

#endif  // COMMON_HELPER_HGGC_H_
