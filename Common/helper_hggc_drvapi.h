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
#ifndef COMMON_HELPER_HGGC_DRVAPI_H_
#define COMMON_HELPER_HGGC_DRVAPI_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <cstring>
#include <iostream>
#include <sstream>

#include <helper_string.h>
#include <helper_hggc.h>

#ifndef MAX
#define MAX(a, b) (a > b ? a : b)
#endif

#ifndef EXIT_SKIPPED
#define EXIT_SKIPPED 2
#endif

// ---------------------------------------------------------------------------
//  Driver-API specific block.
//  Only emitted once the Driver-API public header has been pulled in (the
//  `__hggc_hggc_h__` token is defined by `<hggc.h>`).
// ===========================================================================
#ifdef __hggc_hggc_h__

#ifndef checkHggcErrors
#define checkHggcErrors(err) __checkHggcErrors((err), __FILE__, __LINE__)

/// Print a Driver-API error and abort on non-success status.
inline void __checkHggcErrors(HGresult err, const char *file, const int line)
{
    if (err == HGGC_SUCCESS) {
        return;
    }
    const char *msg = NULL;
    hgGetErrorString(err, &msg);
    std::fprintf(stderr,
                 "checkHggcErrors() Driver API error = %04d \"%s\" from file <%s>, "
                 "line %i.\n",
                 err, msg, file, line);
    std::exit(EXIT_FAILURE);
}
#endif  // checkHggcErrors

/// Templated wrapper around `hgDeviceGetAttribute()` for type-safe queries.
template <class T>
inline void getHggcAttribute(T *attribute,
                             HGdevice_attribute device_attribute,
                             int device)
{
    checkHggcErrors(hgDeviceGetAttribute(attribute, device_attribute, device));
}

#endif  // __hggc_hggc_h__

// ===========================================================================
//  Device-discovery helpers (Driver-API).
//  These need the Driver-API public header, hence the guard.
// ===========================================================================
#ifdef __hggc_hggc_h__

/// Initialise the PPU runtime, pick the device the user asked for (`-device=N`)
/// or device 0, and return its index. Returns negative on validation failure.
inline int ppuDeviceInitDRV(int ARGC, const char **ARGV)
{
    int hgDevice    = 0;
    int deviceCount = 0;
    checkHggcErrors(hgInit(0));
    checkHggcErrors(hgDeviceGetCount(&deviceCount));

    if (deviceCount == 0) {
        std::fprintf(stderr, "hggcDeviceInit error: no devices supporting HGGC\n");
        std::exit(EXIT_FAILURE);
    }

    int dev = getArgInt(ARGC, ARGV, "device=");
    if (dev < 0) {
        dev = 0;
    }

    if (dev > deviceCount - 1) {
        std::fprintf(stderr, "\n");
        std::fprintf(stderr, ">> %d HGGC capable PPU device(s) detected. <<\n", deviceCount);
        std::fprintf(stderr,
                     ">> hggcDeviceInit (-device=%d) is not a valid PPU device. <<\n",
                     dev);
        std::fprintf(stderr, "\n");
        return -dev;
    }

    checkHggcErrors(hgDeviceGet(&hgDevice, dev));
    char name[100];
    checkHggcErrors(hgDeviceGetName(name, 100, hgDevice));

    int computeMode;
    getHggcAttribute<int>(&computeMode, HG_DEVICE_ATTRIBUTE_COMPUTE_MODE, dev);
    if (computeMode == HG_COMPUTEMODE_PROHIBITED) {
        std::fprintf(stderr,
                     "Error: device is running in <HG_COMPUTEMODE_PROHIBITED>, no "
                     "threads can use this HGGC Device.\n");
        return -1;
    }

    if (!hasArg(ARGC, ARGV, "quiet")) {
        std::printf("ppuDeviceInitDRV() Using HGGC Device [%d]: %s\n", dev, name);
    }
    return dev;
}

/// Walk every visible device and return the index of the one with the highest
/// (#compute-units x cores/unit x clock) figure-of-merit.
inline int ppuGetMaxPerfDeviceIdDRV()
{
    int device_count        = 0;
    int devices_prohibited  = 0;

    hgInit(0);
    checkHggcErrors(hgDeviceGetCount(&device_count));

    if (device_count == 0) {
        std::fprintf(stderr,
                     "ppuGetMaxPerfDeviceIdDRV error: no devices supporting HGGC\n");
        std::exit(EXIT_FAILURE);
    }

    HGdevice           max_perf_device   = 0;
    unsigned long long max_compute_perf  = 0;

    for (HGdevice current_device = 0; current_device < device_count; ++current_device) {
        int computeUnitCount = 0;
        int clockRate        = 0;
        int major            = 0;
        int minor            = 0;
        int computeMode      = 0;

        checkHggcErrors(hgDeviceGetAttribute(&computeUnitCount,
                        HG_DEVICE_ATTRIBUTE_MULTIPROCESSOR_COUNT, current_device));
        checkHggcErrors(hgDeviceGetAttribute(&clockRate,
                        HG_DEVICE_ATTRIBUTE_CLOCK_RATE, current_device));
        checkHggcErrors(hgDeviceGetAttribute(&major,
                        HG_DEVICE_ATTRIBUTE_COMPUTE_CAPABILITY_MAJOR, current_device));
        checkHggcErrors(hgDeviceGetAttribute(&minor,
                        HG_DEVICE_ATTRIBUTE_COMPUTE_CAPABILITY_MINOR, current_device));

        getHggcAttribute<int>(&computeMode, HG_DEVICE_ATTRIBUTE_COMPUTE_MODE, current_device);

        if (computeMode == HG_COMPUTEMODE_PROHIBITED) {
            ++devices_prohibited;
            continue;
        }

        const int cores_per_unit = (major == 9999 && minor == 9999)
                                       ? 1
                                       : ppuCoresPerUnit(major, minor);

        const unsigned long long perf =
            static_cast<unsigned long long>(computeUnitCount) * cores_per_unit * clockRate;

        if (perf > max_compute_perf) {
            max_compute_perf = perf;
            max_perf_device  = current_device;
        }
    }

    if (devices_prohibited == device_count) {
        std::fprintf(stderr,
                     "ppuGetMaxPerfDeviceIdDRV error: all devices have compute mode "
                     "prohibited.\n");
        std::exit(EXIT_FAILURE);
    }

    return max_perf_device;
}

/// Combined entry point: honour `-device=N` if present, otherwise pick the PPU
/// with the highest figure-of-merit.
inline HGdevice findHggcDeviceDRV(int argc, const char **argv)
{
    HGdevice hgDevice;
    int      devID = 0;

    if (hasArg(argc, argv, "device")) {
        devID = ppuDeviceInitDRV(argc, argv);
        if (devID < 0) {
            std::printf("exiting...\n");
            std::exit(EXIT_SUCCESS);
        }
    } else {
        char name[100];
        devID = ppuGetMaxPerfDeviceIdDRV();
        checkHggcErrors(hgDeviceGet(&hgDevice, devID));
        hgDeviceGetName(name, 100, hgDevice);
        std::printf("> Using HGGC Device [%d]: %s\n", devID, name);
    }

    hgDeviceGet(&hgDevice, devID);
    return hgDevice;
}

#endif  // __hggc_hggc_h__

// ===========================================================================
//  Locate a fatbin / module file via the sample search path, and, when the
//  located path ends in `fatbin`, slurp its contents into `ostrm`.
// ===========================================================================
inline bool findFatbinPath(const char         *module_file,
                           std::string        &module_path,
                           char              **argv,
                           std::ostringstream &ostrm)
{
    char *located = findSampleAsset(module_file, argv[0]);

    if (!located) {
        std::printf("> findModulePath file not found: <%s> \n", module_file);
        return false;
    }
    module_path = located;

    if (module_path.empty()) {
        std::printf("> findModulePath could not find file: <%s> \n", module_file);
        return false;
    }

    std::printf("> findModulePath found file at <%s>\n", module_path.c_str());

    if (module_path.rfind("fatbin") != std::string::npos) {
        std::ifstream fileIn(module_path.c_str(), std::ios::binary);
        ostrm << fileIn.rdbuf();
        fileIn.close();
    }
    return true;
}

#endif  // COMMON_HELPER_HGGC_DRVAPI_H_
