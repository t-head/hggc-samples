/*
 * Copyright (c) 2023-2026, T-HEAD (SHANGHAI) SEMICONDUCTOR CO., LTD.
 * All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Sample code demonstrating T-HEAD SAIL SDK usage. This code is provided
 * under the Apache License 2.0 for reference and educational purposes.
 *
 * device_query_drv -- PPU Device Property Inspector (Driver API)
 *
 * Queries device properties via the HGGC Driver API (hgDeviceGetAttribute),
 * presenting results as a compact key-value summary. Demonstrates Driver API
 * initialization, per-attribute querying, and P2P topology enumeration.
 */

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <hggc.h>
#include <helper_hggc_drvapi.h>

/* ── Attribute query helper ────────────────────────────────── */

static int attr(HGdevice dev, HGdevice_attribute a) {
    int v = 0;
    checkHggcErrors(hgDeviceGetAttribute(&v, a, dev));
    return v;
}

static const char *yesno(int v) { return v ? "Yes" : "No"; }

/* ── Per-device report ─────────────────────────────────────── */

static void report_device(HGdevice dev) {
    char name[256] = {0};
    checkHggcErrors(hgDeviceGetName(name, sizeof(name), dev));

    int major = attr(dev, HG_DEVICE_ATTRIBUTE_COMPUTE_CAPABILITY_MAJOR);
    int minor = attr(dev, HG_DEVICE_ATTRIBUTE_COMPUTE_CAPABILITY_MINOR);
    int drv_ver = 0;
    checkHggcErrors(hgDriverGetVersion(&drv_ver));

    size_t total_mem = 0;
    checkHggcErrors(hgDeviceTotalMem(&total_mem, dev));

    int cu_count = attr(dev, HG_DEVICE_ATTRIBUTE_MULTIPROCESSOR_COUNT);
    int cores_cu = ppuCoresPerUnit(major, minor);

    printf("\n══ Device %d ══════════════════════════════════════\n", dev);
    printf("  Name              : %s\n", name);
    printf("  Driver Version    : %d.%d\n", drv_ver / 1000, (drv_ver % 100) / 10);
    printf("  Capability        : %d.%d\n", major, minor);
    printf("  Global Memory     : %.0f MB (%llu bytes)\n",
           static_cast<float>(total_mem) / 1048576.0f,
           static_cast<unsigned long long>(total_mem));

    printf("\n  -- Compute --\n");
    printf("  Compute Units     : %d\n", cu_count);
    printf("  Cores/CU          : %d\n", cores_cu);
    printf("  Total Cores       : %d\n", cores_cu * cu_count);
    printf("  Clock Rate        : %.0f MHz\n",
           attr(dev, HG_DEVICE_ATTRIBUTE_CLOCK_RATE) * 1e-3f);
    printf("  Max Threads/CU    : %d\n",
           attr(dev, HG_DEVICE_ATTRIBUTE_MAX_THREADS_PER_MULTIPROCESSOR));

    printf("\n  -- Memory --\n");
    printf("  Memory Clock      : %.0f MHz\n",
           attr(dev, HG_DEVICE_ATTRIBUTE_MEMORY_CLOCK_RATE) * 1e-3f);
    printf("  Bus Width         : %d-bit\n",
           attr(dev, HG_DEVICE_ATTRIBUTE_GLOBAL_MEMORY_BUS_WIDTH));
    int l2 = attr(dev, HG_DEVICE_ATTRIBUTE_LLC_CACHE_SIZE);
    if (l2) printf("  L2 Cache          : %d bytes\n", l2);
    printf("  Constant Memory   : %d bytes\n",
           attr(dev, HG_DEVICE_ATTRIBUTE_TOTAL_CONSTANT_MEMORY));
    printf("  Shared Mem/Block  : %d bytes\n",
           attr(dev, HG_DEVICE_ATTRIBUTE_MAX_SHARED_MEMORY_PER_BLOCK));
    printf("  Registers/Block   : %d\n",
           attr(dev, HG_DEVICE_ATTRIBUTE_MAX_REGISTERS_PER_BLOCK));

    printf("\n  -- Scheduling --\n");
    printf("  Warp Size         : %d\n",
           attr(dev, HG_DEVICE_ATTRIBUTE_WARP_SIZE));
    printf("  Max Threads/Block : %d\n",
           attr(dev, HG_DEVICE_ATTRIBUTE_MAX_THREADS_PER_BLOCK));
    printf("  Block Dims        : (%d, %d, %d)\n",
           attr(dev, HG_DEVICE_ATTRIBUTE_MAX_BLOCK_DIM_X),
           attr(dev, HG_DEVICE_ATTRIBUTE_MAX_BLOCK_DIM_Y),
           attr(dev, HG_DEVICE_ATTRIBUTE_MAX_BLOCK_DIM_Z));
    printf("  Grid Dims         : (%d, %d, %d)\n",
           attr(dev, HG_DEVICE_ATTRIBUTE_MAX_GRID_DIM_X),
           attr(dev, HG_DEVICE_ATTRIBUTE_MAX_GRID_DIM_Y),
           attr(dev, HG_DEVICE_ATTRIBUTE_MAX_GRID_DIM_Z));
    printf("  Memory Pitch      : %d bytes\n",
           attr(dev, HG_DEVICE_ATTRIBUTE_MAX_PITCH));
    printf("  Texture Alignment : %d bytes\n",
           attr(dev, HG_DEVICE_ATTRIBUTE_TEXTURE_ALIGNMENT));

    printf("\n  -- Texture Limits --\n");
    printf("  1D                : %d\n",
           attr(dev, HG_DEVICE_ATTRIBUTE_MAXIMUM_TEXTURE1D_WIDTH));
    printf("  2D                : (%d, %d)\n",
           attr(dev, HG_DEVICE_ATTRIBUTE_MAXIMUM_TEXTURE2D_WIDTH),
           attr(dev, HG_DEVICE_ATTRIBUTE_MAXIMUM_TEXTURE2D_HEIGHT));
    printf("  3D                : (%d, %d, %d)\n",
           attr(dev, HG_DEVICE_ATTRIBUTE_MAXIMUM_TEXTURE3D_WIDTH),
           attr(dev, HG_DEVICE_ATTRIBUTE_MAXIMUM_TEXTURE3D_HEIGHT),
           attr(dev, HG_DEVICE_ATTRIBUTE_MAXIMUM_TEXTURE3D_DEPTH));

    printf("\n  -- Capabilities --\n");
    printf("  Async Copy        : %s (%d engines)\n",
           yesno(attr(dev, HG_DEVICE_ATTRIBUTE_GPU_OVERLAP)),
           attr(dev, HG_DEVICE_ATTRIBUTE_ASYNC_ENGINE_COUNT));
    printf("  Kernel Timeout    : %s\n",
           yesno(attr(dev, HG_DEVICE_ATTRIBUTE_KERNEL_EXEC_TIMEOUT)));
    printf("  Concurrent Kernels: %s\n",
           yesno(attr(dev, HG_DEVICE_ATTRIBUTE_CONCURRENT_KERNELS)));
    printf("  Integrated        : %s\n",
           yesno(attr(dev, HG_DEVICE_ATTRIBUTE_INTEGRATED)));
    printf("  ECC               : %s\n",
           yesno(attr(dev, HG_DEVICE_ATTRIBUTE_ECC_ENABLED)));
    printf("  Unified Addressing: %s\n",
           yesno(attr(dev, HG_DEVICE_ATTRIBUTE_UNIFIED_ADDRESSING)));
    printf("  Managed Memory    : %s\n",
           yesno(attr(dev, HG_DEVICE_ATTRIBUTE_MANAGED_MEMORY)));
    printf("  Compute Preemption: %s\n",
           yesno(attr(dev, HG_DEVICE_ATTRIBUTE_COMPUTE_PREEMPTION_SUPPORTED)));
    printf("  Cooperative Launch: %s\n",
           yesno(attr(dev, HG_DEVICE_ATTRIBUTE_COOPERATIVE_LAUNCH)));
    printf("  Multi-Device Coop : %s\n",
           yesno(attr(dev, HG_DEVICE_ATTRIBUTE_COOPERATIVE_MULTI_DEVICE_LAUNCH)));
    printf("  PCI               : %d:%d:%d\n",
           attr(dev, HG_DEVICE_ATTRIBUTE_PCI_DOMAIN_ID),
           attr(dev, HG_DEVICE_ATTRIBUTE_PCI_BUS_ID),
           attr(dev, HG_DEVICE_ATTRIBUTE_PCI_DEVICE_ID));
    printf("══════════════════════════════════════════════════\n");
}

/* ── P2P topology matrix ───────────────────────────────────── */

static void report_p2p(int num_devices) {
    if (num_devices < 2) return;

    int capable[64], n_capable = 0;
    for (int i = 0; i < num_devices; i++) {
        int major = attr(i, HG_DEVICE_ATTRIBUTE_COMPUTE_CAPABILITY_MAJOR);
        if (major >= 2) capable[n_capable++] = i;
    }
    if (n_capable < 2) return;

    /* Build access matrix */
    int matrix[64][64];
    for (int i = 0; i < n_capable; i++) {
        for (int j = 0; j < n_capable; j++) {
            if (capable[i] == capable[j]) {
                matrix[i][j] = -1;
            } else {
                int can_access;
                checkHggcErrors(hgDeviceCanAccessPeer(&can_access, capable[i], capable[j]));
                matrix[i][j] = can_access;
            }
        }
    }

    /* Print matrix */
    printf("\n══ P2P Access Matrix ══════════════════════════════\n");
    printf("  (row→col: can row access col?)\n\n");
    printf("         ");
    for (int j = 0; j < n_capable; j++)
        printf("D%-3d ", capable[j]);
    printf("\n");

    for (int i = 0; i < n_capable; i++) {
        printf("  D%-3d   ", capable[i]);
        for (int j = 0; j < n_capable; j++) {
            if (matrix[i][j] < 0)
                printf("  -  ");
            else
                printf("  %c  ", matrix[i][j] ? 'Y' : 'N');
        }
        printf("\n");
    }
    printf("══════════════════════════════════════════════════\n");
}

/* ── Main ──────────────────────────────────────────────────── */

int main(int argc, char **argv)
{
    printf("[device_query_drv] PPU Device Property Inspector (Driver API)\n");

    checkHggcErrors(hgInit(0));

    int num_devices = 0;
    checkHggcErrors(hgDeviceGetCount(&num_devices));

    if (num_devices == 0) {
        printf("  No HGGC-capable devices found.\n");
        printf("Result = FAIL\n");
        return EXIT_FAILURE;
    }

    printf("  Detected %d device(s)\n", num_devices);

    for (HGdevice d = 0; d < num_devices; d++)
        report_device(d);

    report_p2p(num_devices);

    printf("\nResult = PASS\n");
    return EXIT_SUCCESS;
}
