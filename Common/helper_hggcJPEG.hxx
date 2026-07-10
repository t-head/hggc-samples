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
#ifndef HG_JPEG_EXAMPLE
#define HG_JPEG_EXAMPLE

#include "hggc_runtime.h"
#include "hgjpeg.h"
#include "helper_hggc.h"
#include "helper_timer.h"

#include <cstdio>      // FILE* fallback paths
#include <cstdlib>     // exit / malloc style helpers
#include <cstring>     // std::memcpy / std::memset on raw pixel buffers
#include <fstream>     // std::ifstream / std::ofstream for binary I/O
#include <iostream>    // diagnostic output on parse failure
#include <sstream>     // std::ostringstream message formatting
#include <string>      // std::string for paths / error messages
#include <vector>      // std::vector<unsigned char> pixel buffer

#include <string.h>
#include <sys/time.h>

#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

// ===========================================================================
//  Internal BMP-writing scaffolding (anonymous namespace, not visible to
//  callers of this header).
// ===========================================================================
namespace {

/// Saturate-cast an `int` channel sample into the 8-bit `[0,255]` BMP range.
inline unsigned char clamp_u8(int v)
{
    if (v < 0)   return 0;
    if (v > 255) return 255;
    return static_cast<unsigned char>(v);
}

/// Emit the standard 54-byte BMP header (file + DIB) for an uncompressed,
/// bottom-up 24-bit-per-pixel image of size `width x height`.
inline void emit_bmp_header(std::FILE *out, int width, int height,
                            int padded_image_bytes)
{
    unsigned int headers[13];
    headers[0]  = padded_image_bytes + 54;  // bfSize
    headers[1]  = 0;                        // bfReserved (both halves)
    headers[2]  = 54;                       // bfOffBits
    headers[3]  = 40;                       // biSize (DIB header)
    headers[4]  = static_cast<unsigned int>(width);
    headers[5]  = static_cast<unsigned int>(height);
    headers[7]  = 0;                        // biCompression
    headers[8]  = padded_image_bytes;       // biSizeImage
    headers[9]  = 0;                        // biXPelsPerMeter
    headers[10] = 0;                        // biYPelsPerMeter
    headers[11] = 0;                        // biClrUsed
    headers[12] = 0;                        // biClrImportant

    std::fprintf(out, "BM");

    // bfSize .. biHeight ----------------------------------------------------
    for (int n = 0; n <= 5; ++n) {
        std::fprintf(out, "%c",  headers[n]                        & 0x000000FFu);
        std::fprintf(out, "%c", (headers[n] >> 8)                  & 0x000000FFu);
        std::fprintf(out, "%c", (headers[n] >> 16)                 & 0x000000FFu);
        std::fprintf(out, "%c", (headers[n] >> 24) & 0x000000FFu);
    }

    // biPlanes (1 short) + biBitCount (1 short = 24) -----------------------
    std::fprintf(out, "%c", 1);
    std::fprintf(out, "%c", 0);
    std::fprintf(out, "%c", 24);
    std::fprintf(out, "%c", 0);

    // biCompression .. biClrImportant --------------------------------------
    for (int n = 7; n <= 12; ++n) {
        std::fprintf(out, "%c",  headers[n]                        & 0x000000FFu);
        std::fprintf(out, "%c", (headers[n] >> 8)                  & 0x000000FFu);
        std::fprintf(out, "%c", (headers[n] >> 16)                 & 0x000000FFu);
        std::fprintf(out, "%c", (headers[n] >> 24) & 0x000000FFu);
    }
}

/// Compute the per-row trailing-padding byte count required by the BMP spec
/// (rows must align to 4 bytes).
inline int bmp_row_padding(int width)
{
    const int extra = 4 - ((width * 3) % 4);
    return (extra == 4) ? 0 : extra;
}

}  // anonymous namespace

// ===========================================================================
//  writeBMP -- three planar device buffers -> 24-bit BMP on disk
// ===========================================================================
inline int writeBMP(const char          *filename,
                    const unsigned char *d_chanR, int pitchR,
                    const unsigned char *d_chanG, int pitchG,
                    const unsigned char *d_chanB, int pitchB,
                    int width, int height)
{
    (void)pitchG; (void)pitchB; // upstream uses pitchR for all three copies

    // ---- pull each plane back to host into a contiguous (no-pitch) buffer
    std::vector<unsigned char> bufR(static_cast<size_t>(height) * width);
    std::vector<unsigned char> bufG(static_cast<size_t>(height) * width);
    std::vector<unsigned char> bufB(static_cast<size_t>(height) * width);

    unsigned char *chanR = bufR.data();
    unsigned char *chanG = bufG.data();
    unsigned char *chanB = bufB.data();

    checkHggcErrors(hggcMemcpy2D(chanR, (size_t)width, d_chanR, (size_t)pitchR,
                                 width, height, hggcMemcpyDeviceToHost));
    checkHggcErrors(hggcMemcpy2D(chanG, (size_t)width, d_chanG, (size_t)pitchR,
                                 width, height, hggcMemcpyDeviceToHost));
    checkHggcErrors(hggcMemcpy2D(chanB, (size_t)width, d_chanB, (size_t)pitchR,
                                 width, height, hggcMemcpyDeviceToHost));

    const int extrabytes = bmp_row_padding(width);
    const int paddedsize = ((width * 3) + extrabytes) * height;

    std::FILE *out = std::fopen(filename, "wb");
    if (!out) {
        std::cerr << "Cannot open file: " << filename << std::endl;
        return 1;
    }

    emit_bmp_header(out, width, height, paddedsize);

    // ---- pixel payload (BMP is bottom-up; channel order is BGR) ----------
    for (int y = height - 1; y >= 0; --y) {
        for (int x = 0; x < width; ++x) {
            const int idx   = y * width + x;
            const unsigned char r = clamp_u8(chanR[idx]);
            const unsigned char g = clamp_u8(chanG[idx]);
            const unsigned char b = clamp_u8(chanB[idx]);
            std::fprintf(out, "%c", b);
            std::fprintf(out, "%c", g);
            std::fprintf(out, "%c", r);
        }
        for (int n = 0; n < extrabytes; ++n) {
            std::fprintf(out, "%c", 0);
        }
    }

    std::fclose(out);
    return 0;
}

// ===========================================================================
//  writeBMPi -- interleaved RGB device buffer -> 24-bit BMP on disk
// ===========================================================================
inline int writeBMPi(const char          *filename,
                     const unsigned char *d_RGB, int pitch,
                     int width, int height)
{
    std::vector<unsigned char> rgb(static_cast<size_t>(height) * width * 3);
    unsigned char *chanRGB = rgb.data();

    checkHggcErrors(hggcMemcpy2D(chanRGB, (size_t)width * 3, d_RGB, (size_t)pitch,
                                 width * 3, height, hggcMemcpyDeviceToHost));

    const int extrabytes = bmp_row_padding(width);
    const int paddedsize = ((width * 3) + extrabytes) * height;

    std::FILE *out = std::fopen(filename, "wb");
    if (!out) {
        std::cerr << "Cannot open file: " << filename << std::endl;
        return 1;
    }

    emit_bmp_header(out, width, height, paddedsize);

    for (int y = height - 1; y >= 0; --y) {
        for (int x = 0; x < width; ++x) {
            const size_t base = (static_cast<size_t>(y) * width + x) * 3;
            const unsigned char r = clamp_u8(chanRGB[base + 0]);
            const unsigned char g = clamp_u8(chanRGB[base + 1]);
            const unsigned char b = clamp_u8(chanRGB[base + 2]);
            std::fprintf(out, "%c", b);
            std::fprintf(out, "%c", g);
            std::fprintf(out, "%c", r);
        }
        for (int n = 0; n < extrabytes; ++n) {
            std::fprintf(out, "%c", 0);
        }
    }

    std::fclose(out);
    return 0;
}

// ===========================================================================
//  Directory-walking helpers used by the multi-image regression demos.
// ===========================================================================

/// Return 1 if `pathname` exists and is a directory; 0 otherwise.
inline int inputDirExists(const char *pathname)
{
    struct stat st;
    if (::stat(pathname, &st) != 0) {
        return 0;
    }
    return (st.st_mode & S_IFDIR) ? 1 : 0;
}

/// Recursively enumerate every regular file under `sInputPath` and append the
/// full paths to `filelist`. Returns 0 on success, 1 on I/O failure.
inline int readInput(const std::string         &sInputPath,
                     std::vector<std::string>  &filelist)
{
    struct stat st;
    if (::stat(sInputPath.c_str(), &st) != 0) {
        std::cout << "Cannot find input path " << sInputPath << std::endl;
        return 1;
    }

    if (st.st_mode & S_IFREG) {
        filelist.push_back(sInputPath);
        return 0;
    }

    if (!(st.st_mode & S_IFDIR)) {
        std::cout << "Cannot open input: " << sInputPath << std::endl;
        return 1;
    }

    DIR *dh = ::opendir(sInputPath.c_str());
    if (!dh) {
        std::cout << "Cannot open input directory: " << sInputPath << std::endl;
        return 1;
    }

    for (struct dirent *ent = ::readdir(dh); ent != NULL; ent = ::readdir(dh)) {
        const std::string full = sInputPath + ent->d_name;
        if (inputDirExists(full.c_str())) {
            const std::string leaf = ent->d_name;
            if (leaf != "." && leaf != "..") {
                readInput(sInputPath + leaf + "/", filelist);
            }
        } else {
            filelist.push_back(full);
        }
    }
    ::closedir(dh);
    return 0;
}

/// Locate the `images/` directory associated with the running executable.
/// First tries `<exec_dir>/images`, then walks a small set of fixed relative
/// paths under `Samples/4_HGGC_Libraries/<exec_name>/images`.
inline int getInputDir(std::string &input_dir, const char *executable_path)
{
    if (executable_path == 0) {
        return 0;
    }

    std::string exec_name = executable_path;
    std::string exec_dir  = "./";

    const size_t slash = exec_name.find_last_of('/');
    if (slash != std::string::npos) {
        exec_dir = exec_name.substr(0, slash + 1);
        exec_name.erase(0, slash + 1);
    }

    // Preferred location: a sibling `images/` directory next to the binary.
    const std::string adjacent = exec_dir + "images";
    if (inputDirExists(adjacent.c_str())) {
        input_dir = adjacent + "/";
        return 1;
    }

    // Fallback: walk a small set of canonical sample-tree relative paths.
    static const char *const kSearchPath[] = {
        "./images",
        "../../../../Samples/4_HGGC_Libraries/<sample_name>/images",
        "../../../Samples/4_HGGC_Libraries/<sample_name>/images",
        "../../Samples/4_HGGC_Libraries/<sample_name>/images"
    };

    for (unsigned int i = 0; i < sizeof(kSearchPath) / sizeof(kSearchPath[0]); ++i) {
        std::string candidate(kSearchPath[i]);
        const size_t marker = candidate.find("<sample_name>");
        if (marker != std::string::npos) {
            candidate.replace(marker, std::strlen("<sample_name>"), exec_name);
        }
        if (inputDirExists(candidate.c_str())) {
            input_dir = candidate + "/";
            return 1;
        }
    }
    return 0;
}

#endif  // HG_JPEG_EXAMPLE
