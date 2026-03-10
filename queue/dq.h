/*
 * dq.h — double-ended queue (Deque) for C, backed by a ring buffer
 * Part of ab_ds — https://github.com/UniquePython/ab_ds
 *
 * USAGE
 *   In ONE .c file:
 *     #define AB_DQ_IMPLEMENTATION
 *     #include "dq.h"
 *
 *   Define a typed deque:
 *     #define int_eq(a, b) ((a) == (b))
 *     AB_DQ_DEFINE(int, IntDeque, int_eq)
 *
 *     IntDeque dq;
 *     IntDeque_init(&dq);
 *     IntDeque_push_back(&dq, 1);
 *     IntDeque_push_front(&dq, 0);
 *     IntDeque_free(&dq);
 *
 * API
 *   _init(dq)                          initialise (must be called before use)
 *   _free(dq)                          free heap memory, reset to empty
 *   _push_back(dq, element)            O(1) amortised append
 *   _push_front(dq, element)           O(1) amortised prepend
 *   _pop_back(dq, *out)        bool    remove from back, write to out if non-NULL
 *   _pop_front(dq, *out)       bool    remove from front, write to out if non-NULL
 *   _peek_front(dq)            type*   pointer to front element, NULL if empty
 *   _peek_back(dq)             type*   pointer to back element, NULL if empty
 *   _get(dq, index)            type*   pointer to element at logical index
 *   _set(dq, index, element, *out)     replace element, write old to out if non-NULL
 *   _clear(dq)                         reset size to 0, keep allocation
 *   _size(dq)                  size_t
 *   _is_empty(dq)              bool
 *   _index_of(dq, element)     ptrdiff_t   first logical index of element, -1 if absent
 *   _contains(dq, element)     bool
 *
 * CONFIGURATION (define before including)
 *   AB_DQ_INITIAL_CAPACITY   default: 8 (must be power of 2)
 *   AB_DQ_REALLOC            default: realloc
 *   AB_DQ_FREE               default: free
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

#ifndef AB_DQ_H
#define AB_DQ_H

#include <stdlib.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>

#ifndef AB_DQ_REALLOC
#define AB_DQ_REALLOC realloc
#endif

#ifndef AB_DQ_FREE
#define AB_DQ_FREE free
#endif

#ifndef AB_DQ_INITIAL_CAPACITY
#define AB_DQ_INITIAL_CAPACITY 8
#endif

#ifdef AB_DQ_IMPLEMENTATION

#define AB_DQ_DEFINE_FUNCTIONS(type, name, equals_fn)                              \
                                                                                   \
    void name##_init(name *dq)                                                     \
    {                                                                              \
        dq->data = NULL;                                                           \
        dq->head = 0;                                                              \
        dq->size = 0;                                                              \
        dq->capacity = 0;                                                          \
    }                                                                              \
                                                                                   \
    void name##_free(name *dq)                                                     \
    {                                                                              \
        if (!dq)                                                                   \
            return;                                                                \
        AB_DQ_FREE(dq->data);                                                      \
        dq->data = NULL;                                                           \
        dq->head = 0;                                                              \
        dq->size = 0;                                                              \
        dq->capacity = 0;                                                          \
    }                                                                              \
                                                                                   \
    static bool name##_grow(name *dq)                                              \
    {                                                                              \
        size_t new_cap = dq->capacity                                              \
                             ? dq->capacity + (dq->capacity / 2)                   \
                             : AB_DQ_INITIAL_CAPACITY;                             \
                                                                                   \
        type *new_data = AB_DQ_REALLOC(NULL, new_cap * sizeof(type));              \
        if (!new_data)                                                             \
            return false;                                                          \
                                                                                   \
        size_t right = dq->capacity - dq->head;                                    \
        if (right >= dq->size)                                                     \
        {                                                                          \
            memcpy(new_data, dq->data + dq->head, dq->size * sizeof(type));        \
        }                                                                          \
        else                                                                       \
        {                                                                          \
            memcpy(new_data, dq->data + dq->head, right * sizeof(type));           \
            memcpy(new_data + right, dq->data, (dq->size - right) * sizeof(type)); \
        }                                                                          \
                                                                                   \
        AB_DQ_FREE(dq->data);                                                      \
        dq->data = new_data;                                                       \
        dq->head = 0;                                                              \
        dq->capacity = new_cap;                                                    \
        return true;                                                               \
    }                                                                              \
                                                                                   \
    void name##_push_back(name *dq, type element)                                  \
    {                                                                              \
        if (!name##_valid(dq))                                                     \
            return;                                                                \
        if (dq->size == dq->capacity)                                              \
            if (!name##_grow(dq))                                                  \
                return;                                                            \
        size_t tail = (dq->head + dq->size) % dq->capacity;                        \
        dq->data[tail] = element;                                                  \
        dq->size++;                                                                \
    }                                                                              \
                                                                                   \
    void name##_push_front(name *dq, type element)                                 \
    {                                                                              \
        if (!name##_valid(dq))                                                     \
            return;                                                                \
        if (dq->size == dq->capacity)                                              \
            if (!name##_grow(dq))                                                  \
                return;                                                            \
        dq->head = (dq->head + dq->capacity - 1) % dq->capacity;                   \
        dq->data[dq->head] = element;                                              \
        dq->size++;                                                                \
    }                                                                              \
                                                                                   \
    bool name##_pop_back(name *dq, type *out)                                      \
    {                                                                              \
        if (!name##_valid(dq) || dq->size == 0)                                    \
            return false;                                                          \
        size_t tail = (dq->head + dq->size - 1) % dq->capacity;                    \
        if (out)                                                                   \
            *out = dq->data[tail];                                                 \
        dq->size--;                                                                \
        return true;                                                               \
    }                                                                              \
                                                                                   \
    bool name##_pop_front(name *dq, type *out)                                     \
    {                                                                              \
        if (!name##_valid(dq) || dq->size == 0)                                    \
            return false;                                                          \
        if (out)                                                                   \
            *out = dq->data[dq->head];                                             \
        dq->head = (dq->head + 1) % dq->capacity;                                  \
        dq->size--;                                                                \
        return true;                                                               \
    }                                                                              \
                                                                                   \
    type *name##_peek_front(name *dq)                                              \
    {                                                                              \
        if (!name##_valid(dq) || dq->size == 0)                                    \
            return NULL;                                                           \
        return &dq->data[dq->head];                                                \
    }                                                                              \
                                                                                   \
    type *name##_peek_back(name *dq)                                               \
    {                                                                              \
        if (!name##_valid(dq) || dq->size == 0)                                    \
            return NULL;                                                           \
        size_t tail = (dq->head + dq->size - 1) % dq->capacity;                    \
        return &dq->data[tail];                                                    \
    }                                                                              \
                                                                                   \
    type *name##_get(name *dq, size_t index)                                       \
    {                                                                              \
        if (!name##_valid(dq) || !name##_index_valid(dq, index))                   \
            return NULL;                                                           \
        return &dq->data[(dq->head + index) % dq->capacity];                       \
    }                                                                              \
                                                                                   \
    bool name##_set(name *dq, size_t index, type element, type *out)               \
    {                                                                              \
        if (!name##_valid(dq) || !name##_index_valid(dq, index))                   \
            return false;                                                          \
        size_t real = (dq->head + index) % dq->capacity;                           \
        if (out)                                                                   \
            *out = dq->data[real];                                                 \
        dq->data[real] = element;                                                  \
        return true;                                                               \
    }                                                                              \
                                                                                   \
    void name##_clear(name *dq)                                                    \
    {                                                                              \
        if (!name##_valid(dq))                                                     \
            return;                                                                \
        dq->head = 0;                                                              \
        dq->size = 0;                                                              \
    }                                                                              \
                                                                                   \
    size_t name##_size(const name *dq)                                             \
    {                                                                              \
        if (!name##_valid(dq))                                                     \
            return 0;                                                              \
        return dq->size;                                                           \
    }                                                                              \
                                                                                   \
    bool name##_is_empty(const name *dq)                                           \
    {                                                                              \
        return name##_size(dq) == 0;                                               \
    }                                                                              \
                                                                                   \
    ptrdiff_t name##_index_of(const name *dq, type element)                        \
    {                                                                              \
        if (!name##_valid(dq))                                                     \
            return -1;                                                             \
        for (size_t i = 0; i < dq->size; i++)                                      \
        {                                                                          \
            size_t real = (dq->head + i) % dq->capacity;                           \
            if (equals_fn(dq->data[real], element))                                \
                return (ptrdiff_t)i;                                               \
        }                                                                          \
        return -1;                                                                 \
    }                                                                              \
                                                                                   \
    bool name##_contains(const name *dq, type element)                             \
    {                                                                              \
        return name##_index_of(dq, element) != -1;                                 \
    }

#else

#define AB_DQ_DEFINE_FUNCTIONS(type, name, equals_fn)                 \
    void name##_init(name *dq);                                       \
    void name##_free(name *dq);                                       \
    void name##_push_back(name *dq, type element);                    \
    void name##_push_front(name *dq, type element);                   \
    bool name##_pop_back(name *dq, type *out);                        \
    bool name##_pop_front(name *dq, type *out);                       \
    type *name##_peek_front(name *dq);                                \
    type *name##_peek_back(name *dq);                                 \
    type *name##_get(name *dq, size_t index);                         \
    bool name##_set(name *dq, size_t index, type element, type *out); \
    void name##_clear(name *dq);                                      \
    size_t name##_size(const name *dq);                               \
    bool name##_is_empty(const name *dq);                             \
    ptrdiff_t name##_index_of(const name *dq, type element);          \
    bool name##_contains(const name *dq, type element);

#endif

#define AB_DQ_DEFINE(type, name, equals_fn)                             \
                                                                        \
    typedef struct                                                      \
    {                                                                   \
        type *data;                                                     \
        size_t head;                                                    \
        size_t size;                                                    \
        size_t capacity;                                                \
    } name;                                                             \
                                                                        \
    static inline bool name##_valid(const name *dq)                     \
    {                                                                   \
        return dq != NULL;                                              \
    }                                                                   \
                                                                        \
    static inline bool name##_index_valid(const name *dq, size_t index) \
    {                                                                   \
        return dq && index < dq->size;                                  \
    }                                                                   \
                                                                        \
    AB_DQ_DEFINE_FUNCTIONS(type, name, equals_fn)

#endif