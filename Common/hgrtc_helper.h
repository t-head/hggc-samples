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
#ifndef COMMON_HGRTC_HELPER_H_
#define COMMON_HGRTC_HELPER_H_ 1

#include <hggc.h>
#include <helper_hggc_drvapi.h>
#include <hgrtc.h>

#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

// ---------------------------------------------------------------------------
//  Error checking macro -- aborts the process on the first HGRTC failure.
// ---------------------------------------------------------------------------
#define HGRTC_CHECK(Name, x)                                              \
    do {                                                                       \
        hgrtcResult _hgrtc_rc = (x);                                          \
        if (_hgrtc_rc != HGRTC_SUCCESS) {                                     \
            std::cerr << "\nerror: " << (Name) << " failed with error "        \
                      << hgrtcGetErrorString(_hgrtc_rc);                       \
            std::exit(1);                                                      \
        }                                                                       \
    } while (0)

// Map (major, minor) -> hgrtc --ppu-arch=<name>
inline const char *hgPpuArchName(int major, int minor)
{
    const int cc = major * 10 + minor;
    switch (cc) {
        case 80: return "ppu001";
        case 89: return "ppu0015";
        default: return "ppu001";
    }
}

// ===========================================================================
//  compileFileToHGBIN
//   Reads the kernel source from disk, queries the best HGGC device for the
//   target architecture, drives HGRTC to compile it, copies out the resulting
//   bytecode and dumps any diagnostic the compiler emitted.
// ===========================================================================
inline void compileFileToHGBIN(char *filename,
                               int    argc,
                               char **argv,
                               char **hgbinResult,
                               size_t *hgbinResultSize,
                               int    requiresCGheaders)
{
    if (!filename) {
        std::cerr << "\nerror: filename is empty for compileFileToHGBIN()!\n";
        std::exit(1);
    }

    // ---- slurp source file into a heap buffer ------------------------------
    std::ifstream src(filename, std::ios::in | std::ios::binary | std::ios::ate);
    if (!src.is_open()) {
        std::cerr << "\nerror: unable to open " << filename << " for reading!\n";
        std::exit(1);
    }

    const size_t src_size = static_cast<size_t>(src.tellg());
    char *src_buf = new char[src_size + 1];
    src.seekg(0, std::ios::beg);
    src.read(src_buf, src_size);
    src.close();
    src_buf[src_size] = '\0';

    // ---- query device capability so we can pick the matching PPU arch -----
    HGdevice hgDevice = findHggcDeviceDRV(argc, const_cast<const char **>(argv));

    int major = 0;
    int minor = 0;
    checkHggcErrors(hgDeviceGetAttribute(&major,
                    HG_DEVICE_ATTRIBUTE_COMPUTE_CAPABILITY_MAJOR, hgDevice));
    checkHggcErrors(hgDeviceGetAttribute(&minor,
                    HG_DEVICE_ATTRIBUTE_COMPUTE_CAPABILITY_MINOR, hgDevice));

    const char *archName = hgPpuArchName(major, minor);

    // ---- build the compile-option vector -----------------------------------
    char *compileParams[2];
    int   numCompileOptions = 0;

    {
        char archOpt[32];
        const int n = std::snprintf(archOpt, sizeof(archOpt),
                                    "--ppu-arch=%s", archName);
        compileParams[numCompileOptions] = reinterpret_cast<char *>(
            std::malloc(static_cast<size_t>(n) + 1));
        std::memcpy(compileParams[numCompileOptions], archOpt,
                    static_cast<size_t>(n) + 1);
        ++numCompileOptions;
    }

    if (requiresCGheaders) {
        char headerName[256];
        std::snprintf(headerName, sizeof(headerName), "%s", "cooperative_groups.h");

        char *located = findSampleAsset(headerName, argv[0]);
        if (!located) {
            std::cerr << "\nerror: header file " << headerName << " not found!\n";
            std::exit(1);
        }

        std::string include_path = located;
        if (!include_path.empty()) {
            const std::size_t hit = include_path.find(headerName);
            include_path.erase(hit);
        } else {
            std::printf(
                "\nCooperativeGroups headers not found, please install it in %s "
                "sample directory..\n Exiting..\n",
                argv[0]);
            std::exit(1);
        }

        std::string opt = "--include-path=";
        opt += include_path;
        compileParams[numCompileOptions] = reinterpret_cast<char *>(
            std::malloc(sizeof(char) * (opt.length() + 1)));
        std::snprintf(compileParams[numCompileOptions], opt.size(),
                      "%s", opt.c_str());
        ++numCompileOptions;
    }

    // ---- run the compile ---------------------------------------------------
    hgrtcProgram prog;
    HGRTC_CHECK("hgrtcCreateProgram",
                    hgrtcCreateProgram(&prog, src_buf, filename, 0, NULL, NULL));

    hgrtcResult compile_rc = hgrtcCompileProgram(prog, numCompileOptions, compileParams);

    // ---- dump diagnostic log (always, regardless of success/failure) -------
    size_t log_bytes = 0;
    HGRTC_CHECK("hgrtcGetProgramLogSize",
                    hgrtcGetProgramLogSize(prog, &log_bytes));
    char *log = reinterpret_cast<char *>(std::malloc(sizeof(char) * log_bytes + 1));
    HGRTC_CHECK("hgrtcGetProgramLog", hgrtcGetProgramLog(prog, log));
    log[log_bytes] = '\0';

    if (std::strlen(log) >= 2) {
        std::cerr << "\n compilation log ---\n";
        std::cerr << log;
        std::cerr << "\n end log ---\n";
    }
    std::free(log);

    HGRTC_CHECK("hgrtcCompileProgram", compile_rc);

    // ---- export the bytecode to the caller ---------------------------------
    size_t code_bytes = 0;
    HGRTC_CHECK("hgrtcGetHGBINSize", hgrtcGetHGBINSize(prog, &code_bytes));
    char *code = new char[code_bytes];
    HGRTC_CHECK("hgrtcGetHGBIN",     hgrtcGetHGBIN(prog, code));
    *hgbinResult     = code;
    *hgbinResultSize = code_bytes;

    for (int i = 0; i < numCompileOptions; ++i) {
        std::free(compileParams[i]);
    }
}

// ===========================================================================
//  loadHGBIN
//   Initialises a HGGC context on the best device and loads a previously
//   compiled HGBIN blob as a module. Frees `hgbin` once loaded.
// ===========================================================================
inline HGmodule loadHGBIN(char *hgbin, int argc, char **argv)
{
    HGdevice hgDevice = findHggcDeviceDRV(argc, const_cast<const char **>(argv));

    int major = 0;
    int minor = 0;
    checkHggcErrors(hgDeviceGetAttribute(&major,
                    HG_DEVICE_ATTRIBUTE_COMPUTE_CAPABILITY_MAJOR, hgDevice));
    checkHggcErrors(hgDeviceGetAttribute(&minor,
                    HG_DEVICE_ATTRIBUTE_COMPUTE_CAPABILITY_MINOR, hgDevice));

    char deviceName[256];
    checkHggcErrors(hgDeviceGetName(deviceName, 256, hgDevice));
    std::printf("> PPU Device: %s\n", deviceName);

    HGcontext context;
    checkHggcErrors(hgInit(0));
#if HGGC_VERSION >= 13000
    checkHggcErrors(hgCtxCreate(&context, NULL, 0, hgDevice));
#else
    checkHggcErrors(hgCtxCreate(&context, 0, hgDevice));
#endif

    HGmodule module;
    checkHggcErrors(hgModuleLoadData(&module, hgbin));
    std::free(hgbin);
    return module;
}

#endif  // COMMON_HGRTC_HELPER_H_
