/*
 * Copyright (c) 2023-2026, T-HEAD (SHANGHAI) SEMICONDUCTOR CO., LTD.
 * All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Sample code demonstrating T-HEAD SAIL SDK usage. This code is provided
 * under the Apache License 2.0 for reference and educational purposes.
 *
 * jit_lto -- Mixed LTO IR + HGBIN JIT Linking Demo
 *
 * Demonstrates runtime linking of two separately compiled modules:
 *   - Module A (LTO IR):  JIT-compiled kernel calling an extern device function
 *   - Module B (HGBIN):   Pre-compiled device function implementation
 *
 * Unlike pure-LTO linking, this shows a mixed-input scenario where a
 * JIT-compiled kernel links against a pre-compiled library function —
 * a common pattern for plugin architectures and hot-patchable kernels.
 *
 * Algorithm: weighted average (out[i] = w*x[i] + (1-w)*y[i])
 * Verification: CPU reference computed dynamically
 */
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>

#include <hggc.h>
#include <hgrtc.h>
#include <hgrtc_helper.h>
#include <hgJitLink.h>

/* ── Configuration ───────────────────────────────────────────── */

#define NUM_THREADS 256
#define NUM_BLOCKS  64
#define WEIGHT      0.3f

/* ── Kernel source strings ──────────────────────────────────── */

/* Module A: kernel that calls an external device function.
 * Compiled with -dlto to produce LTO IR. */
static const char *kernel_src =
    "extern __device__ float blend(float w, float x, float y);\n"
    "\n"
    "extern \"C\" __global__\n"
    "void weighted_average(float w, const float *x, const float *y,\n"
    "                       float *out, int n) {\n"
    "    int tid = blockIdx.x * blockDim.x + threadIdx.x;\n"
    "    if (tid < n) {\n"
    "        out[tid] = blend(w, x[tid], y[tid]);\n"
    "    }\n"
    "}\n";

/* Module B: device function implementation.
 * Compiled WITHOUT -dlto to produce a regular HGBIN. */
static const char *func_src =
    "__device__ float blend(float w, float x, float y) {\n"
    "    return w * x + (1.0f - w) * y;\n"
    "}\n";

/* ── Error handling ─────────────────────────────────────────── */

#define CHECK_HG(expr)                                                \
    do {                                                              \
        HGresult _r = (expr);                                         \
        if (_r != HGGC_SUCCESS) {                                     \
            const char *_msg;                                         \
            hgGetErrorName(_r, &_msg);                                \
            std::cerr << "HG error: " #expr " -> " << _msg << '\n';   \
            exit(EXIT_FAILURE);                                       \
        }                                                             \
    } while (0)

#define CHECK_JIT(h, expr)                                              \
    do {                                                                \
        hgJitLinkResult _r = (expr);                                    \
        if (_r != HGJITLINK_SUCCESS) {                                  \
            std::cerr << "JitLink error: " #expr " -> " << _r << '\n';  \
            size_t _sz;                                                 \
            if (hgJitLinkGetErrorLogSize(h, &_sz) == HGJITLINK_SUCCESS  \
                && _sz > 0) {                                           \
                char *_log = (char *)malloc(_sz + 1);                   \
                if (hgJitLinkGetErrorLog(h, _log) == HGJITLINK_SUCCESS) \
                    std::cerr << "  log: " << _log << '\n';              \
                free(_log);                                             \
            }                                                           \
            exit(EXIT_FAILURE);                                         \
        }                                                               \
    } while (0)

/* ── Compilation helpers ────────────────────────────────────── */

/* Compile source to LTO IR (for cross-module optimization at link time) */
static void compile_to_lto_ir(const char *src, const char *name,
                              char **ir_out, size_t *size_out)
{
    hgrtcProgram prog;
    HGRTC_CHECK("hgrtcCreateProgram",
                hgrtcCreateProgram(&prog, src, name, 0, NULL, NULL));

    const char *opts[] = {"-dlto", "--relocatable-device-code=true"};
    hgrtcResult rc = hgrtcCompileProgram(prog, 2, opts);

    /* Print compilation log */
    size_t log_sz;
    HGRTC_CHECK("hgrtcGetProgramLogSize", hgrtcGetProgramLogSize(prog, &log_sz));
    if (log_sz > 0) {
        char *log = new char[log_sz];
        HGRTC_CHECK("hgrtcGetProgramLog", hgrtcGetProgramLog(prog, log));
        std::cout << "  [" << name << "] compile log: " << log << '\n';
        delete[] log;
    }
    if (rc != HGRTC_SUCCESS) {
        std::cerr << "  LTO compilation failed for " << name << '\n';
        exit(EXIT_FAILURE);
    }

    HGRTC_CHECK("hgrtcGetLTOIRSize", hgrtcGetLTOIRSize(prog, size_out));
    *ir_out = new char[*size_out];
    HGRTC_CHECK("hgrtcGetLTOIR", hgrtcGetLTOIR(prog, *ir_out));
    HGRTC_CHECK("hgrtcDestroyProgram", hgrtcDestroyProgram(&prog));
}

/* Compile source to HGBIN (regular compilation, no LTO) */
static void compile_to_hgbin(const char *src, const char *name,
                             char **bin_out, size_t *size_out)
{
    hgrtcProgram prog;
    HGRTC_CHECK("hgrtcCreateProgram",
                hgrtcCreateProgram(&prog, src, name, 0, NULL, NULL));

    /* No -dlto, but with RDC for cross-module symbol visibility */
    const char *opts[] = {"--relocatable-device-code=true"};
    hgrtcResult rc = hgrtcCompileProgram(prog, 1, opts);

    size_t log_sz;
    HGRTC_CHECK("hgrtcGetProgramLogSize", hgrtcGetProgramLogSize(prog, &log_sz));
    if (log_sz > 0) {
        char *log = new char[log_sz];
        HGRTC_CHECK("hgrtcGetProgramLog", hgrtcGetProgramLog(prog, log));
        std::cout << "  [" << name << "] compile log: " << log << '\n';
        delete[] log;
    }
    if (rc != HGRTC_SUCCESS) {
        std::cerr << "  HGBIN compilation failed for " << name << '\n';
        exit(EXIT_FAILURE);
    }

    HGRTC_CHECK("hgrtcGetHGBINSize", hgrtcGetHGBINSize(prog, size_out));
    *bin_out = new char[*size_out];
    HGRTC_CHECK("hgrtcGetHGBIN", hgrtcGetHGBIN(prog, *bin_out));
    HGRTC_CHECK("hgrtcDestroyProgram", hgrtcDestroyProgram(&prog));
}

/* ── Main ────────────────────────────────────────────────────── */

int main(int argc, char *argv[])
{
    std::cout << "[jit_lto] Mixed LTO IR + HGBIN JIT Linking Demo\n";
    std::cout << "  Algorithm: weighted average (w=" << WEIGHT << ")\n\n";

    /* Print JitLink version */
    const char *version = nullptr;
    if (hgJitLinkVersion(&version) == HGJITLINK_SUCCESS)
        std::cout << "  JitLink version: " << version << "\n\n";

    /* ── Step 1: Compile both modules ── */
    std::cout << "Step 1: Compiling modules\n";
    char *lto_ir;
    size_t lto_ir_size;
    compile_to_lto_ir(kernel_src, "weighted_average.hg", &lto_ir, &lto_ir_size);
    std::cout << "  Module A (LTO IR): " << lto_ir_size << " bytes\n";

    char *hgbin;
    size_t hgbin_size;
    compile_to_hgbin(func_src, "blend.hg", &hgbin, &hgbin_size);
    std::cout << "  Module B (HGBIN):  " << hgbin_size << " bytes\n\n";

    /* ── Step 2: Initialize device ── */
    std::cout << "Step 2: Initialize device\n";
    CHECK_HG(hgInit(0));

    HGdevice device;
    CHECK_HG(hgDeviceGet(&device, 0));

    int major = 0, minor = 0;
    CHECK_HG(hgDeviceGetAttribute(&major,
        HG_DEVICE_ATTRIBUTE_COMPUTE_CAPABILITY_MAJOR, device));
    CHECK_HG(hgDeviceGetAttribute(&minor,
        HG_DEVICE_ATTRIBUTE_COMPUTE_CAPABILITY_MINOR, device));

    const char *arch = "ppu001";
    int cc = major * 10 + minor;
    if (cc == 89) arch = "ppu0015";

    char arch_opt[32];
    snprintf(arch_opt, sizeof(arch_opt), "--arch=%s", arch);
    std::cout << "  Target arch: " << arch << " (cc " << major << "." << minor << ")\n\n";

    /* ── Step 3: Mixed linking (LTO IR + HGBIN) ── */
    std::cout << "Step 3: Linking LTO IR + HGBIN\n";
    HGcontext context;
#if HGGC_VERSION >= 13000
    CHECK_HG(hgCtxCreate(&context, NULL, 0, device));
#else
    CHECK_HG(hgCtxCreate(&context, 0, device));
#endif

    hgJitLinkHandle handle;
    const char *link_opts[] = {"-lto", arch_opt};
    CHECK_JIT(handle, hgJitLinkCreate(&handle, 2, link_opts));

    /* Add Module A as LTO IR */
    CHECK_JIT(handle, hgJitLinkAddData(handle,
        HGJITLINK_INPUT_LTOIR, lto_ir, lto_ir_size, "weighted_average"));
    std::cout << "  Added LTO IR module (kernel)\n";

    /* Add Module B as HGBIN — the key difference from pure-LTO demos */
    CHECK_JIT(handle, hgJitLinkAddData(handle,
        HGJITLINK_INPUT_HGBIN, hgbin, hgbin_size, "blend"));
    std::cout << "  Added HGBIN module (device function)\n";

    CHECK_JIT(handle, hgJitLinkComplete(handle));
    std::cout << "  Link complete\n";

    /* Retrieve linked HGBIN */
    size_t linked_size;
    CHECK_JIT(handle, hgJitLinkGetLinkedHgbinSize(handle, &linked_size));
    void *linked_bin = malloc(linked_size);
    CHECK_JIT(handle, hgJitLinkGetLinkedHgbin(handle, linked_bin));
    CHECK_JIT(handle, hgJitLinkDestroy(&handle));

    delete[] lto_ir;
    delete[] hgbin;

    std::cout << "  Linked HGBIN: " << linked_size << " bytes\n\n";

    /* ── Step 4: Load and execute ── */
    std::cout << "Step 4: Load and execute kernel\n";
    HGmodule module;
    HGfunction kernel;
    CHECK_HG(hgModuleLoadData(&module, linked_bin));
    CHECK_HG(hgModuleGetFunction(&kernel, module, "weighted_average"));

    int n = NUM_THREADS * NUM_BLOCKS;
    size_t buf_sz = n * sizeof(float);
    float w = WEIGHT;

    /* Prepare input data */
    float *h_x = new float[n];
    float *h_y = new float[n];
    float *h_out = new float[n];
    for (int i = 0; i < n; i++) {
        h_x[i] = static_cast<float>(i);
        h_y[i] = static_cast<float>(i * 2);
    }

    HGdeviceptr d_x, d_y, d_out;
    CHECK_HG(hgMemAlloc(&d_x, buf_sz));
    CHECK_HG(hgMemAlloc(&d_y, buf_sz));
    CHECK_HG(hgMemAlloc(&d_out, buf_sz));
    CHECK_HG(hgMemcpyHtoD(d_x, h_x, buf_sz));
    CHECK_HG(hgMemcpyHtoD(d_y, h_y, buf_sz));

    /* Launch kernel */
    void *args[] = {&w, &d_x, &d_y, &d_out, &n};
    CHECK_HG(hgLaunchKernel(kernel,
        NUM_BLOCKS, 1, 1,
        NUM_THREADS, 1, 1,
        0, NULL, args, 0));
    CHECK_HG(hgCtxSynchronize());

    CHECK_HG(hgMemcpyDtoH(h_out, d_out, buf_sz));

    /* ── Step 5: Verify with CPU reference ── */
    std::cout << "\nStep 5: Verification\n";
    float max_err = 0.0f;
    int mismatches = 0;
    for (int i = 0; i < n; i++) {
        float expected = w * h_x[i] + (1.0f - w) * h_y[i];
        float err = fabsf(h_out[i] - expected);
        /* Use relative error for large values */
        float rel = err / fmaxf(fabsf(expected), 1.0f);
        if (rel > max_err) max_err = rel;
        if (rel > 1e-5f && mismatches < 5) {
            std::cout << "  mismatch[" << mismatches << "]: out[" << i
                      << "] got " << h_out[i] << " expected " << expected << '\n';
            mismatches++;
        }
    }

    std::cout << "\n  First 5 results:\n";
    for (int i = 0; i < 5 && i < n; i++) {
        float expected = w * h_x[i] + (1.0f - w) * h_y[i];
        std::cout << "    out[" << i << "] = " << h_out[i]
                  << " (expected " << expected << ")\n";
    }
    std::cout << "  ...\n  Last result: out[" << n-1 << "] = " << h_out[n-1]
              << " (expected " << (w * h_x[n-1] + (1.0f-w) * h_y[n-1]) << ")\n";

    bool pass = (max_err < 1e-5f);
    std::cout << "\n  Max relative error: " << max_err << '\n';
    std::cout << "  Result: " << (pass ? "PASS" : "FAIL") << '\n';

    /* ── Cleanup ── */
    CHECK_HG(hgMemFree(d_x));
    CHECK_HG(hgMemFree(d_y));
    CHECK_HG(hgMemFree(d_out));
    CHECK_HG(hgModuleUnload(module));
    CHECK_HG(hgCtxDestroy(context));
    free(linked_bin);
    delete[] h_x;
    delete[] h_y;
    delete[] h_out;

    return pass ? EXIT_SUCCESS : EXIT_FAILURE;
}
