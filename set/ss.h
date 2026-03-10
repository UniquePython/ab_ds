/*
 * ss.h — sorted set for C, backed by a sorted array with binary search
 * Part of ab_ds — https://github.com/UniquePython/ab_ds
 *
 * USAGE
 *   In ONE .c file:
 *     #define AB_SS_IMPLEMENTATION
 *     #include "ss.h"
 *
 *   Define a typed sorted set:
 *     #define int_cmp(a, b) ((a) > (b)) - ((a) < (b))
 *     AB_SS_DEFINE(int, IntSortedSet, int_cmp)
 *
 *     IntSortedSet ss;
 *     IntSortedSet_init(&ss);
 *     IntSortedSet_add(&ss, 5);
 *     IntSortedSet_add(&ss, 2);
 *     IntSortedSet_free(&ss);
 *
 * API
 *   _init(ss)                          initialise (must be called before use)
 *   _free(ss)                          free heap memory, reset to empty
 *   _add(ss, element)          bool    insert; false if already present
 *   _remove(ss, element)       bool    remove; false if not found
 *   _contains(ss, element)     bool
 *   _get(ss, index)            type*   element at sorted position index
 *   _first(ss)                 type*   smallest element
 *   _last(ss)                  type*   largest element
 *   _ceiling(ss, element)      type*   first element >= element
 *   _higher(ss, element)       type*   first element > element
 *   _floor(ss, element)        type*   last element <= element
 *   _lower(ss, element)        type*   last element < element
 *   _clear(ss)                         set size to 0, keep allocation
 *   _size(ss)                  size_t
 *   _is_empty(ss)              bool
 *
 * cmp_fn(a, b) must return:
 *   < 0  if a < b,   0  if a == b,   > 0  if a > b
 *
 * CONFIGURATION (define before including)
 *   AB_SS_INITIAL_CAPACITY   default: 8
 *   AB_SS_REALLOC            default: realloc
 *   AB_SS_FREE               default: free
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

#ifndef AB_SS_H
#define AB_SS_H

#include <stdlib.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>

#ifndef AB_SS_REALLOC
#define AB_SS_REALLOC realloc
#endif

#ifndef AB_SS_FREE
#define AB_SS_FREE free
#endif

#ifndef AB_SS_INITIAL_CAPACITY
#define AB_SS_INITIAL_CAPACITY 8
#endif

#ifdef AB_SS_IMPLEMENTATION

#define AB_SS_DEFINE_FUNCTIONS(type, name, cmp_fn)                        \
                                                                          \
    void name##_init(name *ss)                                            \
    {                                                                     \
        ss->data = NULL;                                                  \
        ss->size = 0;                                                     \
        ss->capacity = 0;                                                 \
    }                                                                     \
                                                                          \
    void name##_free(name *ss)                                            \
    {                                                                     \
        if (!ss)                                                          \
            return;                                                       \
        AB_SS_FREE(ss->data);                                             \
        ss->data = NULL;                                                  \
        ss->size = 0;                                                     \
        ss->capacity = 0;                                                 \
    }                                                                     \
                                                                          \
    static bool name##_grow(name *ss)                                     \
    {                                                                     \
        size_t new_cap = ss->capacity                                     \
                             ? ss->capacity + (ss->capacity / 2)          \
                             : AB_SS_INITIAL_CAPACITY;                    \
        type *new_data = AB_SS_REALLOC(ss->data, new_cap * sizeof(type)); \
        if (!new_data)                                                    \
            return false;                                                 \
        ss->data = new_data;                                              \
        ss->capacity = new_cap;                                           \
        return true;                                                      \
    }                                                                     \
                                                                          \
    static size_t name##_lower_bound(const name *ss, type element)        \
    {                                                                     \
        size_t lo = 0, hi = ss->size;                                     \
        while (lo < hi)                                                   \
        {                                                                 \
            size_t mid = lo + (hi - lo) / 2;                              \
            if (cmp_fn(ss->data[mid], element) < 0)                       \
                lo = mid + 1;                                             \
            else                                                          \
                hi = mid;                                                 \
        }                                                                 \
        return lo;                                                        \
    }                                                                     \
                                                                          \
    bool name##_add(name *ss, type element)                               \
    {                                                                     \
        if (!name##_valid(ss))                                            \
            return false;                                                 \
        size_t i = name##_lower_bound(ss, element);                       \
        if (i < ss->size && cmp_fn(ss->data[i], element) == 0)            \
            return false;                                                 \
        if (ss->size == ss->capacity)                                     \
            if (!name##_grow(ss))                                         \
                return false;                                             \
        memmove(&ss->data[i + 1], &ss->data[i],                           \
                (ss->size - i) * sizeof(type));                           \
        ss->data[i] = element;                                            \
        ss->size++;                                                       \
        return true;                                                      \
    }                                                                     \
                                                                          \
    bool name##_remove(name *ss, type element)                            \
    {                                                                     \
        if (!name##_valid(ss))                                            \
            return false;                                                 \
        size_t i = name##_lower_bound(ss, element);                       \
        if (i >= ss->size || cmp_fn(ss->data[i], element) != 0)           \
            return false;                                                 \
        memmove(&ss->data[i], &ss->data[i + 1],                           \
                (ss->size - i - 1) * sizeof(type));                       \
        ss->size--;                                                       \
        return true;                                                      \
    }                                                                     \
                                                                          \
    bool name##_contains(const name *ss, type element)                    \
    {                                                                     \
        if (!name##_valid(ss))                                            \
            return false;                                                 \
        size_t i = name##_lower_bound(ss, element);                       \
        return i < ss->size && cmp_fn(ss->data[i], element) == 0;         \
    }                                                                     \
                                                                          \
    type *name##_get(const name *ss, size_t index)                        \
    {                                                                     \
        if (!name##_valid(ss) || index >= ss->size)                       \
            return NULL;                                                  \
        return &ss->data[index];                                          \
    }                                                                     \
                                                                          \
    type *name##_first(const name *ss)                                    \
    {                                                                     \
        if (!name##_valid(ss) || ss->size == 0)                           \
            return NULL;                                                  \
        return &ss->data[0];                                              \
    }                                                                     \
                                                                          \
    type *name##_last(const name *ss)                                     \
    {                                                                     \
        if (!name##_valid(ss) || ss->size == 0)                           \
            return NULL;                                                  \
        return &ss->data[ss->size - 1];                                   \
    }                                                                     \
                                                                          \
    type *name##_ceiling(const name *ss, type element)                    \
    {                                                                     \
        if (!name##_valid(ss))                                            \
            return NULL;                                                  \
        size_t i = name##_lower_bound(ss, element);                       \
        return i < ss->size ? &ss->data[i] : NULL;                        \
    }                                                                     \
                                                                          \
    type *name##_higher(const name *ss, type element)                     \
    {                                                                     \
        if (!name##_valid(ss))                                            \
            return NULL;                                                  \
        size_t i = name##_lower_bound(ss, element);                       \
        if (i < ss->size && cmp_fn(ss->data[i], element) == 0)            \
            i++;                                                          \
        return i < ss->size ? &ss->data[i] : NULL;                        \
    }                                                                     \
                                                                          \
    type *name##_floor(const name *ss, type element)                      \
    {                                                                     \
        if (!name##_valid(ss))                                            \
            return NULL;                                                  \
        size_t i = name##_lower_bound(ss, element);                       \
        if (i < ss->size && cmp_fn(ss->data[i], element) == 0)            \
            return &ss->data[i];                                          \
        return i > 0 ? &ss->data[i - 1] : NULL;                           \
    }                                                                     \
                                                                          \
    type *name##_lower(const name *ss, type element)                      \
    {                                                                     \
        if (!name##_valid(ss))                                            \
            return NULL;                                                  \
        size_t i = name##_lower_bound(ss, element);                       \
        return i > 0 ? &ss->data[i - 1] : NULL;                           \
    }                                                                     \
                                                                          \
    void name##_clear(name *ss)                                           \
    {                                                                     \
        if (!name##_valid(ss))                                            \
            return;                                                       \
        ss->size = 0;                                                     \
    }                                                                     \
                                                                          \
    size_t name##_size(const name *ss)                                    \
    {                                                                     \
        if (!name##_valid(ss))                                            \
            return 0;                                                     \
        return ss->size;                                                  \
    }                                                                     \
                                                                          \
    bool name##_is_empty(const name *ss)                                  \
    {                                                                     \
        return name##_size(ss) == 0;                                      \
    }

#else

#define AB_SS_DEFINE_FUNCTIONS(type, name, cmp_fn)      \
    void name##_init(name *ss);                         \
    void name##_free(name *ss);                         \
    bool name##_add(name *ss, type element);            \
    bool name##_remove(name *ss, type element);         \
    bool name##_contains(const name *ss, type element); \
    type *name##_get(const name *ss, size_t index);     \
    type *name##_first(const name *ss);                 \
    type *name##_last(const name *ss);                  \
    type *name##_ceiling(const name *ss, type element); \
    type *name##_higher(const name *ss, type element);  \
    type *name##_floor(const name *ss, type element);   \
    type *name##_lower(const name *ss, type element);   \
    void name##_clear(name *ss);                        \
    size_t name##_size(const name *ss);                 \
    bool name##_is_empty(const name *ss);

#endif

#define AB_SS_DEFINE(type, name, cmp_fn)            \
                                                    \
    typedef struct                                  \
    {                                               \
        type *data;                                 \
        size_t size;                                \
        size_t capacity;                            \
    } name;                                         \
                                                    \
    static inline bool name##_valid(const name *ss) \
    {                                               \
        return ss != NULL;                          \
    }                                               \
                                                    \
    AB_SS_DEFINE_FUNCTIONS(type, name, cmp_fn)

#endif