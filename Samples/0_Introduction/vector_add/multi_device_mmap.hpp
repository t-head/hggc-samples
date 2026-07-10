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

#include <hggc.h>
#include <vector>

/**
 * @brief Allocate a virtually contiguous buffer striped across @p backing_devices
 *        and accessible from @p mapping_devices.
 *
 * @param[out] dev_ptr         Reserved virtual address.
 * @param[out] allocation_size Optional output of the rounded-up byte count.
 * @param[in]  size            Minimum requested size in bytes.
 * @param[in]  backing_devices Devices that physically hold a stripe of memory.
 * @param[in]  mapping_devices Devices that will read/write through the mapping.
 * @param[in]  align           Optional VA alignment, 0 for the default.
 * @return @c HGGC_SUCCESS on success, otherwise an HGGC error code.
 */
HGresult multi_device_mmap_alloc(HGdeviceptr                 *dev_ptr,
                                 size_t                      *allocation_size,
                                 size_t                       size,
                                 const std::vector<HGdevice> &backing_devices,
                                 const std::vector<HGdevice> &mapping_devices,
                                 size_t                       align = 0);

/**
 * @brief Release the resources reserved by @c multi_device_mmap_alloc.
 *
 * @param[in] dev_ptr Virtual address obtained from @c multi_device_mmap_alloc.
 * @param[in] size    The @c allocation_size value reported by the allocation call.
 */
HGresult multi_device_mmap_free(HGdeviceptr dev_ptr, size_t size);
