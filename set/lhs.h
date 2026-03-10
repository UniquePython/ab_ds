/*
 * lhs.h — linked hash set for C; insertion-order iteration over a hash set
 * Part of ab_ds — https://github.com/UniquePython/ab_ds
 *
 * USAGE
 *   In ONE .c file:
 *     #define AB_LHS_IMPLEMENTATION
 *     #include "lhs.h"
 *
 *   Define a typed linked hash set:
 *     #define int_hash(x)  ((size_t)(x) * 2654435761UL)
 *     #define int_eq(a, b) ((a) == (b))
 *     AB_LHS_DEFINE(int, IntLinkedHashSet, int_hash, int_eq)
 *
 *     IntLinkedHashSet lhs;
 *     IntLinkedHashSet_init(&lhs);
 *     IntLinkedHashSet_add(&lhs, 1);
 *     IntLinkedHashSet_add(&lhs, 2);
 *
 *     size_t it = AB_LHS_NULL_IDX;
 *     int *val;
 *     while ((val = IntLinkedHashSet_next(&lhs, &it)) != NULL)
 *         printf("%d\n", *val);
 *
 *     IntLinkedHashSet_free(&lhs);
 *
 * API
 *   _init(lhs)                        initialise (must be called before use)
 *   _free(lhs)                        free heap memory, reset to empty
 *   _add(lhs, element)        bool    insert; false if already present
 *   _remove(lhs, element)     bool    remove; false if not found
 *   _contains(lhs, element)   bool
 *   _next(lhs, *it)           type*   iterate in insertion order; pass
 *                                     AB_LHS_NULL_IDX to start, returns NULL
 *                                     when done
 *   _clear(lhs)                       mark all buckets empty, keep allocation
 *   _size(lhs)                size_t
 *   _is_empty(lhs)            bool
 *
 * CONFIGURATION (define before including)
 *   AB_LHS_INITIAL_CAPACITY   default: 8 (must be power of 2)
 *   AB_LHS_MALLOC             default: malloc
 *   AB_LHS_FREE               default: free
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

#ifndef AB_LHS_H
#define AB_LHS_H

#include <stdlib.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include <limits.h>

#ifndef AB_LHS_MALLOC
#define AB_LHS_MALLOC malloc
#endif

#ifndef AB_LHS_FREE
#define AB_LHS_FREE free
#endif

#ifndef AB_LHS_INITIAL_CAPACITY
#define AB_LHS_INITIAL_CAPACITY 8
#endif

#define AB_LHS_LOAD_FACTOR_NUM 3
#define AB_LHS_LOAD_FACTOR_DEN 4

#define AB_LHS_NULL_IDX SIZE_MAX

#ifdef AB_LHS_IMPLEMENTATION

#define AB_LHS_DEFINE_FUNCTIONS(type, name, hash_fn, equals_fn)             \
                                                                            \
    void name##_init(name *lhs)                                             \
    {                                                                       \
        lhs->buckets = NULL;                                                \
        lhs->size = 0;                                                      \
        lhs->capacity = 0;                                                  \
        lhs->head_idx = AB_LHS_NULL_IDX;                                    \
        lhs->tail_idx = AB_LHS_NULL_IDX;                                    \
    }                                                                       \
                                                                            \
    void name##_free(name *lhs)                                             \
    {                                                                       \
        if (!lhs)                                                           \
            return;                                                         \
        AB_LHS_FREE(lhs->buckets);                                          \
        lhs->buckets = NULL;                                                \
        lhs->size = 0;                                                      \
        lhs->capacity = 0;                                                  \
        lhs->head_idx = AB_LHS_NULL_IDX;                                    \
        lhs->tail_idx = AB_LHS_NULL_IDX;                                    \
    }                                                                       \
                                                                            \
    static size_t name##_next_pow2(size_t n)                                \
    {                                                                       \
        if (n == 0)                                                         \
            return AB_LHS_INITIAL_CAPACITY;                                 \
        n--;                                                                \
        n |= n >> 1;                                                        \
        n |= n >> 2;                                                        \
        n |= n >> 4;                                                        \
        n |= n >> 8;                                                        \
        n |= n >> 16;                                                       \
        if (sizeof(size_t) > 4)                                             \
            n |= n >> 32;                                                   \
        return n + 1;                                                       \
    }                                                                       \
                                                                            \
    static bool name##_needs_resize(const name *lhs)                        \
    {                                                                       \
        return lhs->size * AB_LHS_LOAD_FACTOR_DEN >                         \
               lhs->capacity * AB_LHS_LOAD_FACTOR_NUM;                      \
    }                                                                       \
                                                                            \
    static size_t name##_find_idx(const name *lhs, type element)            \
    {                                                                       \
        if (!lhs->buckets || lhs->capacity == 0)                            \
            return AB_LHS_NULL_IDX;                                         \
        size_t mask = lhs->capacity - 1;                                    \
        size_t i = hash_fn(element) & mask;                                 \
        while (lhs->buckets[i].occupied)                                    \
        {                                                                   \
            if (equals_fn(lhs->buckets[i].data, element))                   \
                return i;                                                   \
            i = (i + 1) & mask;                                             \
        }                                                                   \
        return AB_LHS_NULL_IDX;                                             \
    }                                                                       \
                                                                            \
    static void name##_list_append(name *lhs, size_t i)                     \
    {                                                                       \
        lhs->buckets[i].prev_idx = lhs->tail_idx;                           \
        lhs->buckets[i].next_idx = AB_LHS_NULL_IDX;                         \
        if (lhs->tail_idx != AB_LHS_NULL_IDX)                               \
            lhs->buckets[lhs->tail_idx].next_idx = i;                       \
        else                                                                \
            lhs->head_idx = i;                                              \
        lhs->tail_idx = i;                                                  \
    }                                                                       \
                                                                            \
    static void name##_list_remove(name *lhs, size_t i)                     \
    {                                                                       \
        size_t p = lhs->buckets[i].prev_idx;                                \
        size_t n = lhs->buckets[i].next_idx;                                \
        if (p != AB_LHS_NULL_IDX)                                           \
            lhs->buckets[p].next_idx = n;                                   \
        else                                                                \
            lhs->head_idx = n;                                              \
        if (n != AB_LHS_NULL_IDX)                                           \
            lhs->buckets[n].prev_idx = p;                                   \
        else                                                                \
            lhs->tail_idx = p;                                              \
    }                                                                       \
                                                                            \
    static bool name##_rehash(name *lhs, size_t new_cap)                    \
    {                                                                       \
        name##_bucket *old_buckets = lhs->buckets;                          \
        size_t old_head = lhs->head_idx;                                    \
        size_t old_cap = lhs->capacity;                                     \
                                                                            \
        lhs->buckets = AB_LHS_MALLOC(new_cap * sizeof(name##_bucket));      \
        if (!lhs->buckets)                                                  \
        {                                                                   \
            lhs->buckets = old_buckets;                                     \
            return false;                                                   \
        }                                                                   \
        for (size_t i = 0; i < new_cap; i++)                                \
            lhs->buckets[i].occupied = false;                               \
        lhs->capacity = new_cap;                                            \
        lhs->size = 0;                                                      \
        lhs->head_idx = AB_LHS_NULL_IDX;                                    \
        lhs->tail_idx = AB_LHS_NULL_IDX;                                    \
                                                                            \
        size_t cur = old_head;                                              \
        while (cur != AB_LHS_NULL_IDX)                                      \
        {                                                                   \
            size_t next_cur = old_buckets[cur].next_idx;                    \
            name##_add(lhs, old_buckets[cur].data);                         \
            cur = next_cur;                                                 \
        }                                                                   \
                                                                            \
        AB_LHS_FREE(old_buckets);                                           \
        (void)old_cap;                                                      \
        return true;                                                        \
    }                                                                       \
                                                                            \
    bool name##_add(name *lhs, type element)                                \
    {                                                                       \
        if (!name##_valid(lhs))                                             \
            return false;                                                   \
                                                                            \
        if (lhs->capacity == 0)                                             \
        {                                                                   \
            lhs->buckets = AB_LHS_MALLOC(                                   \
                AB_LHS_INITIAL_CAPACITY * sizeof(name##_bucket));           \
            if (!lhs->buckets)                                              \
                return false;                                               \
            for (size_t i = 0; i < AB_LHS_INITIAL_CAPACITY; i++)            \
                lhs->buckets[i].occupied = false;                           \
            lhs->capacity = AB_LHS_INITIAL_CAPACITY;                        \
        }                                                                   \
                                                                            \
        if (name##_needs_resize(lhs))                                       \
            if (!name##_rehash(lhs, lhs->capacity * 2))                     \
                return false;                                               \
                                                                            \
        size_t mask = lhs->capacity - 1;                                    \
        size_t i = hash_fn(element) & mask;                                 \
        while (lhs->buckets[i].occupied)                                    \
        {                                                                   \
            if (equals_fn(lhs->buckets[i].data, element))                   \
                return false;                                               \
            i = (i + 1) & mask;                                             \
        }                                                                   \
                                                                            \
        lhs->buckets[i].data = element;                                     \
        lhs->buckets[i].occupied = true;                                    \
        name##_list_append(lhs, i);                                         \
        lhs->size++;                                                        \
        return true;                                                        \
    }                                                                       \
                                                                            \
    bool name##_contains(const name *lhs, type element)                     \
    {                                                                       \
        return name##_find_idx(lhs, element) != AB_LHS_NULL_IDX;            \
    }                                                                       \
                                                                            \
    bool name##_remove(name *lhs, type element)                             \
    {                                                                       \
        if (!name##_valid(lhs) || lhs->capacity == 0)                       \
            return false;                                                   \
        size_t mask = lhs->capacity - 1;                                    \
        size_t i = hash_fn(element) & mask;                                 \
        while (lhs->buckets[i].occupied)                                    \
        {                                                                   \
            if (equals_fn(lhs->buckets[i].data, element))                   \
                break;                                                      \
            i = (i + 1) & mask;                                             \
        }                                                                   \
        if (!lhs->buckets[i].occupied)                                      \
            return false;                                                   \
                                                                            \
        name##_list_remove(lhs, i);                                         \
        lhs->buckets[i].occupied = false;                                   \
        lhs->size--;                                                        \
                                                                            \
        size_t gap = i;                                                     \
        size_t j = (gap + 1) & mask;                                        \
        while (lhs->buckets[j].occupied)                                    \
        {                                                                   \
            size_t natural = hash_fn(lhs->buckets[j].data) & mask;          \
            bool should_shift =                                             \
                (j > gap)                                                   \
                    ? (natural <= gap || natural > j)                       \
                    : (natural <= gap && natural > j);                      \
                                                                            \
            if (should_shift)                                               \
            {                                                               \
                size_t p = lhs->buckets[j].prev_idx;                        \
                size_t n = lhs->buckets[j].next_idx;                        \
                if (p != AB_LHS_NULL_IDX)                                   \
                    lhs->buckets[p].next_idx = gap;                         \
                else                                                        \
                    lhs->head_idx = gap;                                    \
                if (n != AB_LHS_NULL_IDX)                                   \
                    lhs->buckets[n].prev_idx = gap;                         \
                else                                                        \
                    lhs->tail_idx = gap;                                    \
                                                                            \
                lhs->buckets[gap] = lhs->buckets[j];                        \
                lhs->buckets[j].occupied = false;                           \
                gap = j;                                                    \
            }                                                               \
            j = (j + 1) & mask;                                             \
        }                                                                   \
        return true;                                                        \
    }                                                                       \
                                                                            \
    type *name##_next(const name *lhs, size_t *it)                          \
    {                                                                       \
        if (!name##_valid(lhs) || !it)                                      \
            return NULL;                                                    \
        size_t cur = (*it == AB_LHS_NULL_IDX) ? lhs->head_idx               \
                                              : lhs->buckets[*it].next_idx; \
        if (cur == AB_LHS_NULL_IDX)                                         \
            return NULL;                                                    \
        *it = cur;                                                          \
        return &lhs->buckets[cur].data;                                     \
    }                                                                       \
                                                                            \
    void name##_clear(name *lhs)                                            \
    {                                                                       \
        if (!name##_valid(lhs) || !lhs->buckets)                            \
            return;                                                         \
        for (size_t i = 0; i < lhs->capacity; i++)                          \
            lhs->buckets[i].occupied = false;                               \
        lhs->size = 0;                                                      \
        lhs->head_idx = AB_LHS_NULL_IDX;                                    \
        lhs->tail_idx = AB_LHS_NULL_IDX;                                    \
    }                                                                       \
                                                                            \
    size_t name##_size(const name *lhs)                                     \
    {                                                                       \
        if (!name##_valid(lhs))                                             \
            return 0;                                                       \
        return lhs->size;                                                   \
    }                                                                       \
                                                                            \
    bool name##_is_empty(const name *lhs)                                   \
    {                                                                       \
        return name##_size(lhs) == 0;                                       \
    }

#else

#define AB_LHS_DEFINE_FUNCTIONS(type, name, hash_fn, equals_fn) \
    void name##_init(name *lhs);                                \
    void name##_free(name *lhs);                                \
    bool name##_add(name *lhs, type element);                   \
    bool name##_contains(const name *lhs, type element);        \
    bool name##_remove(name *lhs, type element);                \
    type *name##_next(const name *lhs, size_t *it);             \
    void name##_clear(name *lhs);                               \
    size_t name##_size(const name *lhs);                        \
    bool name##_is_empty(const name *lhs);

#endif

#define AB_LHS_DEFINE(type, name, hash_fn, equals_fn) \
                                                      \
    typedef struct                                    \
    {                                                 \
        type data;                                    \
        bool occupied;                                \
        size_t prev_idx;                              \
        size_t next_idx;                              \
    } name##_bucket;                                  \
                                                      \
    typedef struct                                    \
    {                                                 \
        name##_bucket *buckets;                       \
        size_t size;                                  \
        size_t capacity;                              \
        size_t head_idx;                              \
        size_t tail_idx;                              \
    } name;                                           \
                                                      \
    static inline bool name##_valid(const name *lhs)  \
    {                                                 \
        return lhs != NULL;                           \
    }                                                 \
                                                      \
    AB_LHS_DEFINE_FUNCTIONS(type, name, hash_fn, equals_fn)

#endif