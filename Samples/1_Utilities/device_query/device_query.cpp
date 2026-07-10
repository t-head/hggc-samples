/*
 * Copyright (c) 2023-2026, T-HEAD (SHANGHAI) SEMICONDUCTOR CO., LTD.
 * All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Sample code demonstrating T-HEAD SAIL SDK usage. This code is provided
 * under the Apache License 2.0 for reference and educational purposes.
 *
 * device_query -- PPU Device Property Inspector (Runtime API)
 *
 * Queries and displays PPU device properties organized by functional
 * category (Compute, Memory, Scheduling, Topology). Uses the HGGC
 * Runtime API.
 */

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

#include <hggc.h>
#include <hggc_runtime.h>
#include <helper_hggc.h>

/* ── DeviceReporter: grouped device property report ────────── */

class DeviceReporter {
public:
    explicit DeviceReporter(int dev_id) : id(dev_id) {
        hggcSetDevice(dev_id);
        hggcGetDeviceProperties(&props, dev_id);
        hggcDriverGetVersion(&drv_ver);
        hggcRuntimeGetVersion(&rt_ver);
    }

    void report_header() const {
        printf("\n┌─ Device %d: %s ", id, props.name);
        printf("\n│  Driver %d.%d / Runtime %d.%d\n",
               drv_ver / 1000, (drv_ver % 100) / 10,
               rt_ver / 1000, (rt_ver % 100) / 10);
        printf("│  Capability %d.%d\n", props.major, props.minor);
    }

    void report_compute() const {
        int cores_per_cu = ppuCoresPerUnit(props.major, props.minor);
        int clk = query_attr(hggcDevAttrClockRate);
        printf("│\n│  [Compute]\n");
        printf("│  Compute Units      : %d\n", props.multiProcessorCount);
        printf("│  Cores per CU       : %d\n", cores_per_cu);
        printf("│  Total Cores        : %d\n", cores_per_cu * props.multiProcessorCount);
        printf("│  Clock Rate         : %.0f MHz (%.2f GHz)\n",
               clk * 1e-3f, clk * 1e-6f);
        printf("│  Max Threads/CU     : %d\n", props.maxThreadsPerMultiProcessor);
    }

    void report_memory() const {
        int mem_clk = query_attr(hggcDevAttrMemoryClockRate);
        printf("│\n│  [Memory]\n");
        printf("│  Global Memory      : %.0f MB (%llu bytes)\n",
               static_cast<float>(props.totalGlobalMem) / 1048576.0f,
               static_cast<unsigned long long>(props.totalGlobalMem));
        printf("│  Memory Clock       : %.0f MHz\n", mem_clk * 1e-3f);
        printf("│  Bus Width          : %d-bit\n", props.memoryBusWidth);
        if (props.l2CacheSize)
            printf("│  L2 Cache           : %d bytes\n", props.l2CacheSize);
        printf("│  Constant Memory    : %zu bytes\n", props.totalConstMem);
        printf("│  Shared Mem/Block   : %zu bytes\n", props.sharedMemPerBlock);
        printf("│  Shared Mem/CU      : %zu bytes\n", props.sharedMemPerMultiprocessor);
        printf("│  Registers/Block    : %d\n", props.regsPerBlock);
    }

    void report_scheduling() const {
        printf("│\n│  [Scheduling]\n");
        printf("│  Warp Size          : %d\n", props.warpSize);
        printf("│  Max Threads/Block  : %d\n", props.maxThreadsPerBlock);
        printf("│  Block Dim Limit    : (%d, %d, %d)\n",
               props.maxThreadsDim[0], props.maxThreadsDim[1], props.maxThreadsDim[2]);
        printf("│  Grid Dim Limit     : (%d, %d, %d)\n",
               props.maxGridSize[0], props.maxGridSize[1], props.maxGridSize[2]);
        printf("│  Memory Pitch       : %zu bytes\n", props.memPitch);
        printf("│  Texture Alignment  : %zu bytes\n", props.textureAlignment);

        int overlap = query_attr(hggcDevAttrGpuOverlap);
        int timeout = query_attr(hggcDevAttrKernelExecTimeout);
        printf("│  Async Copy Engine  : %s (%d engine(s))\n",
               overlap ? "Yes" : "No", props.asyncEngineCount);
        printf("│  Kernel Timeout     : %s\n", timeout ? "Yes" : "No");
    }

    void report_features() const {
        printf("│\n│  [Features]\n");
        printf("│  ECC                : %s\n", props.ECCEnabled ? "Enabled" : "Disabled");
        printf("│  Unified Addressing : %s\n", props.unifiedAddressing ? "Yes" : "No");
        printf("│  Managed Memory     : %s\n", props.managedMemory ? "Yes" : "No");
        printf("│  Compute Preemption : %s\n",
               props.computePreemptionSupported ? "Yes" : "No");
        printf("│  Cooperative Launch : %s\n", props.cooperativeLaunch ? "Yes" : "No");
        int coop_multi = query_attr(hggcDevAttrCooperativeLaunch);
        printf("│  Multi-Device Coop  : %s\n", coop_multi ? "Yes" : "No");
        printf("│  Integrated         : %s\n", props.integrated ? "Yes" : "No");
        printf("│  PCI Location       : %d:%d:%d\n",
               props.pciDomainID, props.pciBusID, props.pciDeviceID);
    }

    void report_texture_limits() const {
        printf("│\n│  [Texture Limits]\n");
        printf("│  1D Max             : %d\n", props.maxTexture1D);
        printf("│  2D Max             : (%d, %d)\n",
               props.maxTexture2D[0], props.maxTexture2D[1]);
        printf("│  3D Max             : (%d, %d, %d)\n",
               props.maxTexture3D[0], props.maxTexture3D[1], props.maxTexture3D[2]);
        printf("│  1D Layered         : (%d, %d layers)\n",
               props.maxTexture1DLayered[0], props.maxTexture1DLayered[1]);
        printf("│  2D Layered         : (%d, %d, %d layers)\n",
               props.maxTexture2DLayered[0], props.maxTexture2DLayered[1],
               props.maxTexture2DLayered[2]);
    }

    const char *name() const { return props.name; }
    int capability_major() const { return props.major; }

private:
    int query_attr(int attr_enum) const {
        int val = 0;
        hggcDeviceGetAttribute(&val, (hggcDeviceAttr)attr_enum, id);
        return val;
    }

    int           id;
    hggcDeviceProp props;
    int           drv_ver = 0, rt_ver = 0;
};

/* ── P2P topology matrix ───────────────────────────────────── */

static void check_p2p_access(int num_devices) {
    if (num_devices < 2) return;

    int capable[64], n_capable = 0;
    for (int i = 0; i < num_devices; i++) {
        hggcDeviceProp p;
        hggcGetDeviceProperties(&p, i);
        if (p.major >= 2)
            capable[n_capable++] = i;
    }

    if (n_capable < 2) return;

    /* Build access matrix */
    int matrix[64][64];
    for (int i = 0; i < n_capable; i++) {
        for (int j = 0; j < n_capable; j++) {
            if (capable[i] == capable[j]) {
                matrix[i][j] = -1;  /* self */
            } else {
                int can_access;
                hggcDeviceCanAccessPeer(&can_access, capable[i], capable[j]);
                matrix[i][j] = can_access;
            }
        }
    }

    /* Print matrix */
    printf("\n┌─ [P2P Access Matrix]  (row→col: can row access col?)\n");
    printf("│        ");
    for (int j = 0; j < n_capable; j++)
        printf("D%-3d ", capable[j]);
    printf("\n");

    for (int i = 0; i < n_capable; i++) {
        printf("│  D%-3d  ", capable[i]);
        for (int j = 0; j < n_capable; j++) {
            if (matrix[i][j] < 0)
                printf("  -  ");
            else
                printf("  %c  ", matrix[i][j] ? 'Y' : 'N');
        }
        printf("\n");
    }
    printf("└\n");
}

/* ── Main ──────────────────────────────────────────────────── */

int main(int argc, char **argv)
{
    printf("[device_query] PPU Device Property Inspector (Runtime API)\n");

    int num_devices = 0;
    hggcError_t status = hggcGetDeviceCount(&num_devices);
    if (status != hggcSuccess) {
        fprintf(stderr, "hggcGetDeviceCount failed: %s\n", hggcGetErrorString(status));
        return EXIT_FAILURE;
    }

    if (num_devices == 0) {
        printf("  No HGGC-capable devices found.\n");
        printf("Result = FAIL\n");
        return EXIT_FAILURE;
    }

    printf("  Detected %d device(s)\n", num_devices);

    for (int i = 0; i < num_devices; i++) {
        DeviceReporter rpt(i);
        rpt.report_header();
        rpt.report_compute();
        rpt.report_memory();
        rpt.report_scheduling();
        rpt.report_texture_limits();
        rpt.report_features();
        printf("└\n");
    }

    check_p2p_access(num_devices);

    printf("\nResult = PASS\n");
    return EXIT_SUCCESS;
}
