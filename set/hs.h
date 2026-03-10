/*
 * hs.h — hash set for C; open addressing with linear probing
 * Part of ab_ds — https://github.com/UniquePython/ab_ds
 *
 * USAGE
 *   In ONE .c file:
 *     #define AB_HS_IMPLEMENTATION
 *     #include "hs.h"
 *
 *   Define a typed hash set:
 *     #define int_hash(x)  ((size_t)(x) * 2654435761UL)
 *     #define int_eq(a, b) ((a) == (b))
 *     AB_HS_DEFINE(int, IntHashSet, int_hash, int_eq)
 *
 *     IntHashSet hs;
 *     IntHashSet_init(&hs);
 *     IntHashSet_add(&hs, 42);
 *     IntHashSet_free(&hs);
 *
 *   For strings:
 *     static size_t str_hash(const char *s) {
 *         size_t h = 14695981039346656037ULL;
 *         while (*s) { h ^= (unsigned char)*s++; h *= 1099511628211ULL; }
 *         return h;
 *     }
 *     #define str_eq(a, b) (strcmp((a), (b)) == 0)
 *     AB_HS_DEFINE(const char*, StrHashSet, str_hash, str_eq)
 *
 * API
 *   _init(hs)                       initialise (must be called before use)
 *   _free(hs)                       free heap memory, reset to empty
 *   _add(hs, element)       bool    insert; false if already present
 *   _remove(hs, element)    bool    remove; false if not found
 *   _contains(hs, element)  bool
 *   _clear(hs)                      mark all buckets empty, keep allocation
 *   _size(hs)               size_t
 *   _is_empty(hs)           bool
 *
 * CONFIGURATION (define before including)
 *   AB_HS_INITIAL_CAPACITY   default: 8 (must be power of 2)
 *   AB_HS_MALLOC             default: malloc
 *   AB_HS_FREE               default: free
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

#ifndef AB_HS_H
#define AB_HS_H

#include <stdlib.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>

#ifndef AB_HS_MALLOC
#define AB_HS_MALLOC malloc
#endif

#ifndef AB_HS_FREE
#define AB_HS_FREE free
#endif

#ifndef AB_HS_INITIAL_CAPACITY
#define AB_HS_INITIAL_CAPACITY 8
#endif

#define AB_HS_LOAD_FACTOR_NUM 3
#define AB_HS_LOAD_FACTOR_DEN 4

#ifdef AB_HS_IMPLEMENTATION

#define AB_HS_DEFINE_FUNCTIONS(type, name, hash_fn, equals_fn)       \
                                                                     \
    static bool name##_alloc_buckets(name *hs, size_t cap)           \
    {                                                                \
        hs->buckets = AB_HS_MALLOC(cap * sizeof(name##_bucket));     \
        if (!hs->buckets)                                            \
            return false;                                            \
        for (size_t i = 0; i < cap; i++)                             \
            hs->buckets[i].occupied = false;                         \
        hs->capacity = cap;                                          \
        return true;                                                 \
    }                                                                \
                                                                     \
    void name##_init(name *hs)                                       \
    {                                                                \
        hs->buckets = NULL;                                          \
        hs->size = 0;                                                \
        hs->capacity = 0;                                            \
    }                                                                \
                                                                     \
    void name##_free(name *hs)                                       \
    {                                                                \
        if (!hs)                                                     \
            return;                                                  \
        AB_HS_FREE(hs->buckets);                                     \
        hs->buckets = NULL;                                          \
        hs->size = 0;                                                \
        hs->capacity = 0;                                            \
    }                                                                \
                                                                     \
    static size_t name##_next_pow2(size_t n)                         \
    {                                                                \
        if (n == 0)                                                  \
            return AB_HS_INITIAL_CAPACITY;                           \
        n--;                                                         \
        n |= n >> 1;                                                 \
        n |= n >> 2;                                                 \
        n |= n >> 4;                                                 \
        n |= n >> 8;                                                 \
        n |= n >> 16;                                                \
        if (sizeof(size_t) > 4)                                      \
            n |= n >> 32;                                            \
        return n + 1;                                                \
    }                                                                \
                                                                     \
    static bool name##_rehash(name *hs, size_t new_cap)              \
    {                                                                \
        name##_bucket *old = hs->buckets;                            \
        size_t old_cap = hs->capacity;                               \
                                                                     \
        hs->buckets = AB_HS_MALLOC(new_cap * sizeof(name##_bucket)); \
        if (!hs->buckets)                                            \
        {                                                            \
            hs->buckets = old;                                       \
            return false;                                            \
        }                                                            \
        for (size_t i = 0; i < new_cap; i++)                         \
            hs->buckets[i].occupied = false;                         \
        hs->capacity = new_cap;                                      \
        hs->size = 0;                                                \
                                                                     \
        for (size_t i = 0; i < old_cap; i++)                         \
            if (old[i].occupied)                                     \
                name##_add(hs, old[i].data);                         \
                                                                     \
        AB_HS_FREE(old);                                             \
        return true;                                                 \
    }                                                                \
                                                                     \
    static bool name##_needs_resize(const name *hs)                  \
    {                                                                \
        return hs->size * AB_HS_LOAD_FACTOR_DEN >                    \
               hs->capacity * AB_HS_LOAD_FACTOR_NUM;                 \
    }                                                                \
                                                                     \
    bool name##_add(name *hs, type element)                          \
    {                                                                \
        if (!name##_valid(hs))                                       \
            return false;                                            \
                                                                     \
        if (hs->capacity == 0)                                       \
            if (!name##_alloc_buckets(hs, AB_HS_INITIAL_CAPACITY))   \
                return false;                                        \
                                                                     \
        if (name##_needs_resize(hs))                                 \
            if (!name##_rehash(hs, hs->capacity * 2))                \
                return false;                                        \
                                                                     \
        size_t mask = hs->capacity - 1;                              \
        size_t i = hash_fn(element) & mask;                          \
                                                                     \
        while (hs->buckets[i].occupied)                              \
        {                                                            \
            if (equals_fn(hs->buckets[i].data, element))             \
                return false;                                        \
            i = (i + 1) & mask;                                      \
        }                                                            \
                                                                     \
        hs->buckets[i].data = element;                               \
        hs->buckets[i].occupied = true;                              \
        hs->size++;                                                  \
        return true;                                                 \
    }                                                                \
                                                                     \
    bool name##_contains(const name *hs, type element)               \
    {                                                                \
        if (!name##_valid(hs) || hs->capacity == 0)                  \
            return false;                                            \
        size_t mask = hs->capacity - 1;                              \
        size_t i = hash_fn(element) & mask;                          \
        while (hs->buckets[i].occupied)                              \
        {                                                            \
            if (equals_fn(hs->buckets[i].data, element))             \
                return true;                                         \
            i = (i + 1) & mask;                                      \
        }                                                            \
        return false;                                                \
    }                                                                \
                                                                     \
    bool name##_remove(name *hs, type element)                       \
    {                                                                \
        if (!name##_valid(hs) || hs->capacity == 0)                  \
            return false;                                            \
        size_t mask = hs->capacity - 1;                              \
        size_t i = hash_fn(element) & mask;                          \
                                                                     \
        while (hs->buckets[i].occupied)                              \
        {                                                            \
            if (equals_fn(hs->buckets[i].data, element))             \
                break;                                               \
            i = (i + 1) & mask;                                      \
        }                                                            \
        if (!hs->buckets[i].occupied)                                \
            return false;                                            \
                                                                     \
        hs->buckets[i].occupied = false;                             \
        hs->size--;                                                  \
                                                                     \
        size_t gap = i;                                              \
        size_t j = (gap + 1) & mask;                                 \
        while (hs->buckets[j].occupied)                              \
        {                                                            \
            size_t natural = hash_fn(hs->buckets[j].data) & mask;    \
            bool should_shift =                                      \
                (j > gap)                                            \
                    ? (natural <= gap || natural > j)                \
                    : (natural <= gap && natural > j);               \
                                                                     \
            if (should_shift)                                        \
            {                                                        \
                hs->buckets[gap] = hs->buckets[j];                   \
                hs->buckets[j].occupied = false;                     \
                gap = j;                                             \
            }                                                        \
            j = (j + 1) & mask;                                      \
        }                                                            \
        return true;                                                 \
    }                                                                \
                                                                     \
    void name##_clear(name *hs)                                      \
    {                                                                \
        if (!name##_valid(hs) || !hs->buckets)                       \
            return;                                                  \
        for (size_t i = 0; i < hs->capacity; i++)                    \
            hs->buckets[i].occupied = false;                         \
        hs->size = 0;                                                \
    }                                                                \
                                                                     \
    size_t name##_size(const name *hs)                               \
    {                                                                \
        if (!name##_valid(hs))                                       \
            return 0;                                                \
        return hs->size;                                             \
    }                                                                \
                                                                     \
    bool name##_is_empty(const name *hs)                             \
    {                                                                \
        return name##_size(hs) == 0;                                 \
    }

#else

#define AB_HS_DEFINE_FUNCTIONS(type, name, hash_fn, equals_fn) \
    void name##_init(name *hs);                                \
    void name##_free(name *hs);                                \
    bool name##_add(name *hs, type element);                   \
    bool name##_contains(const name *hs, type element);        \
    bool name##_remove(name *hs, type element);                \
    void name##_clear(name *hs);                               \
    size_t name##_size(const name *hs);                        \
    bool name##_is_empty(const name *hs);

#endif

#define AB_HS_DEFINE(type, name, hash_fn, equals_fn) \
                                                     \
    typedef struct                                   \
    {                                                \
        type data;                                   \
        bool occupied;                               \
    } name##_bucket;                                 \
                                                     \
    typedef struct                                   \
    {                                                \
        name##_bucket *buckets;                      \
        size_t size;                                 \
        size_t capacity;                             \
    } name;                                          \
                                                     \
    static inline bool name##_valid(const name *hs)  \
    {                                                \
        return hs != NULL;                           \
    }                                                \
                                                     \
    AB_HS_DEFINE_FUNCTIONS(type, name, hash_fn, equals_fn)

#endif