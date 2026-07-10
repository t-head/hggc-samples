/*
 * Copyright (c) 2023-2026, T-HEAD (SHANGHAI) SEMICONDUCTOR CO., LTD.
 * All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Sample code demonstrating T-HEAD SAIL SDK usage. This code is provided
 * under the Apache License 2.0 for reference and educational purposes.
 */
#include "multi_device_mmap.hpp"

namespace {

/// Round @p value up to the nearest multiple of @p step.
inline size_t align_up(size_t value, size_t step)
{
    return ((value + step - 1) / step) * step;
}

/// Query the maximum minimum-granularity across a set of devices.
HGresult query_max_min_granularity(const std::vector<HGdevice> &devices,
                                   HGmemAllocationProp         &prop,
                                   size_t                      &out_granularity)
{
    for (size_t idx = 0; idx < devices.size(); ++idx) {
        size_t granularity = 0;
        prop.location.id   = devices[idx];
        HGresult st = hgMemGetAllocationGranularity(&granularity, &prop, HG_MEM_ALLOC_GRANULARITY_MINIMUM);
        if (st != HGGC_SUCCESS) {
            return st;
        }
        if (out_granularity < granularity) {
            out_granularity = granularity;
        }
    }
    return HGGC_SUCCESS;
}

/// Create a pinned chunk on device @p dev_id and map it at @p va.
HGresult create_and_map_stripe(HGdeviceptr           va,
                               size_t                stripe_size,
                               HGdevice              dev_id,
                               HGmemAllocationProp  &prop)
{
    prop.location.id = dev_id;

    HGmemGenericAllocationHandle handle;
    HGresult st = hgMemCreate(&handle, stripe_size, &prop, 0);
    if (st != HGGC_SUCCESS) {
        return st;
    }

    HGresult map_st     = hgMemMap(va, stripe_size, 0, handle, 0);
    HGresult release_st = hgMemRelease(handle);
    if (map_st != HGGC_SUCCESS) {
        return map_st;
    }
    return release_st;
}

}  // namespace

HGresult multi_device_mmap_alloc(HGdeviceptr                 *dev_ptr,
                                 size_t                      *allocation_size,
                                 size_t                       size,
                                 const std::vector<HGdevice> &backing_devices,
                                 const std::vector<HGdevice> &mapping_devices,
                                 size_t                       align)
{
    if (dev_ptr == nullptr || backing_devices.empty()) {
        return HGGC_ERROR_INVALID_VALUE;
    }

    HGmemAllocationProp prop = {};
    prop.type                = HG_MEM_ALLOCATION_TYPE_PINNED;
    prop.location.type       = HG_MEM_LOCATION_TYPE_DEVICE;

    // Granularity = max over all participating devices.
    size_t   min_granularity = 0;
    HGresult status          = query_max_min_granularity(backing_devices, prop, min_granularity);
    if (status != HGGC_SUCCESS) {
        return status;
    }
    status = query_max_min_granularity(mapping_devices, prop, min_granularity);
    if (status != HGGC_SUCCESS) {
        return status;
    }

    // Round so the buffer can be split into N equal stripes that each meet the
    // minimum granularity requirement.
    const size_t stripe_count = backing_devices.size();
    size_t       total_size   = align_up(size, stripe_count * min_granularity);
    const size_t stripe_size  = total_size / stripe_count;

    if (allocation_size != nullptr) {
        *allocation_size = total_size;
    }

    // Reserve a contiguous VA range to hold all the stripes.
    *dev_ptr = 0;
    status   = hgMemAddressReserve(dev_ptr, total_size, align, 0, 0);
    if (status != HGGC_SUCCESS) {
        return status;
    }

    // Place each backing stripe and release the temporary handle.
    for (size_t idx = 0; idx < stripe_count; ++idx) {
        HGdeviceptr va = *dev_ptr + stripe_size * idx;
        status = create_and_map_stripe(va, stripe_size, backing_devices[idx], prop);
        if (status != HGGC_SUCCESS) {
            multi_device_mmap_free(*dev_ptr, total_size);
            return status;
        }
    }

    // Build access descriptors for every mapping device.
    std::vector<HGmemAccessDesc> access_desc(mapping_devices.size());
    for (size_t idx = 0; idx < mapping_devices.size(); ++idx) {
        access_desc[idx].location.type = HG_MEM_LOCATION_TYPE_DEVICE;
        access_desc[idx].location.id   = mapping_devices[idx];
        access_desc[idx].flags         = HG_MEM_ACCESS_FLAGS_PROT_READWRITE;
    }

    status = hgMemSetAccess(*dev_ptr, total_size, access_desc.data(), access_desc.size());
    if (status != HGGC_SUCCESS) {
        multi_device_mmap_free(*dev_ptr, total_size);
        return status;
    }

    return HGGC_SUCCESS;
}

HGresult multi_device_mmap_free(HGdeviceptr dev_ptr, size_t size)
{
    if (dev_ptr == 0) {
        return HGGC_SUCCESS;
    }

    HGresult unmap_st = hgMemUnmap(dev_ptr, size);
    if (unmap_st != HGGC_SUCCESS) {
        return unmap_st;
    }

    return hgMemAddressFree(dev_ptr, size);
}
