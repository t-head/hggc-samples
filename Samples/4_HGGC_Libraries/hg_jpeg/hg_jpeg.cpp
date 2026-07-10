/*
 * Copyright (c) 2023-2026, T-HEAD (SHANGHAI) SEMICONDUCTOR CO., LTD.
 * All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Sample code demonstrating T-HEAD SAIL SDK usage. This code is provided
 * under the Apache License 2.0 for reference and educational purposes.
 *
 * hg_jpeg -- Batch JPEG Decoder with Pixel Statistics Verification
 *
 * Demonstrates hgjpeg batch decoding with on-device pixel statistics
 * computation. Unlike a pure performance benchmark, this sample verifies
 * decode correctness by computing per-channel min/max/mean values using
 * a custom PPU kernel, showing hgjpeg + kernel interoperability.
 *
 * Pipeline:
 *   1. Load JPEG files from a directory
 *   2. Query image metadata (dimensions, channels, subsampling)
 *   3. Batch-decode via hgjpegDecodeBatched
 *   4. Compute per-channel pixel statistics on device
 *   5. Print summary table with metadata + stats
 */
#include <dirent.h>
#include <cmath>
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

/* ── Device kernel: per-channel pixel statistics ────────────── */

struct ChannelStats {
    int min_val;
    int max_val;
    double sum_val;  // use double for accumulation
    int count;
};

/* Compute min/max/sum for each channel of a decoded image.
 * Each block handles one channel; threads cooperatively reduce.
 */
__global__ void compute_channel_stats(
    const unsigned char *channel_data, int pitch, int width, int height,
    int *out_min, int *out_max, long long *out_sum)
{
    extern __shared__ int smem[];
    int *smin = smem;
    int *smax = smem + blockDim.x;

    int tid = threadIdx.x;
    int total = width * height;

    int local_min = 255;
    int local_max = 0;
    long long local_sum = 0;

    for (int i = tid; i < total; i += blockDim.x) {
        int row = i / width;
        int col = i % width;
        int val = channel_data[row * pitch + col];
        if (val < local_min) local_min = val;
        if (val > local_max) local_max = val;
        local_sum += val;
    }

    smin[tid] = local_min;
    smax[tid] = local_max;
    __syncthreads();

    /* Tree reduction for min */
    for (int s = blockDim.x / 2; s > 0; s >>= 1) {
        if (tid < s) {
            if (smin[tid + s] < smin[tid]) smin[tid] = smin[tid + s];
            if (smax[tid + s] > smax[tid]) smax[tid] = smax[tid + s];
        }
        __syncthreads();
    }

    if (tid == 0) {
        *out_min = smin[0];
        *out_max = smax[0];
    }

    /* Sum reduction using atomicAdd on long long */
    if (local_sum != 0) {
        atomicAdd((unsigned long long *)out_sum, (unsigned long long)local_sum);
    }
}

/* ── JpegBatchDecoder class ─────────────────────────────────── */

class JpegBatchDecoder {
public:
    JpegBatchDecoder(int batch_size = 1, hgjpegOutputFormat_t fmt = HGJPEG_OUTPUT_RGB)
        : batch_size_(batch_size), fmt_(fmt), handle_(nullptr), state_(nullptr) {}

    ~JpegBatchDecoder() { cleanup(); }

    bool init()
    {
        hgjpegDevAllocator_t dev_alloc = {
            [](void **p, size_t s) -> int { return (int)hggcMalloc(p, s); },
            [](void *p) -> int { return (int)hggcFree(p); }
        };
        hgjpegPinnedAllocator_t pinned_alloc = {
            [](void **p, size_t s, unsigned int f) -> int { return (int)hggcHostAlloc(p, s, f); },
            [](void *p) -> int { return (int)hggcFreeHost(p); }
        };

        checkHggcErrors(hgjpegCreateEx(HGJPEG_BACKEND_DEFAULT,
            &dev_alloc, &pinned_alloc, 0, &handle_));
        checkHggcErrors(hgjpegJpegStateCreate(handle_, &state_));
        checkHggcErrors(hgjpegDecodeBatchedInitialize(
            handle_, state_, batch_size_, 1, fmt_));
        return true;
    }

    void cleanup()
    {
        if (state_) {
            checkHggcErrors(hgjpegJpegStateDestroy(state_));
            state_ = nullptr;
        }
        if (handle_) {
            checkHggcErrors(hgjpegDestroy(handle_));
            handle_ = nullptr;
        }
    }

    /* Query metadata for a single JPEG buffer */
    bool query_image_info(const unsigned char *data, size_t len,
                          int &channels, hgjpegChromaSubsampling_t &subsampling,
                          int *widths, int *heights)
    {
        checkHggcErrors(hgjpegGetImageInfo(handle_, data, len,
            &channels, &subsampling, widths, heights));
        return true;
    }

    /* Decode a batch of images */
    bool decode_batch(const std::vector<std::vector<char>> &raw_data,
                      const std::vector<size_t> &raw_lens,
                      std::vector<hgjpegImage_t> &out_images)
    {
        std::vector<const unsigned char *> ptrs(batch_size_);
        for (int i = 0; i < batch_size_; i++)
            ptrs[i] = (const unsigned char *)raw_data[i].data();

        checkHggcErrors(hgjpegDecodeBatched(
            handle_, state_, ptrs.data(), raw_lens.data(),
            out_images.data(), 0));
        return true;
    }

    hgjpegHandle_t handle() const { return handle_; }
    hgjpegJpegState_t state() const { return state_; }
    int batch_size() const { return batch_size_; }
    hgjpegOutputFormat_t format() const { return fmt_; }

private:
    int batch_size_;
    hgjpegOutputFormat_t fmt_;
    hgjpegHandle_t handle_;
    hgjpegJpegState_t state_;
};

/* ── File loading helpers ───────────────────────────────────── */

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
            if (ext == ".jpg" || ext == ".JPG" || ext == "jpeg") {
                files.push_back(dir + "/" + name);
            }
        }
    }
    closedir(d);
    return files;
}

static bool load_file(const std::string &path, std::vector<char> &data, size_t &len)
{
    std::ifstream f(path, std::ios::binary | std::ios::ate);
    if (!f.is_open()) return false;
    std::streamsize sz = f.tellg();
    f.seekg(0, std::ios::beg);
    data.resize(sz);
    if (!f.read(data.data(), sz)) return false;
    len = sz;
    return true;
}

/* ── Stats computation ─────────────────────────────────────── */

static void compute_and_print_stats(
    const hgjpegImage_t &img, int width, int height,
    const std::string &name)
{
    int num_ch = (img.channel[1] != nullptr) ? 3 : 1;
    int smem_size = 2 * 256 * sizeof(int);  /* min + max for 256 threads */

    printf("    %-30s  %dx%d  %dch  ", name.c_str(), width, height, num_ch);

    for (int c = 0; c < num_ch; c++) {
        int *d_min, *d_max;
        long long *d_sum;
        long long h_sum = 0;
        int h_min = 0, h_max = 0;

        checkHggcErrors(hggcMalloc(&d_min, sizeof(int)));
        checkHggcErrors(hggcMalloc(&d_max, sizeof(int)));
        checkHggcErrors(hggcMalloc(&d_sum, sizeof(long long)));
        checkHggcErrors(hggcMemset(d_sum, 0, sizeof(long long)));

        int pitch = img.pitch[c];
        int total = width * height;
        int threads = (total < 256) ? total : 256;

        compute_channel_stats<<<1, 256, smem_size>>>(
            img.channel[c], pitch, width, height, d_min, d_max, d_sum);
        checkHggcErrors(hggcDeviceSynchronize());

        checkHggcErrors(hggcMemcpy(&h_min, d_min, sizeof(int),
                         hggcMemcpyDeviceToHost));
        checkHggcErrors(hggcMemcpy(&h_max, d_max, sizeof(int),
                         hggcMemcpyDeviceToHost));
        checkHggcErrors(hggcMemcpy(&h_sum, d_sum, sizeof(long long),
                         hggcMemcpyDeviceToHost));

        double mean = (total > 0) ? (double)h_sum / total : 0.0;
        printf("ch%d:[%d-%d] mean=%.1f  ", c, h_min, h_max, mean);

        checkHggcErrors(hggcFree(d_min));
        checkHggcErrors(hggcFree(d_max));
        checkHggcErrors(hggcFree(d_sum));
    }
    printf("\n");
}

/* ── Subsampling name helper ────────────────────────────────── */

static const char *subsampling_name(hgjpegChromaSubsampling_t s)
{
    switch (s) {
        case HGJPEG_CSS_444:    return "4:4:4";
        case HGJPEG_CSS_422:    return "4:2:2";
        case HGJPEG_CSS_420:    return "4:2:0";
        case HGJPEG_CSS_411:    return "4:1:1";
        case HGJPEG_CSS_410:    return "4:1:0";
        case HGJPEG_CSS_GRAY:   return "gray";
        case HGJPEG_CSS_440:    return "4:4:0";
        default:                return "unknown";
    }
}

/* ── Main ───────────────────────────────────────────────────── */

int main(int argc, const char *argv[])
{
    printf("[hg_jpeg] Batch JPEG Decoder with Pixel Statistics\n\n");

    // findHggcDevice(argc, (const char **)argv);

    /* Parse input directory */
    std::string input_dir = "./images";
    int batch_size = 1;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-i") == 0 && i + 1 < argc) {
            input_dir = argv[++i];
        } else if (strcmp(argv[i], "-b") == 0 && i + 1 < argc) {
            batch_size = atoi(argv[++i]);
        }
    }

    /* List JPEG files */
    std::vector<std::string> files = list_jpeg_files(input_dir);
    if (files.empty()) {
        printf("  No JPEG files found in: %s\n", input_dir.c_str());
        return EXIT_FAILURE;
    }
    printf("  Found %d JPEG files in %s\n", (int)files.size(), input_dir.c_str());
    printf("  Batch size: %d\n\n", batch_size);

    /* Initialize decoder */
    JpegBatchDecoder decoder(batch_size, HGJPEG_OUTPUT_RGB);
    if (!decoder.init()) {
        fprintf(stderr, "  ERROR: Failed to initialize hgjpeg\n");
        return EXIT_FAILURE;
    }

    /* Print header */
    printf("  %-30s  %-7s  %-4s  %s\n",
           "File", "Size", "Ch", "Pixel Statistics");
    printf("  %-30s  %-7s  %-4s  %s\n",
           "----", "----", "--", "----------------");

    /* Process files in batches */
    int total_decoded = 0;
    int file_idx = 0;

    while (file_idx < (int)files.size()) {
        /* Load batch */
        std::vector<std::vector<char>> raw_data(batch_size);
        std::vector<size_t> raw_lens(batch_size);
        std::vector<std::string> batch_names(batch_size);

        for (int b = 0; b < batch_size && file_idx < (int)files.size(); b++) {
            batch_names[b] = files[file_idx];
            if (!load_file(files[file_idx], raw_data[b], raw_lens[b])) {
                fprintf(stderr, "  Cannot read: %s\n", files[file_idx].c_str());
                file_idx++;
                b--;
                continue;
            }
            file_idx++;
        }

        /* Query metadata for first image in batch */
        int channels = 0;
        hgjpegChromaSubsampling_t subsampling;
        int widths[HGJPEG_MAX_COMPONENT] = {0};
        int heights[HGJPEG_MAX_COMPONENT] = {0};

        decoder.query_image_info(
            (const unsigned char *)raw_data[0].data(), raw_lens[0],
            channels, subsampling, widths, heights);

        /* Allocate output buffers */
        std::vector<hgjpegImage_t> out_images(batch_size);
        int out_channels = (decoder.format() == HGJPEG_OUTPUT_RGB ||
                            decoder.format() == HGJPEG_OUTPUT_BGR) ? 3 : 1;
        for (int b = 0; b < batch_size; b++) {
            for (int c = 0; c < HGJPEG_MAX_COMPONENT; c++) {
                out_images[b].channel[c] = nullptr;
                out_images[b].pitch[c] = 0;
            }
            for (int c = 0; c < out_channels; c++) {
                int sz = widths[0] * heights[0];
                checkHggcErrors(hggcMalloc(&out_images[b].channel[c], sz));
                out_images[b].pitch[c] = widths[0];
            }
        }

        /* Decode */
        if (!decoder.decode_batch(raw_data, raw_lens, out_images)) {
            fprintf(stderr, "  Decode failed for batch starting at %s\n",
                    batch_names[0].c_str());
        } else {
            /* Compute and print stats for each decoded image */
            for (int b = 0; b < batch_size; b++) {
                /* Extract filename from path */
                std::string fname = batch_names[b];
                size_t pos = fname.rfind('/');
                if (pos != std::string::npos)
                    fname = fname.substr(pos + 1);

                compute_and_print_stats(out_images[b],
                    widths[0], heights[0], fname);
                total_decoded++;
            }
        }

        /* Free output buffers */
        for (int b = 0; b < batch_size; b++) {
            for (int c = 0; c < HGJPEG_MAX_COMPONENT; c++) {
                if (out_images[b].channel[c])
                    checkHggcErrors(hggcFree(out_images[b].channel[c]));
            }
        }
    }

    printf("\n  Total images decoded: %d\n", total_decoded);
    printf("  Result: %s\n", total_decoded > 0 ? "PASS" : "FAIL");

    return (total_decoded > 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
