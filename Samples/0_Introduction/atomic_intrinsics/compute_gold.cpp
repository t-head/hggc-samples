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
        unsigned int seed = 0;
        for (int i = 0; i < len; ++i) seed |= (1u << (i % 32));
        if (static_cast<unsigned int>(ppuData[kSlotOr]) != seed)
            return report_mismatch("atomicOr");
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

bool computeUllGold(const UllResults &results) {
    unsigned long long expected[kUllSlotCount] = {
        kUllBase, ~0ULL, 0, kUllSeed, ~0ULL, 0, 0, kUllBase};
    for (unsigned int tid = 0; tid < kUllThreads; ++tid) {
        expected[kUllAdd] += kUllIncrement;
        expected[kUllAnd] &= ~(1ULL << (tid % 64));
        expected[kUllOr] |= 1ULL << (tid % 64);
        expected[kUllXor] ^= kUllBase | tid;
        expected[kUllMin] = std::min(expected[kUllMin], kUllBase + tid);
        expected[kUllMax] = std::max(expected[kUllMax], kUllBase + tid);
    }
    unsigned long long value = kUllSeed;
    unsigned long long old[kUllOldCount];
    old[0] = value; value += kUllIncrement;
    old[1] = value; value &= kUllMask;
    old[2] = value; value |= kUllOrBits;
    old[3] = value; value ^= kUllXorBits;
    old[4] = value; value = std::min(value, kUllBase);
    old[5] = value; value = std::max(value, kUllSeed);
    old[6] = value; value = kUllBase;
    old[7] = value; value = kUllSeed;
    old[8] = value;
    bool ok = true;
    for (int space = 0; space < 2; ++space) {
        for (int slot = 0; slot < kUllSlotCount; ++slot) {
            const auto actual = results.concurrent[space][slot];
            const bool match = slot == kUllExch
                ? actual >= kUllBase && actual < kUllBase + kUllThreads
                : actual == expected[slot];
            if (!match) {
                std::printf("ULL concurrent space=%d slot=%d got=%016llx expected=%016llx\n",
                            space, slot, actual, expected[slot]);
                ok = false;
            }
        }
        for (int step = 0; step < kUllOldCount; ++step) {
            if (results.old[space][step] != old[step]) {
                std::printf("ULL old-value space=%d step=%d got=%016llx expected=%016llx\n",
                            space, step, results.old[space][step], old[step]);
                ok = false;
            }
        }
        if (results.final[space] != value) ok = false;
    }
    std::printf("ULL global/shared concurrent and old-value: %s\n", ok ? "PASS" : "FAIL");
    return ok;
}
