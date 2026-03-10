/*
 * dl.h — doubly linked list for C
 * Part of ab_ds — https://github.com/UniquePython/ab_ds
 *
 * USAGE
 *   In ONE .c file:
 *     #define AB_DL_IMPLEMENTATION
 *     #include "dl.h"
 *
 *   Define a typed list:
 *     #define int_eq(a, b) ((a) == (b))
 *     AB_DL_DEFINE(int, IntDList, int_eq)
 *
 *     IntDList dl;
 *     IntDList_init(&dl);
 *     IntDList_push_back(&dl, 10);
 *     IntDList_push_front(&dl, 5);
 *     IntDList_free(&dl);
 *
 * API
 *   _init(dl)                          initialise (must be called before use)
 *   _free(dl)                          free all nodes and reset
 *   _push_front(dl, element)           O(1) prepend
 *   _push_back(dl, element)            O(1) append
 *   _pop_front(dl, *out)       bool    remove head, write to out if non-NULL
 *   _pop_back(dl, *out)        bool    O(1) — uses tail->prev directly
 *   _insert(dl, index, element)        insert at index [0, size]; walks from
 *                                      nearer end (O(n/2) worst case)
 *   _remove(dl, index, *out)   bool    remove at index [0, size-1]
 *   _get(dl, index)            type*   pointer to element, NULL if out of range
 *   _set(dl, index, element, *out)     replace element, write old to out if non-NULL
 *   _clear(dl)                         free all nodes, reset to empty
 *   _size(dl)                  size_t
 *   _is_empty(dl)              bool
 *   _index_of(dl, element)     ptrdiff_t   first index of element, -1 if absent
 *   _contains(dl, element)     bool
 *
 * CONFIGURATION (define before including)
 *   AB_DL_MALLOC   default: malloc
 *   AB_DL_FREE     default: free
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

#ifndef AB_DL_H
#define AB_DL_H

#include <stdlib.h>
#include <stddef.h>
#include <stdbool.h>

#ifndef AB_DL_MALLOC
#define AB_DL_MALLOC malloc
#endif

#ifndef AB_DL_FREE
#define AB_DL_FREE free
#endif

#ifdef AB_DL_IMPLEMENTATION

#define AB_DL_DEFINE_FUNCTIONS(type, name, equals_fn)                   \
                                                                        \
    name##_node *name##_alloc_node(type element)                        \
    {                                                                   \
        name##_node *node = AB_DL_MALLOC(sizeof(name##_node));          \
        if (!node)                                                      \
            return NULL;                                                \
        node->data = element;                                           \
        node->next = NULL;                                              \
        node->prev = NULL;                                              \
        return node;                                                    \
    }                                                                   \
                                                                        \
    void name##_init(name *dl)                                          \
    {                                                                   \
        dl->head = NULL;                                                \
        dl->tail = NULL;                                                \
        dl->size = 0;                                                   \
    }                                                                   \
                                                                        \
    void name##_free(name *dl)                                          \
    {                                                                   \
        if (!dl)                                                        \
            return;                                                     \
        name##_node *cur = dl->head;                                    \
        while (cur)                                                     \
        {                                                               \
            name##_node *next = cur->next;                              \
            AB_DL_FREE(cur);                                            \
            cur = next;                                                 \
        }                                                               \
        dl->head = NULL;                                                \
        dl->tail = NULL;                                                \
        dl->size = 0;                                                   \
    }                                                                   \
                                                                        \
    void name##_push_front(name *dl, type element)                      \
    {                                                                   \
        if (!name##_valid(dl))                                          \
            return;                                                     \
        name##_node *node = name##_alloc_node(element);                 \
        if (!node)                                                      \
            return;                                                     \
        node->next = dl->head;                                          \
        if (dl->head)                                                   \
            dl->head->prev = node;                                      \
        dl->head = node;                                                \
        if (!dl->tail)                                                  \
            dl->tail = node;                                            \
        dl->size++;                                                     \
    }                                                                   \
                                                                        \
    void name##_push_back(name *dl, type element)                       \
    {                                                                   \
        if (!name##_valid(dl))                                          \
            return;                                                     \
        name##_node *node = name##_alloc_node(element);                 \
        if (!node)                                                      \
            return;                                                     \
        node->prev = dl->tail;                                          \
        if (dl->tail)                                                   \
            dl->tail->next = node;                                      \
        dl->tail = node;                                                \
        if (!dl->head)                                                  \
            dl->head = node;                                            \
        dl->size++;                                                     \
    }                                                                   \
                                                                        \
    bool name##_pop_front(name *dl, type *out)                          \
    {                                                                   \
        if (!name##_valid(dl) || !dl->head)                             \
            return false;                                               \
        name##_node *old = dl->head;                                    \
        if (out)                                                        \
            *out = old->data;                                           \
        dl->head = old->next;                                           \
        if (dl->head)                                                   \
            dl->head->prev = NULL;                                      \
        else                                                            \
            dl->tail = NULL;                                            \
        AB_DL_FREE(old);                                                \
        dl->size--;                                                     \
        return true;                                                    \
    }                                                                   \
                                                                        \
    bool name##_pop_back(name *dl, type *out)                           \
    {                                                                   \
        if (!name##_valid(dl) || !dl->tail)                             \
            return false;                                               \
        name##_node *old = dl->tail;                                    \
        if (out)                                                        \
            *out = old->data;                                           \
        dl->tail = old->prev;                                           \
        if (dl->tail)                                                   \
            dl->tail->next = NULL;                                      \
        else                                                            \
            dl->head = NULL;                                            \
        AB_DL_FREE(old);                                                \
        dl->size--;                                                     \
        return true;                                                    \
    }                                                                   \
                                                                        \
    static name##_node *name##_node_at(name *dl, size_t index)          \
    {                                                                   \
        name##_node *cur;                                               \
        if (index < dl->size / 2)                                       \
        {                                                               \
            cur = dl->head;                                             \
            for (size_t i = 0; i < index; i++)                          \
                cur = cur->next;                                        \
        }                                                               \
        else                                                            \
        {                                                               \
            cur = dl->tail;                                             \
            for (size_t i = dl->size - 1; i > index; i--)               \
                cur = cur->prev;                                        \
        }                                                               \
        return cur;                                                     \
    }                                                                   \
                                                                        \
    void name##_insert(name *dl, size_t index, type element)            \
    {                                                                   \
        if (!name##_valid(dl) || !name##_insert_index_valid(dl, index)) \
            return;                                                     \
        if (index == 0)                                                 \
        {                                                               \
            name##_push_front(dl, element);                             \
            return;                                                     \
        }                                                               \
        if (index == dl->size)                                          \
        {                                                               \
            name##_push_back(dl, element);                              \
            return;                                                     \
        }                                                               \
        name##_node *node = name##_alloc_node(element);                 \
        if (!node)                                                      \
            return;                                                     \
        name##_node *next = name##_node_at(dl, index);                  \
        name##_node *prev = next->prev;                                 \
        node->next = next;                                              \
        node->prev = prev;                                              \
        prev->next = node;                                              \
        next->prev = node;                                              \
        dl->size++;                                                     \
    }                                                                   \
                                                                        \
    bool name##_remove(name *dl, size_t index, type *out)               \
    {                                                                   \
        if (!name##_valid(dl) || !name##_index_valid(dl, index))        \
            return false;                                               \
        if (index == 0)                                                 \
            return name##_pop_front(dl, out);                           \
        if (index == dl->size - 1)                                      \
            return name##_pop_back(dl, out);                            \
        name##_node *target = name##_node_at(dl, index);                \
        if (out)                                                        \
            *out = target->data;                                        \
        target->prev->next = target->next;                              \
        target->next->prev = target->prev;                              \
        AB_DL_FREE(target);                                             \
        dl->size--;                                                     \
        return true;                                                    \
    }                                                                   \
                                                                        \
    type *name##_get(name *dl, size_t index)                            \
    {                                                                   \
        if (!name##_valid(dl) || !name##_index_valid(dl, index))        \
            return NULL;                                                \
        return &name##_node_at(dl, index)->data;                        \
    }                                                                   \
                                                                        \
    bool name##_set(name *dl, size_t index, type element, type *out)    \
    {                                                                   \
        if (!name##_valid(dl) || !name##_index_valid(dl, index))        \
            return false;                                               \
        name##_node *node = name##_node_at(dl, index);                  \
        if (out)                                                        \
            *out = node->data;                                          \
        node->data = element;                                           \
        return true;                                                    \
    }                                                                   \
                                                                        \
    void name##_clear(name *dl)                                         \
    {                                                                   \
        if (!name##_valid(dl))                                          \
            return;                                                     \
        name##_node *cur = dl->head;                                    \
        while (cur)                                                     \
        {                                                               \
            name##_node *next = cur->next;                              \
            AB_DL_FREE(cur);                                            \
            cur = next;                                                 \
        }                                                               \
        dl->head = NULL;                                                \
        dl->tail = NULL;                                                \
        dl->size = 0;                                                   \
    }                                                                   \
                                                                        \
    size_t name##_size(const name *dl)                                  \
    {                                                                   \
        if (!name##_valid(dl))                                          \
            return 0;                                                   \
        return dl->size;                                                \
    }                                                                   \
                                                                        \
    bool name##_is_empty(const name *dl)                                \
    {                                                                   \
        return name##_size(dl) == 0;                                    \
    }                                                                   \
                                                                        \
    ptrdiff_t name##_index_of(const name *dl, type element)             \
    {                                                                   \
        if (!name##_valid(dl))                                          \
            return -1;                                                  \
        name##_node *cur = dl->head;                                    \
        for (size_t i = 0; i < dl->size; i++)                           \
        {                                                               \
            if (equals_fn(cur->data, element))                          \
                return (ptrdiff_t)i;                                    \
            cur = cur->next;                                            \
        }                                                               \
        return -1;                                                      \
    }                                                                   \
                                                                        \
    bool name##_contains(const name *dl, type element)                  \
    {                                                                   \
        return name##_index_of(dl, element) != -1;                      \
    }

#else

#define AB_DL_DEFINE_FUNCTIONS(type, name, equals_fn)                 \
    name##_node *name##_alloc_node(type element);                     \
    void name##_init(name *dl);                                       \
    void name##_free(name *dl);                                       \
    void name##_push_front(name *dl, type element);                   \
    void name##_push_back(name *dl, type element);                    \
    bool name##_pop_front(name *dl, type *out);                       \
    bool name##_pop_back(name *dl, type *out);                        \
    void name##_insert(name *dl, size_t index, type element);         \
    bool name##_remove(name *dl, size_t index, type *out);            \
    type *name##_get(name *dl, size_t index);                         \
    bool name##_set(name *dl, size_t index, type element, type *out); \
    void name##_clear(name *dl);                                      \
    size_t name##_size(const name *dl);                               \
    bool name##_is_empty(const name *dl);                             \
    ptrdiff_t name##_index_of(const name *dl, type element);          \
    bool name##_contains(const name *dl, type element);

#endif

#define AB_DL_DEFINE(type, name, equals_fn)                                    \
                                                                               \
    typedef struct name##_node                                                 \
    {                                                                          \
        type data;                                                             \
        struct name##_node *next;                                              \
        struct name##_node *prev;                                              \
    } name##_node;                                                             \
                                                                               \
    typedef struct                                                             \
    {                                                                          \
        name##_node *head;                                                     \
        name##_node *tail;                                                     \
        size_t size;                                                           \
    } name;                                                                    \
                                                                               \
    static inline bool name##_valid(const name *dl)                            \
    {                                                                          \
        return dl != NULL;                                                     \
    }                                                                          \
                                                                               \
    static inline bool name##_index_valid(const name *dl, size_t index)        \
    {                                                                          \
        return dl && index < dl->size;                                         \
    }                                                                          \
                                                                               \
    static inline bool name##_insert_index_valid(const name *dl, size_t index) \
    {                                                                          \
        return dl && index <= dl->size;                                        \
    }                                                                          \
                                                                               \
    AB_DL_DEFINE_FUNCTIONS(type, name, equals_fn)

#endif