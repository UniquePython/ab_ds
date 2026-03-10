/*
 * sl.h — singly linked list for C
 * Part of ab_ds — https://github.com/UniquePython/ab_ds
 *
 * USAGE
 *   In ONE .c file:
 *     #define AB_SL_IMPLEMENTATION
 *     #include "sl.h"
 *
 *   Define a typed list:
 *     #define int_eq(a, b) ((a) == (b))
 *     AB_SL_DEFINE(int, IntSList, int_eq)
 *
 *     IntSList sl;
 *     IntSList_init(&sl);
 *     IntSList_push_back(&sl, 10);
 *     IntSList_push_front(&sl, 5);
 *     IntSList_free(&sl);
 *
 * API
 *   _init(sl)                          initialise (must be called before use)
 *   _free(sl)                          free all nodes and reset
 *   _push_front(sl, element)           O(1) prepend
 *   _push_back(sl, element)            O(1) append
 *   _pop_front(sl, *out)       bool    remove head, write to out if non-NULL
 *   _pop_back(sl, *out)        bool    O(n) — must walk to second-to-last node
 *   _insert(sl, index, element)        insert at index [0, size]
 *   _remove(sl, index, *out)   bool    remove at index [0, size-1]
 *   _get(sl, index)            type*   pointer to element, NULL if out of range
 *   _set(sl, index, element, *out)     replace element, write old to out if non-NULL
 *   _clear(sl)                         free all nodes, reset to empty
 *   _size(sl)                  size_t
 *   _is_empty(sl)              bool
 *   _index_of(sl, element)     ptrdiff_t   first index of element, -1 if absent
 *   _contains(sl, element)     bool
 *
 * CONFIGURATION (define before including)
 *   AB_SL_MALLOC   default: malloc
 *   AB_SL_FREE     default: free
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

#ifndef AB_SL_H
#define AB_SL_H

#include <stdlib.h>
#include <stddef.h>
#include <stdbool.h>

#ifndef AB_SL_MALLOC
#define AB_SL_MALLOC malloc
#endif

#ifndef AB_SL_FREE
#define AB_SL_FREE free
#endif

#ifdef AB_SL_IMPLEMENTATION

#define AB_SL_DEFINE_FUNCTIONS(type, name, equals_fn)                   \
                                                                        \
    name##_node *name##_alloc_node(type element)                        \
    {                                                                   \
        name##_node *node = AB_SL_MALLOC(sizeof(name##_node));          \
        if (!node)                                                      \
            return NULL;                                                \
        node->data = element;                                           \
        node->next = NULL;                                              \
        return node;                                                    \
    }                                                                   \
                                                                        \
    void name##_init(name *sl)                                          \
    {                                                                   \
        sl->head = NULL;                                                \
        sl->tail = NULL;                                                \
        sl->size = 0;                                                   \
    }                                                                   \
                                                                        \
    void name##_free(name *sl)                                          \
    {                                                                   \
        if (!sl)                                                        \
            return;                                                     \
        name##_node *cur = sl->head;                                    \
        while (cur)                                                     \
        {                                                               \
            name##_node *next = cur->next;                              \
            AB_SL_FREE(cur);                                            \
            cur = next;                                                 \
        }                                                               \
        sl->head = NULL;                                                \
        sl->tail = NULL;                                                \
        sl->size = 0;                                                   \
    }                                                                   \
                                                                        \
    void name##_push_front(name *sl, type element)                      \
    {                                                                   \
        if (!name##_valid(sl))                                          \
            return;                                                     \
        name##_node *node = name##_alloc_node(element);                 \
        if (!node)                                                      \
            return;                                                     \
        node->next = sl->head;                                          \
        sl->head = node;                                                \
        if (!sl->tail)                                                  \
            sl->tail = node;                                            \
        sl->size++;                                                     \
    }                                                                   \
                                                                        \
    void name##_push_back(name *sl, type element)                       \
    {                                                                   \
        if (!name##_valid(sl))                                          \
            return;                                                     \
        name##_node *node = name##_alloc_node(element);                 \
        if (!node)                                                      \
            return;                                                     \
        if (!sl->tail)                                                  \
        {                                                               \
            sl->head = sl->tail = node;                                 \
        }                                                               \
        else                                                            \
        {                                                               \
            sl->tail->next = node;                                      \
            sl->tail = node;                                            \
        }                                                               \
        sl->size++;                                                     \
    }                                                                   \
                                                                        \
    bool name##_pop_front(name *sl, type *out)                          \
    {                                                                   \
        if (!name##_valid(sl) || !sl->head)                             \
            return false;                                               \
        name##_node *old = sl->head;                                    \
        if (out)                                                        \
            *out = old->data;                                           \
        sl->head = old->next;                                           \
        if (!sl->head)                                                  \
            sl->tail = NULL;                                            \
        AB_SL_FREE(old);                                                \
        sl->size--;                                                     \
        return true;                                                    \
    }                                                                   \
                                                                        \
    bool name##_pop_back(name *sl, type *out)                           \
    {                                                                   \
        if (!name##_valid(sl) || !sl->head)                             \
            return false;                                               \
        if (sl->head == sl->tail)                                       \
        {                                                               \
            if (out)                                                    \
                *out = sl->head->data;                                  \
            AB_SL_FREE(sl->head);                                       \
            sl->head = sl->tail = NULL;                                 \
            sl->size--;                                                 \
            return true;                                                \
        }                                                               \
        name##_node *cur = sl->head;                                    \
        while (cur->next != sl->tail)                                   \
            cur = cur->next;                                            \
        if (out)                                                        \
            *out = sl->tail->data;                                      \
        AB_SL_FREE(sl->tail);                                           \
        sl->tail = cur;                                                 \
        sl->tail->next = NULL;                                          \
        sl->size--;                                                     \
        return true;                                                    \
    }                                                                   \
                                                                        \
    void name##_insert(name *sl, size_t index, type element)            \
    {                                                                   \
        if (!name##_valid(sl) || !name##_insert_index_valid(sl, index)) \
            return;                                                     \
        if (index == 0)                                                 \
        {                                                               \
            name##_push_front(sl, element);                             \
            return;                                                     \
        }                                                               \
        if (index == sl->size)                                          \
        {                                                               \
            name##_push_back(sl, element);                              \
            return;                                                     \
        }                                                               \
        name##_node *node = name##_alloc_node(element);                 \
        if (!node)                                                      \
            return;                                                     \
        name##_node *cur = sl->head;                                    \
        for (size_t i = 0; i < index - 1; i++)                          \
            cur = cur->next;                                            \
        node->next = cur->next;                                         \
        cur->next = node;                                               \
        sl->size++;                                                     \
    }                                                                   \
                                                                        \
    bool name##_remove(name *sl, size_t index, type *out)               \
    {                                                                   \
        if (!name##_valid(sl) || !name##_index_valid(sl, index))        \
            return false;                                               \
        if (index == 0)                                                 \
            return name##_pop_front(sl, out);                           \
        if (index == sl->size - 1)                                      \
            return name##_pop_back(sl, out);                            \
        name##_node *prev = sl->head;                                   \
        for (size_t i = 0; i < index - 1; i++)                          \
            prev = prev->next;                                          \
        name##_node *target = prev->next;                               \
        if (out)                                                        \
            *out = target->data;                                        \
        prev->next = target->next;                                      \
        AB_SL_FREE(target);                                             \
        sl->size--;                                                     \
        return true;                                                    \
    }                                                                   \
                                                                        \
    type *name##_get(name *sl, size_t index)                            \
    {                                                                   \
        if (!name##_valid(sl) || !name##_index_valid(sl, index))        \
            return NULL;                                                \
        name##_node *cur = sl->head;                                    \
        for (size_t i = 0; i < index; i++)                              \
            cur = cur->next;                                            \
        return &cur->data;                                              \
    }                                                                   \
                                                                        \
    bool name##_set(name *sl, size_t index, type element, type *out)    \
    {                                                                   \
        if (!name##_valid(sl) || !name##_index_valid(sl, index))        \
            return false;                                               \
        name##_node *cur = sl->head;                                    \
        for (size_t i = 0; i < index; i++)                              \
            cur = cur->next;                                            \
        if (out)                                                        \
            *out = cur->data;                                           \
        cur->data = element;                                            \
        return true;                                                    \
    }                                                                   \
                                                                        \
    void name##_clear(name *sl)                                         \
    {                                                                   \
        if (!name##_valid(sl))                                          \
            return;                                                     \
        name##_node *cur = sl->head;                                    \
        while (cur)                                                     \
        {                                                               \
            name##_node *next = cur->next;                              \
            AB_SL_FREE(cur);                                            \
            cur = next;                                                 \
        }                                                               \
        sl->head = NULL;                                                \
        sl->tail = NULL;                                                \
        sl->size = 0;                                                   \
    }                                                                   \
                                                                        \
    size_t name##_size(const name *sl)                                  \
    {                                                                   \
        if (!name##_valid(sl))                                          \
            return 0;                                                   \
        return sl->size;                                                \
    }                                                                   \
                                                                        \
    bool name##_is_empty(const name *sl)                                \
    {                                                                   \
        return name##_size(sl) == 0;                                    \
    }                                                                   \
                                                                        \
    ptrdiff_t name##_index_of(const name *sl, type element)             \
    {                                                                   \
        if (!name##_valid(sl))                                          \
            return -1;                                                  \
        name##_node *cur = sl->head;                                    \
        for (size_t i = 0; i < sl->size; i++)                           \
        {                                                               \
            if (equals_fn(cur->data, element))                          \
                return (ptrdiff_t)i;                                    \
            cur = cur->next;                                            \
        }                                                               \
        return -1;                                                      \
    }                                                                   \
                                                                        \
    bool name##_contains(const name *sl, type element)                  \
    {                                                                   \
        return name##_index_of(sl, element) != -1;                      \
    }

#else

#define AB_SL_DEFINE_FUNCTIONS(type, name, equals_fn)                 \
    name##_node *name##_alloc_node(type element);                     \
    void name##_init(name *sl);                                       \
    void name##_free(name *sl);                                       \
    void name##_push_front(name *sl, type element);                   \
    void name##_push_back(name *sl, type element);                    \
    bool name##_pop_front(name *sl, type *out);                       \
    bool name##_pop_back(name *sl, type *out);                        \
    void name##_insert(name *sl, size_t index, type element);         \
    bool name##_remove(name *sl, size_t index, type *out);            \
    type *name##_get(name *sl, size_t index);                         \
    bool name##_set(name *sl, size_t index, type element, type *out); \
    void name##_clear(name *sl);                                      \
    size_t name##_size(const name *sl);                               \
    bool name##_is_empty(const name *sl);                             \
    ptrdiff_t name##_index_of(const name *sl, type element);          \
    bool name##_contains(const name *sl, type element);

#endif

#define AB_SL_DEFINE(type, name, equals_fn)                                    \
                                                                               \
    typedef struct name##_node                                                 \
    {                                                                          \
        type data;                                                             \
        struct name##_node *next;                                              \
    } name##_node;                                                             \
                                                                               \
    typedef struct                                                             \
    {                                                                          \
        name##_node *head;                                                     \
        name##_node *tail;                                                     \
        size_t size;                                                           \
    } name;                                                                    \
                                                                               \
    static inline bool name##_valid(const name *sl)                            \
    {                                                                          \
        return sl != NULL;                                                     \
    }                                                                          \
                                                                               \
    static inline bool name##_index_valid(const name *sl, size_t index)        \
    {                                                                          \
        return sl && index < sl->size;                                         \
    }                                                                          \
                                                                               \
    static inline bool name##_insert_index_valid(const name *sl, size_t index) \
    {                                                                          \
        return sl && index <= sl->size;                                        \
    }                                                                          \
                                                                               \
    AB_SL_DEFINE_FUNCTIONS(type, name, equals_fn)

#endif