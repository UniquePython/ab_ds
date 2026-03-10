/*
 * al.h — dynamic array (ArrayList) for C
 * Part of ab_ds — https://github.com/UniquePython/ab_ds
 *
 * USAGE
 *   In ONE .c file:
 *     #define AB_AL_IMPLEMENTATION
 *     #include "al.h"
 *
 *   Define a typed list:
 *     #define int_eq(a, b) ((a) == (b))
 *     AB_AL_DEFINE(int, IntList, int_eq)
 *
 *     IntList list;
 *     IntList_init(&list);
 *     IntList_push_back(&list, 42);
 *     IntList_push_back(&list, 99);
 *     IntList_free(&list);
 *
 * API
 *   _init(al)                        initialise (must be called before use)
 *   _free(al)                        free heap memory, reset to empty
 *   _push_back(al, element)          append to back
 *   _insert(al, index, element)      insert at index [0, size]
 *   _remove(al, index, *out)  bool   remove at index [0, size-1], write to out if non-NULL
 *   _get(al, index)           type*  pointer to element, NULL if out of range
 *   _set(al, index, element, *out)   replace element, write old to out if non-NULL
 *   _clear(al)                       set size to 0, keep allocation
 *   _size(al)                size_t
 *   _is_empty(al)            bool
 *   _index_of(al, element)   ptrdiff_t   first index of element, -1 if absent
 *   _contains(al, element)   bool
 *
 * CONFIGURATION (define before including)
 *   AB_AL_INITIAL_CAPACITY   default: 10
 *   AB_AL_REALLOC            default: realloc
 *   AB_AL_FREE               default: free
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

#ifndef AB_AL_H
#define AB_AL_H

#include <stdlib.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>

#ifndef AB_AL_REALLOC
#define AB_AL_REALLOC realloc
#endif

#ifndef AB_AL_FREE
#define AB_AL_FREE free
#endif

#ifndef AB_AL_INITIAL_CAPACITY
#define AB_AL_INITIAL_CAPACITY 10
#endif

#ifdef AB_AL_IMPLEMENTATION

#define AB_AL_DEFINE_FUNCTIONS(type, name, equals_fn)                   \
    void name##_init(name *al)                                          \
    {                                                                   \
        al->data = NULL;                                                \
        al->size = 0;                                                   \
        al->capacity = 0;                                               \
    }                                                                   \
                                                                        \
    void name##_free(name *al)                                          \
    {                                                                   \
        if (!al)                                                        \
            return;                                                     \
        AB_AL_FREE(al->data);                                           \
        al->data = NULL;                                                \
        al->size = 0;                                                   \
        al->capacity = 0;                                               \
    }                                                                   \
                                                                        \
    void name##_push_back(name *al, type element)                       \
    {                                                                   \
        if (!name##_valid(al))                                          \
            return;                                                     \
                                                                        \
        if (name##_needs_resize(al))                                    \
            if (!name##_grow(al))                                       \
                return;                                                 \
                                                                        \
        al->data[al->size++] = element;                                 \
    }                                                                   \
                                                                        \
    void name##_insert(name *al, size_t index, type element)            \
    {                                                                   \
        if (!name##_valid(al) || !name##_insert_index_valid(al, index)) \
            return;                                                     \
                                                                        \
        if (name##_needs_resize(al))                                    \
            if (!name##_grow(al))                                       \
                return;                                                 \
                                                                        \
        memmove(&al->data[index + 1],                                   \
                &al->data[index],                                       \
                (al->size - index) * sizeof(type));                     \
                                                                        \
        al->data[index] = element;                                      \
        al->size++;                                                     \
    }                                                                   \
                                                                        \
    bool name##_remove(name *al, size_t index, type *out)               \
    {                                                                   \
        if (!name##_valid(al) || !name##_index_valid(al, index))        \
            return false;                                               \
                                                                        \
        if (out)                                                        \
            *out = al->data[index];                                     \
                                                                        \
        if (index < al->size - 1)                                       \
        {                                                               \
            memmove(&al->data[index],                                   \
                    &al->data[index + 1],                               \
                    (al->size - index - 1) * sizeof(type));             \
        }                                                               \
                                                                        \
        al->size--;                                                     \
        return true;                                                    \
    }                                                                   \
                                                                        \
    type *name##_get(name *al, size_t index)                            \
    {                                                                   \
        if (!name##_valid(al) || !name##_index_valid(al, index))        \
            return NULL;                                                \
                                                                        \
        return &al->data[index];                                        \
    }                                                                   \
                                                                        \
    bool name##_set(name *al, size_t index, type element, type *out)    \
    {                                                                   \
        if (!name##_valid(al) || !name##_index_valid(al, index))        \
            return false;                                               \
                                                                        \
        if (out)                                                        \
            *out = al->data[index];                                     \
                                                                        \
        al->data[index] = element;                                      \
        return true;                                                    \
    }                                                                   \
                                                                        \
    void name##_clear(name *al)                                         \
    {                                                                   \
        if (!name##_valid(al))                                          \
            return;                                                     \
                                                                        \
        al->size = 0;                                                   \
    }                                                                   \
                                                                        \
    size_t name##_size(const name *al)                                  \
    {                                                                   \
        if (!name##_valid(al))                                          \
            return 0;                                                   \
                                                                        \
        return al->size;                                                \
    }                                                                   \
                                                                        \
    bool name##_is_empty(const name *al)                                \
    {                                                                   \
        return name##_size(al) == 0;                                    \
    }                                                                   \
                                                                        \
    ptrdiff_t name##_index_of(const name *al, type element)             \
    {                                                                   \
        if (!name##_valid(al))                                          \
            return -1;                                                  \
                                                                        \
        for (size_t i = 0; i < al->size; i++)                           \
        {                                                               \
            if (equals_fn(al->data[i], element))                        \
                return (ptrdiff_t)i;                                    \
        }                                                               \
                                                                        \
        return -1;                                                      \
    }                                                                   \
                                                                        \
    bool name##_contains(const name *al, type element)                  \
    {                                                                   \
        return name##_index_of(al, element) != -1;                      \
    }

#else

#define AB_AL_DEFINE_FUNCTIONS(type, name, equals_fn)                 \
    void name##_init(name *al);                                       \
    void name##_free(name *al);                                       \
                                                                      \
    void name##_push_back(name *al, type element);                    \
    void name##_insert(name *al, size_t index, type element);         \
                                                                      \
    bool name##_remove(name *al, size_t index, type *out);            \
                                                                      \
    type *name##_get(name *al, size_t index);                         \
                                                                      \
    bool name##_set(name *al, size_t index, type element, type *out); \
                                                                      \
    void name##_clear(name *al);                                      \
                                                                      \
    size_t name##_size(const name *al);                               \
    bool name##_is_empty(const name *al);                             \
                                                                      \
    ptrdiff_t name##_index_of(const name *al, type element);          \
    bool name##_contains(const name *al, type element);

#endif

#define AB_AL_DEFINE(type, name, equals_fn)                                                              \
    typedef struct                                                                                       \
    {                                                                                                    \
        type *data;                                                                                      \
        size_t size;                                                                                     \
        size_t capacity;                                                                                 \
    } name;                                                                                              \
                                                                                                         \
    static inline bool name##_valid(const name *al)                                                      \
    {                                                                                                    \
        return al != NULL;                                                                               \
    }                                                                                                    \
                                                                                                         \
    static inline bool name##_index_valid(const name *al, size_t index)                                  \
    {                                                                                                    \
        return al && index < al->size;                                                                   \
    }                                                                                                    \
                                                                                                         \
    static inline bool name##_insert_index_valid(const name *al, size_t index)                           \
    {                                                                                                    \
        return al && index <= al->size;                                                                  \
    }                                                                                                    \
                                                                                                         \
    static inline bool name##_needs_resize(const name *al)                                               \
    {                                                                                                    \
        return al->size >= al->capacity;                                                                 \
    }                                                                                                    \
                                                                                                         \
    static inline bool name##_grow(name *al)                                                             \
    {                                                                                                    \
        size_t new_capacity = al->capacity ? al->capacity + (al->capacity / 2) : AB_AL_INITIAL_CAPACITY; \
                                                                                                         \
        type *new_data = AB_AL_REALLOC(al->data, new_capacity * sizeof(type));                           \
        if (!new_data)                                                                                   \
            return false;                                                                                \
                                                                                                         \
        al->data = new_data;                                                                             \
        al->capacity = new_capacity;                                                                     \
        return true;                                                                                     \
    }                                                                                                    \
                                                                                                         \
    AB_AL_DEFINE_FUNCTIONS(type, name, equals_fn)

#endif