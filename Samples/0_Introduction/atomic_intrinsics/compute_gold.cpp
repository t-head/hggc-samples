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
#include <algorithm>

#include "atomic_kernel.hgh"

namespace {

/// Helper to report a single failed atomic check and return false.
bool report_mismatch(const char *label) {
    std::printf("[verify] %s mismatch\n", label);
    return false;
}

/// Verify that `ppu_slots[slot]` matches the expected closed-form value.
bool check_eq(const int *ppu_slots, AtomicSlot slot, int expected, const char *label) {
    if (ppu_slots[slot] != expected) {
        return report_mismatch(label);
    }
    return true;
}

/// Verify that `ppu_slots[slot]` is a member of [0, total_threads).
bool check_member(const int *ppu_slots, AtomicSlot slot, int total_threads, const char *label) {
    int probe = ppu_slots[slot];
    if (probe < 0 || probe >= total_threads) {
        return report_mismatch(label);
    }
    return true;
}

}  /// anonymous

extern "C" bool computeGold(int *ppuData, const int len) {
    /// Bitwise group.
    {
        int seed = ATOMIC_BITWISE_SEED;
        for (int i = 0; i < len; ++i) seed &= (2 * i + 7);
        if (!check_eq(ppuData, kSlotAnd, seed, "atomicAnd")) return false;
    }
    {
        int seed = 0;
        for (int i = 0; i < len; ++i) seed |= (1 << i);
        if (!check_eq(ppuData, kSlotOr, seed, "atomicOr")) return false;
    }
    {
        int seed = ATOMIC_BITWISE_SEED;
        for (int i = 0; i < len; ++i) seed ^= i;
        if (!check_eq(ppuData, kSlotXor, seed, "atomicXor")) return false;
    }

    /// Arithmetic group.
    if (!check_eq(ppuData, kSlotAdd, 10 * len, "atomicAdd")) return false;
    if (!check_eq(ppuData, kSlotSub, -10 * len, "atomicSub")) return false;
    if (!check_member(ppuData, kSlotExch, len, "atomicExch")) return false;
    {
        int hi = -(1 << 8);
        for (int i = 0; i < len; ++i) hi = std::max(hi, i);
        if (!check_eq(ppuData, kSlotMax, hi, "atomicMax")) return false;
    }
    {
        int lo = 1 << 8;
        for (int i = 0; i < len; ++i) lo = std::min(lo, i);
        if (!check_eq(ppuData, kSlotMin, lo, "atomicMin")) return false;
    }

    /// Counter group.
    {
        int v = 0;
        for (int i = 0; i < len; ++i) v = (v >= (int)ATOMIC_INC_LIMIT) ? 0 : v + 1;
        if (!check_eq(ppuData, kSlotInc, v, "atomicInc")) return false;
    }
    {
        int v = 0;
        for (int i = 0; i < len; ++i)
            v = ((v == 0) || (v > (int)ATOMIC_DEC_LIMIT)) ? (int)ATOMIC_DEC_LIMIT : v - 1;
        if (!check_eq(ppuData, kSlotDec, v, "atomicDec")) return false;
    }
    if (!check_member(ppuData, kSlotCas, len, "atomicCAS")) return false;

    return true;
}
