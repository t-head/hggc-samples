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

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <string>

#include <strings.h>  // strcasecmp / strncasecmp on POSIX

// ---------------------------------------------------------------------------
//  Cross-platform shim macros — pinned for source-level ABI parity.
// ---------------------------------------------------------------------------
#ifndef STRCASECMP
#  define STRCASECMP strcasecmp        /* POSIX */
#endif  /* STRCASECMP */
#ifndef STRNCASECMP
#  define STRNCASECMP strncasecmp      /* POSIX */
#endif  /* STRNCASECMP */
#ifndef STRCPY
#  define STRCPY(dst, n, src) std::strcpy((dst), (src))   /* size param ignored on POSIX */
#endif  /* STRCPY */

#ifndef FOPEN
#define FOPEN(handle, path, mode) ((handle) = std::fopen((path), (mode)))
#endif
#ifndef FOPEN_FAIL
#define FOPEN_FAIL(result) ((result) == nullptr)
#endif
#ifndef SSCANF
#define SSCANF std::sscanf
#endif
#ifndef SPRINTF
#define SPRINTF std::sprintf
#endif

#ifndef EXIT_SKIPPED
#define EXIT_SKIPPED 2
#endif

// ===========================================================================
//  Internal helpers (not part of the public ABI).
// ===========================================================================
namespace hggc_str_detail {

/// Length of the @c "<sample_name>" placeholder used inside search paths.
constexpr std::size_t kSamplePlaceholderLen = 13;  // strlen("<sample_name>")

/// Strip the leading run of @p delim characters from @p text. Returns the
/// offset of the first non-delimiter character or zero when the whole string
/// (apart from the trailing NUL) is delimiters.
inline int skip_leading(char delim, const char *text) {
    if (text == nullptr) {
        return 0;
    }
    int idx = 0;
    while (text[idx] == delim) {
        ++idx;
    }
    const int total = static_cast<int>(std::strlen(text));
    return (idx >= total - 1) ? 0 : idx;
}

/// Case-insensitive equality of the first @p n bytes of two C strings.
inline bool iequal_n(const char *a, const char *b, int n) {
    return STRNCASECMP(a, b, static_cast<std::size_t>(n)) == 0;
}

/// Heap-duplicate @p s using the C allocator so callers may free() the result.
/// Returns @c nullptr when @p s is empty/null.
inline char *strdup_c(const std::string &s) {
    if (s.empty()) {
        return nullptr;
    }
    char *out = static_cast<char *>(std::malloc(s.size() + 1));
    if (out != nullptr) {
        STRCPY(out, s.size() + 1, s.c_str());
    }
    return out;
}

/// Try to open @p path read-binary; return true if successful (and close it).
inline bool exists(const std::string &path) {
    std::FILE *fp = nullptr;
    FOPEN(fp, path.c_str(), "rb");
    if (FOPEN_FAIL(fp)) {
        return false;
    }
    std::fclose(fp);
    return true;
}

/// Locate the directory portion (including trailing slash) of @p exe.
inline std::string exec_dir_of(const char *exe) {
    if (exe == nullptr) {
        return "./";
    }
    std::string p(exe);
    const auto pos = p.find_last_of('/');
    if (pos == std::string::npos) {
        return "./";
    }
    return p.substr(0, pos + 1);
}

/// Locate the file-name portion (without directory) of @p exe.
inline std::string exec_name_of(const char *exe) {
    if (exe == nullptr) {
        return {};
    }
    std::string p(exe);
    const auto pos = p.find_last_of('/');
    if (pos == std::string::npos) {
        return p;
    }
    return p.substr(pos + 1);
}

/// Generic key=value extractor used by both the typed and string overloads of
/// the public getArg* family. Returns true if @p key was matched
/// and assigns the raw value pointer to @p out_value (which may point inside
/// @p argv elements). When the key is matched without a value, @p out_value is
/// set to a pointer past the key (caller decides the default).
inline bool find_argument(const int argc,
                          const char **argv,
                          const char *key,
                          const char **out_value,
                          int *out_value_offset) {
    if (argv == nullptr || key == nullptr || argc <= 1) {
        return false;
    }
    const int key_len = static_cast<int>(std::strlen(key));
    for (int i = 1; i < argc; ++i) {
        const int lead = skip_leading('-', argv[i]);
        const char *cursor = argv[i] + lead;
        if (!iequal_n(cursor, key, key_len)) {
            continue;
        }
        const int cur_len = static_cast<int>(std::strlen(cursor));
        if (key_len + 1 <= cur_len) {
            const int adj = (cursor[key_len] == '=') ? 1 : 0;
            *out_value = cursor + key_len + adj;
            *out_value_offset = key_len + adj;
        } else {
            *out_value = nullptr;
            *out_value_offset = key_len;
        }
        return true;
    }
    return false;
}

}  // namespace hggc_str_detail

// ===========================================================================
//  Public free-function API (signatures frozen).
// ===========================================================================

/// Return true if the boolean-style flag @p string_ref is present in @p argv.
inline bool hasArg(const int argc,
                             const char **argv,
                             const char *string_ref) {
    if (argv == nullptr || string_ref == nullptr || argc < 1) {
        return false;
    }
    const int ref_len = static_cast<int>(std::strlen(string_ref));
    bool hit = false;
    for (int i = 1; i < argc; ++i) {
        const int lead = hggc_str_detail::skip_leading('-', argv[i]);
        const char *cursor = argv[i] + lead;
        const char *eq = std::strchr(cursor, '=');
        const int cursor_len = static_cast<int>(
            eq == nullptr ? std::strlen(cursor) : eq - cursor);
        if (cursor_len == ref_len &&
            hggc_str_detail::iequal_n(cursor, string_ref, ref_len)) {
            hit = true;
        }
    }
    return hit;
}

/// Integer extractor. Returns the parsed value, or 0 when key absent / has no
/// payload (matches historical behaviour).
inline int getArgInt(const int argc,
                                 const char **argv,
                                 const char *string_ref) {
    const char *raw = nullptr;
    int off = 0;
    if (!hggc_str_detail::find_argument(argc, argv, string_ref, &raw, &off)) {
        return 0;
    }
    return (raw != nullptr && *raw != '\0') ? std::atoi(raw) : 0;
}

/// String extractor. On match @c *string_retval is set to a pointer inside
/// @p argv (one past the key + optional '='). On miss @c *string_retval is
/// set to @c nullptr.
inline bool getArgStr(const int argc,
                                     const char **argv,
                                     const char *string_ref,
                                     char **string_retval) {
    if (string_retval == nullptr) {
        return false;
    }
    if (argv == nullptr || string_ref == nullptr || argc < 1) {
        *string_retval = nullptr;
        return false;
    }
    const int ref_len = static_cast<int>(std::strlen(string_ref));
    bool hit = false;
    for (int i = 1; i < argc; ++i) {
        const int lead = hggc_str_detail::skip_leading('-', argv[i]);
        char *cursor = const_cast<char *>(argv[i] + lead);
        if (hggc_str_detail::iequal_n(cursor, string_ref, ref_len)) {
            *string_retval = cursor + ref_len + 1;
            hit = true;
        }
    }
    if (!hit) {
        *string_retval = nullptr;
    }
    return hit;
}

// ===========================================================================
//  Companion-file resolver (findSampleAsset)
//
//  Walks a curated set of relative search prefixes anchored either at the
//  executable directory or at the project's Samples/ tree, expanding the
//  placeholder "<sample_name>" with the actual binary name extracted from
//  @p executable_path. Returns a heap-allocated C string the caller must
//  release with free().
// ===========================================================================
inline char *findSampleAsset(const char *filename,
                             const char *executable_path) {
    if (filename == nullptr) {
        return nullptr;
    }

    const std::string exec_dir = hggc_str_detail::exec_dir_of(executable_path);
    const std::string exec_name = hggc_str_detail::exec_name_of(executable_path);

    // 1) Direct lookup next to the executable, then in its data/ subfolder.
    {
        const std::string direct = exec_dir + filename;
        if (hggc_str_detail::exists(direct)) {
            return hggc_str_detail::strdup_c(direct);
        }
        const std::string in_data = exec_dir + "data/" + filename;
        if (hggc_str_detail::exists(in_data)) {
            return hggc_str_detail::strdup_c(in_data);
        }
    }

    // 2) Walk the curated relative prefixes. Each entry may contain
    //    "<sample_name>" — substituted with @p exec_name (or skipped
    //    when no executable path was supplied).
    static const char *kSearchPrefixes[] = {
        "./",
        "./data/",

        "../../../../Samples/<sample_name>/",
        "../../../Samples/<sample_name>/",
        "../../Samples/<sample_name>/",

        "../../../../Samples/<sample_name>/data/",
        "../../../Samples/<sample_name>/data/",
        "../../Samples/<sample_name>/data/",

        "../../../../Samples/0_Introduction/<sample_name>/",
        "../../../Samples/0_Introduction/<sample_name>/",
        "../../Samples/0_Introduction/<sample_name>/",

        "../../../../Samples/1_Utilities/<sample_name>/",
        "../../../Samples/1_Utilities/<sample_name>/",
        "../../Samples/1_Utilities/<sample_name>/",

        "../../../../Samples/2_Algorithms_and_Techniques/<sample_name>/",
        "../../../Samples/2_Algorithms_and_Techniques/<sample_name>/",
        "../../Samples/2_Algorithms_and_Techniques/<sample_name>/",

        "../../../../Samples/3_HGGC_Features/<sample_name>/",
        "../../../Samples/3_HGGC_Features/<sample_name>/",
        "../../Samples/3_HGGC_Features/<sample_name>/",

        "../../../../Samples/4_HGGC_Libraries/<sample_name>/",
        "../../../Samples/4_HGGC_Libraries/<sample_name>/",
        "../../Samples/4_HGGC_Libraries/<sample_name>/",

        "../../../../Samples/5_Performance/<sample_name>/",
        "../../../Samples/5_Performance/<sample_name>/",
        "../../Samples/5_Performance/<sample_name>/",

        "../../../../Samples/0_Introduction/<sample_name>/data/",
        "../../../Samples/0_Introduction/<sample_name>/data/",
        "../../Samples/0_Introduction/<sample_name>/data/",

        "../../../../Samples/1_Utilities/<sample_name>/data/",
        "../../../Samples/1_Utilities/<sample_name>/data/",
        "../../Samples/1_Utilities/<sample_name>/data/",

        "../../../../Samples/2_Algorithms_and_Techniques/<sample_name>/data/",
        "../../../Samples/2_Algorithms_and_Techniques/<sample_name>/data/",
        "../../Samples/2_Algorithms_and_Techniques/<sample_name>/data/",

        "../../../../Samples/3_HGGC_Features/<sample_name>/data/",
        "../../../Samples/3_HGGC_Features/<sample_name>/data/",
        "../../Samples/3_HGGC_Features/<sample_name>/data/",

        "../../../../Samples/4_HGGC_Libraries/<sample_name>/data/",
        "../../../Samples/4_HGGC_Libraries/<sample_name>/data/",
        "../../Samples/4_HGGC_Libraries/<sample_name>/data/",

        "../../../../Samples/5_Performance/<sample_name>/data/",
        "../../../Samples/5_Performance/<sample_name>/data/",
        "../../Samples/5_Performance/<sample_name>/data/",

        "../../../../Common/data/",
        "../../../Common/data/",
        "../../Common/data/",
    };

    constexpr std::size_t kPrefixCount =
        sizeof(kSearchPrefixes) / sizeof(kSearchPrefixes[0]);

    for (std::size_t i = 0; i < kPrefixCount; ++i) {
        std::string candidate(kSearchPrefixes[i]);
        const auto holder = candidate.find("<sample_name>");
        if (holder != std::string::npos) {
            if (exec_name.empty()) {
                continue;  // cannot expand the placeholder
            }
            candidate.replace(holder,
                              hggc_str_detail::kSamplePlaceholderLen,
                              exec_name);
        }

#ifdef _DEBUG
        std::printf("findSampleAsset <%s> in %s\n", filename, candidate.c_str());
#endif

        candidate.append(filename);
        if (hggc_str_detail::exists(candidate)) {
            return hggc_str_detail::strdup_c(candidate);
        }
    }

    std::printf("\nerror: findSampleAsset: file <%s> not found!\n", filename);
    return nullptr;
}
