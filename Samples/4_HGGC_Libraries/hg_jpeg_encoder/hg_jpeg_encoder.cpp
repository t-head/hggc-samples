/*
 * Copyright (c) 2023-2026, T-HEAD (SHANGHAI) SEMICONDUCTOR CO., LTD.
 * All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Sample code demonstrating T-HEAD SAIL SDK usage. This code is provided
 * under the Apache License 2.0 for reference and educational purposes.
 *
 * hg_jpeg_encoder -- JPEG Transcoder with Quality-Size Analysis
 *
 * Decodes JPEG images and re-encodes them at multiple quality levels to
 * demonstrate the quality-size tradeoff. For each image, reports encoded
 * size at quality 30/50/70/90, plus compression ratio vs original.
 *
 * Pipeline per image:
 *   1. Decode JPEG via hgjpegDecode → pixel data on device
 *   2. For each quality level Q in {30, 50, 70, 90}:
 *      a. Set quality via hgjpegEncoderParamsSetQuality
 *      b. Encode via hgjpegEncodeImage
 *      c. Retrieve bitstream size via hgjpegEncodeRetrieveBitstream
 *   3. Print quality-vs-size table
 *   4. Write highest-quality output to disk
 */
#include <dirent.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include <hggc_runtime.h>
#include <hgjpeg.h>
#include <helper_hggc.h>
#include <helper_functions.h>

/* ── Configuration ──────────────────────────────────────────── */

static const int QUALITY_LEVELS[] = {30, 50, 70, 90};
static const int NUM_QUALITY = 4;

/* ── JpegTranscoder class ───────────────────────────────────── */

class JpegTranscoder {
public:
    JpegTranscoder() : handle_(nullptr), dec_state_(nullptr),
                       enc_state_(nullptr), enc_params_(nullptr) {}

    ~JpegTranscoder() { cleanup(); }

    bool init()
    {
        hgjpegDevAllocator_t dev_alloc = {
            [](void **p, size_t s) -> int { return (int)hggcMalloc(p, s); },
            [](void *p) -> int { return (int)hggcFree(p); }
        };

        checkHggcErrors(hgjpegCreate(HGJPEG_BACKEND_DEFAULT, &dev_alloc, &handle_));
        checkHggcErrors(hgjpegJpegStateCreate(handle_, &dec_state_));
        checkHggcErrors(hgjpegEncoderStateCreate(handle_, &enc_state_, nullptr));
        checkHggcErrors(hgjpegEncoderParamsCreate(handle_, &enc_params_, nullptr));
        return true;
    }

    void cleanup()
    {
        if (enc_params_) { checkHggcErrors(hgjpegEncoderParamsDestroy(enc_params_)); enc_params_ = nullptr; }
        if (enc_state_)  { checkHggcErrors(hgjpegEncoderStateDestroy(enc_state_)); enc_state_ = nullptr; }
        if (dec_state_)  { checkHggcErrors(hgjpegJpegStateDestroy(dec_state_)); dec_state_ = nullptr; }
        if (handle_)     { checkHggcErrors(hgjpegDestroy(handle_)); handle_ = nullptr; }
    }

    /* Decode a JPEG buffer, return image info + decoded pixels */
    bool decode(const unsigned char *jpeg_data, size_t jpeg_len,
                hgjpegImage_t &img, int &width, int &height,
                int &channels, hgjpegChromaSubsampling_t &subsampling)
    {
        int w[HGJPEG_MAX_COMPONENT] = {0}, h[HGJPEG_MAX_COMPONENT] = {0};
        checkHggcErrors(hgjpegGetImageInfo(handle_, jpeg_data, jpeg_len,
            &channels, &subsampling, w, h));
        width = w[0];
        height = h[0];

        /* Allocate output buffer for RGB interleaved */
        size_t buf_sz = width * height * 3;
        unsigned char *d_buf;
        checkHggcErrors(hggcMalloc(&d_buf, buf_sz));

        img.channel[0] = d_buf;
        img.pitch[0] = width * 3;
        for (int c = 1; c < HGJPEG_MAX_COMPONENT; c++) {
            img.channel[c] = nullptr;
            img.pitch[c] = 0;
        }

        checkHggcErrors(hgjpegDecode(handle_, dec_state_, jpeg_data, jpeg_len,
            HGJPEG_OUTPUT_RGBI, &img, nullptr));
        checkHggcErrors(hggcDeviceSynchronize());
        return true;
    }

    /* Encode at given quality, return bitstream size */
    size_t encode_at_quality(const hgjpegImage_t &img, int width, int height,
                             int quality, std::vector<unsigned char> &out_buf)
    {
        checkHggcErrors(hgjpegEncoderParamsSetQuality(enc_params_, quality, nullptr));
        checkHggcErrors(hgjpegEncoderParamsSetSamplingFactors(
            enc_params_, HGJPEG_CSS_420, nullptr));

        checkHggcErrors(hgjpegEncodeImage(handle_, enc_state_, enc_params_,
            &img, HGJPEG_INPUT_RGBI, width, height, nullptr));

        size_t len = 0;
        checkHggcErrors(hgjpegEncodeRetrieveBitstream(handle_, enc_state_,
            nullptr, &len, nullptr));
        out_buf.resize(len);
        checkHggcErrors(hgjpegEncodeRetrieveBitstream(handle_, enc_state_,
            out_buf.data(), &len, nullptr));
        return len;
    }

    void free_image(hgjpegImage_t &img)
    {
        if (img.channel[0]) {
            checkHggcErrors(hggcFree(img.channel[0]));
            img.channel[0] = nullptr;
        }
    }

private:
    hgjpegHandle_t handle_;
    hgjpegJpegState_t dec_state_;
    hgjpegEncoderState_t enc_state_;
    hgjpegEncoderParams_t enc_params_;
};

/* ── File helpers ───────────────────────────────────────────── */

static std::vector<std::string> list_jpeg_files(const std::string &dir)
{
    std::vector<std::string> files;
    DIR *d = opendir(dir.c_str());
    if (!d) return files;
    struct dirent *ent;
    while ((ent = readdir(d)) != nullptr) {
        std::string name = ent->d_name;
        if (name.size() > 4) {
            std::string ext = name.substr(name.size() - 4);
            if (ext == ".jpg" || ext == ".JPG") {
                files.push_back(dir + "/" + name);
            }
        }
    }
    closedir(d);
    return files;
}

static bool load_file(const std::string &path, std::vector<char> &data)
{
    std::ifstream f(path, std::ios::binary | std::ios::ate);
    if (!f.is_open()) return false;
    std::streamsize sz = f.tellg();
    f.seekg(0, std::ios::beg);
    data.resize(sz);
    return (bool)f.read(data.data(), sz);
}

static std::string basename(const std::string &path)
{
    size_t pos = path.rfind('/');
    return (pos == std::string::npos) ? path : path.substr(pos + 1);
}

/* ── Main ───────────────────────────────────────────────────── */

int main(int argc, const char *argv[])
{
    printf("[hg_jpeg_encoder] JPEG Transcoder with Quality-Size Analysis\n\n");

    // findHggcDevice(argc, (const char **)argv);

    /* Parse args */
    std::string input_dir = "./images";
    std::string output_dir = "encode_output";
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-i") == 0 && i + 1 < argc)
            input_dir = argv[++i];
        else if (strcmp(argv[i], "-o") == 0 && i + 1 < argc)
            output_dir = argv[++i];
    }

    /* List files */
    auto files = list_jpeg_files(input_dir);
    if (files.empty()) {
        printf("  No JPEG files found in: %s\n", input_dir.c_str());
        return EXIT_FAILURE;
    }
    printf("  Found %d JPEG files\n", (int)files.size());
    printf("  Quality levels: ");
    for (int i = 0; i < NUM_QUALITY; i++)
        printf("%d%s", QUALITY_LEVELS[i], i < NUM_QUALITY - 1 ? "/" : "\n\n");

    /* Init transcoder */
    JpegTranscoder transcoder;
    transcoder.init();

    /* Create output dir */
    char mkdir_cmd[256];
    snprintf(mkdir_cmd, sizeof(mkdir_cmd), "mkdir -p %s 2>/dev/null", output_dir.c_str());
    system(mkdir_cmd);

    /* Print table header */
    printf("  %-20s %-8s %-5s", "File", "Size", "Orig");
    for (int q = 0; q < NUM_QUALITY; q++)
        printf(" Q%d", QUALITY_LEVELS[q]);
    printf("\n");
    printf("  %-20s %-8s %-5s", "----", "----", "----");
    for (int q = 0; q < NUM_QUALITY; q++)
        printf(" ---");
    printf("\n");

    int total_processed = 0;

    for (const auto &filepath : files) {
        std::string fname = basename(filepath);

        /* Load JPEG file */
        std::vector<char> raw;
        if (!load_file(filepath, raw)) {
            fprintf(stderr, "  Cannot read: %s\n", filepath.c_str());
            continue;
        }
        size_t orig_size = raw.size();

        /* Decode */
        hgjpegImage_t img = {};
        int width, height, channels;
        hgjpegChromaSubsampling_t subsampling;
        if (!transcoder.decode((const unsigned char *)raw.data(), raw.size(),
                               img, width, height, channels, subsampling)) {
            fprintf(stderr, "  Decode failed: %s\n", fname.c_str());
            continue;
        }

        /* Encode at each quality level */
        size_t sizes[NUM_QUALITY];
        std::vector<unsigned char> best_buf;
        size_t best_size = 0;

        for (int q = 0; q < NUM_QUALITY; q++) {
            std::vector<unsigned char> buf;
            sizes[q] = transcoder.encode_at_quality(img, width, height,
                                                     QUALITY_LEVELS[q], buf);
            /* Keep the highest quality output */
            if (q == NUM_QUALITY - 1) {
                best_buf = std::move(buf);
                best_size = sizes[q];
            }
        }

        /* Print row */
        printf("  %-20s %dx%-4d %-5zu", fname.c_str(), width, height, orig_size);
        for (int q = 0; q < NUM_QUALITY; q++)
            printf(" %4zu", sizes[q]);
        printf("\n");

        /* Write best quality output */
        std::string out_path = output_dir + "/" + fname;
        std::ofstream out(out_path, std::ios::binary);
        if (out.is_open()) {
            out.write(reinterpret_cast<const char *>(best_buf.data()), best_size);
            out.close();
        }

        transcoder.free_image(img);
        total_processed++;
    }

    printf("\n  Total images transcoded: %d\n", total_processed);
    printf("  Result: %s\n", total_processed > 0 ? "PASS" : "FAIL");

    return (total_processed > 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
