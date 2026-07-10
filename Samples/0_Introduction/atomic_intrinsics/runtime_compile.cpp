/*
 * Copyright (c) 2023-2026, T-HEAD (SHANGHAI) SEMICONDUCTOR CO., LTD.
 * All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Sample code demonstrating T-HEAD SAIL SDK usage. This code is provided
 * under the Apache License 2.0 for reference and educational purposes.
 */
#include <cstdio>
#include <cstdlib>
#include <vector>

#include <hggc_runtime.h>
#include <hgrtc_helper.h>
#include <helper_functions.h>

#include "atomic_kernel.hgh"

namespace {

constexpr unsigned int kThreadsPerBlock = 256;
constexpr unsigned int kBlocksPerGrid   = 64;

constexpr const char *kSampleLabel  = "atomic_intrinsics_hgrtc";
constexpr const char *kKernelHeader = "atomic_kernel_rtc.hg";
constexpr const char *kKernelEntry  = "atomic_op_suite";

/// Initialise host buffer with the bit-pattern that the kernel expects.
void seed_host_buffer(int *host_slots) {
    for (int i = 0; i < kAtomicSlotCount; ++i) {
        host_slots[i] = 0;
    }
    host_slots[kSlotAnd] = ATOMIC_BITWISE_SEED;
    host_slots[kSlotXor] = ATOMIC_BITWISE_SEED;
}

}  /// anonymous

extern "C" bool computeGold(int *ppuData, const int len);

int main(int argc, char **argv) {
    std::printf("%s starting...\n", kSampleLabel);

    /// Compile the kernel source into a HGBIN blob and resolve the entry point.
    char  *hgbin       = nullptr;
    size_t hgbin_size  = 0;
    char  *kernel_path = findSampleAsset(kKernelHeader, argv[0]);
    compileFileToHGBIN(kernel_path, argc, argv, &hgbin, &hgbin_size, 0);

    HGmodule   module = loadHGBIN(hgbin, argc, argv);
    HGfunction kernel_addr;
    checkHggcErrors(hgModuleGetFunction(&kernel_addr, module, kKernelEntry));

    HggcTimer timer;
    timer.start();

    constexpr unsigned int byte_count = sizeof(int) * kAtomicSlotCount;

    std::vector<int> host_slots(kAtomicSlotCount);
    seed_host_buffer(host_slots.data());

    HGdeviceptr dev_slots;
    checkHggcErrors(hgMemAlloc(&dev_slots, byte_count));
    checkHggcErrors(hgMemcpyHtoD(dev_slots, host_slots.data(), byte_count));

    dim3 block_dim(kThreadsPerBlock, 1, 1);
    dim3 grid_dim(kBlocksPerGrid, 1, 1);

    void *kernel_args[] = {(void *)&dev_slots};
    checkHggcErrors(hgLaunchKernel(kernel_addr,
                                   grid_dim.x, grid_dim.y, grid_dim.z,
                                   block_dim.x, block_dim.y, block_dim.z,
                                   0, 0,
                                   &kernel_args[0],
                                   0));

    checkHggcErrors(hgCtxSynchronize());
    checkHggcErrors(hgMemcpyDtoH(host_slots.data(), dev_slots, byte_count));

    timer.stop();
    std::printf("Processing time: %f (ms)\n", timer.elapsed());

    bool ok = computeGold(host_slots.data(), kThreadsPerBlock * kBlocksPerGrid);

    checkHggcErrors(hgMemFree(dev_slots));

    std::printf("%s completed, returned %s\n", kSampleLabel, ok ? "OK" : "ERROR!");
    return ok ? EXIT_SUCCESS : EXIT_FAILURE;
}
