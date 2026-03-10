/*
 * ab_ds_common.h — common comparators, equality functions, and hash functions
 * Part of ab_ds — https://github.com/UniquePython/ab_ds
 *
 * USAGE
 *   #include "ab_ds_common.h"
 *
 *   No implementation guard needed — everything here is either a macro or
 *   a static inline function.
 *
 * PROVIDED
 *   Equality (for AB_AL_DEFINE, AB_SL_DEFINE, AB_DL_DEFINE, AB_DQ_DEFINE,
 *             AB_HS_DEFINE, AB_LHS_DEFINE):
 *     ab_eq_i8,  ab_eq_i16,  ab_eq_i32,  ab_eq_i64
 *     ab_eq_u8,  ab_eq_u16,  ab_eq_u32,  ab_eq_u64
 *     ab_eq_f32, ab_eq_f64
 *     ab_eq_char, ab_eq_ptr
 *     ab_eq_str   (null-safe strcmp)
 *
 *   Comparators (for AB_SS_DEFINE, AB_TS_DEFINE):
 *     ab_cmp_i8,  ab_cmp_i16,  ab_cmp_i32,  ab_cmp_i64
 *     ab_cmp_u8,  ab_cmp_u16,  ab_cmp_u32,  ab_cmp_u64
 *     ab_cmp_f32, ab_cmp_f64
 *     ab_cmp_char, ab_cmp_ptr
 *     ab_cmp_str   (strcmp)
 *
 *   Hash functions (for AB_HS_DEFINE, AB_LHS_DEFINE):
 *     ab_hash_i8,  ab_hash_i16,  ab_hash_i32,  ab_hash_i64
 *     ab_hash_u8,  ab_hash_u16,  ab_hash_u32,  ab_hash_u64
 *     ab_hash_char, ab_hash_ptr
 *     ab_hash_str  (FNV-1a)
 *
 * LICENSE — MIT
 *   Copyright (c) 2025 ab_ds contributors
 *   Permission is hereby granted, free of charge, to any person obtaining
 *   a copy of this software and associated documentation files (the
 *   "Software"), to deal in the Software without restriction, including
 *   without limitation the rights to use, copy, modify, merge, publish,
 *   distribute, sublicense, and/or sell copies of the Software, and to
 *   permit persons to whom the Software is furnished to do so, subject to
 *   the following conditions: The above copyright notice and this
 *   permission notice shall be included in all copies or substantial
 *   portions of the Software. THE SOFTWARE IS PROVIDED "AS IS", WITHOUT
 *   WARRANTY OF ANY KIND.
 *
 * VERSION — 0.1.0
 */

#ifndef AB_DS_COMMON_H
#define AB_DS_COMMON_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>

/* -------------------------------------------------------------------------
 * Equality macros
 * Returns bool. Used by: AL, SL, DL, DQ, HS, LHS.
 * ------------------------------------------------------------------------- */

#define ab_eq_i8(a, b) ((a) == (b))
#define ab_eq_i16(a, b) ((a) == (b))
#define ab_eq_i32(a, b) ((a) == (b))
#define ab_eq_i64(a, b) ((a) == (b))

#define ab_eq_u8(a, b) ((a) == (b))
#define ab_eq_u16(a, b) ((a) == (b))
#define ab_eq_u32(a, b) ((a) == (b))
#define ab_eq_u64(a, b) ((a) == (b))

#define ab_eq_char(a, b) ((a) == (b))
#define ab_eq_ptr(a, b) ((a) == (b))

/* Floating point: exact bitwise equality. For epsilon comparison,
 * define your own: #define my_f32_eq(a, b) (fabsf((a)-(b)) < 1e-6f) */
#define ab_eq_f32(a, b) ((a) == (b))
#define ab_eq_f64(a, b) ((a) == (b))

/* Null-safe string equality */
static inline bool ab_eq_str(const char *a, const char *b)
{
    if (a == b)
        return true;
    if (!a || !b)
        return false;
    return strcmp(a, b) == 0;
}

/* -------------------------------------------------------------------------
 * Comparator functions
 * Returns int: <0 if a<b, 0 if a==b, >0 if a>b.
 * Used by: SS, TS.
 * ------------------------------------------------------------------------- */

static inline int ab_cmp_i8(int8_t a, int8_t b) { return (a > b) - (a < b); }
static inline int ab_cmp_i16(int16_t a, int16_t b) { return (a > b) - (a < b); }
static inline int ab_cmp_i32(int32_t a, int32_t b) { return (a > b) - (a < b); }
static inline int ab_cmp_i64(int64_t a, int64_t b) { return (a > b) - (a < b); }

static inline int ab_cmp_u8(uint8_t a, uint8_t b) { return (a > b) - (a < b); }
static inline int ab_cmp_u16(uint16_t a, uint16_t b) { return (a > b) - (a < b); }
static inline int ab_cmp_u32(uint32_t a, uint32_t b) { return (a > b) - (a < b); }
static inline int ab_cmp_u64(uint64_t a, uint64_t b) { return (a > b) - (a < b); }

static inline int ab_cmp_char(char a, char b) { return (a > b) - (a < b); }
static inline int ab_cmp_ptr(const void *a, const void *b) { return (a > b) - (a < b); }

static inline int ab_cmp_f32(float a, float b) { return (a > b) - (a < b); }
static inline int ab_cmp_f64(double a, double b) { return (a > b) - (a < b); }

static inline int ab_cmp_str(const char *a, const char *b)
{
    if (a == b)
        return 0;
    if (!a)
        return -1;
    if (!b)
        return 1;
    return strcmp(a, b);
}

/* -------------------------------------------------------------------------
 * Hash functions
 * Returns size_t. Used by: HS, LHS.
 * All integer hashes use Knuth multiplicative hashing.
 * String hash uses FNV-1a.
 * ------------------------------------------------------------------------- */

static inline size_t ab_hash_u8(uint8_t v) { return (size_t)v * 2654435761UL; }
static inline size_t ab_hash_u16(uint16_t v) { return (size_t)v * 2654435761UL; }
static inline size_t ab_hash_u32(uint32_t v) { return (size_t)v * 2654435761UL; }
static inline size_t ab_hash_u64(uint64_t v)
{
    v ^= v >> 33;
    v *= 0xff51afd7ed558ccdULL;
    v ^= v >> 33;
    v *= 0xc4ceb9fe1a85ec53ULL;
    v ^= v >> 33;
    return (size_t)v;
}

static inline size_t ab_hash_i8(int8_t v) { return ab_hash_u8((uint8_t)v); }
static inline size_t ab_hash_i16(int16_t v) { return ab_hash_u16((uint16_t)v); }
static inline size_t ab_hash_i32(int32_t v) { return ab_hash_u32((uint32_t)v); }
static inline size_t ab_hash_i64(int64_t v) { return ab_hash_u64((uint64_t)v); }

static inline size_t ab_hash_char(char v) { return ab_hash_u8((uint8_t)v); }
static inline size_t ab_hash_ptr(const void *v) { return ab_hash_u64((uint64_t)(uintptr_t)v); }

static inline size_t ab_hash_str(const char *s)
{
    if (!s)
        return 0;
    size_t h = 14695981039346656037ULL;
    while (*s)
    {
        h ^= (unsigned char)*s++;
        h *= 1099511628211ULL;
    }
    return h;
}

#endif