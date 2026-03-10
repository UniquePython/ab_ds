/*
 * ts.h — tree set for C, backed by a red-black tree
 * Part of ab_ds — https://github.com/UniquePython/ab_ds
 *
 * USAGE
 *   In ONE .c file:
 *     #define AB_TS_IMPLEMENTATION
 *     #include "ts.h"
 *
 *   Define a typed tree set:
 *     #define int_cmp(a, b) ((a) > (b)) - ((a) < (b))
 *     AB_TS_DEFINE(int, IntTreeSet, int_cmp)
 *
 *     IntTreeSet ts;
 *     IntTreeSet_init(&ts);
 *     IntTreeSet_add(&ts, 5);
 *     IntTreeSet_add(&ts, 2);
 *
 *     IntTreeSet_node *it = NULL;
 *     int *val;
 *     while ((val = IntTreeSet_next(&ts, &it)) != NULL)
 *         printf("%d\n", *val);
 *
 *     IntTreeSet_free(&ts);
 *
 * API
 *   _init(ts)                          initialise (must be called before use)
 *   _free(ts)                          free all nodes, reset to empty
 *   _add(ts, element)          bool    insert; false if already present
 *   _remove(ts, element)       bool    remove; false if not found
 *   _contains(ts, element)     bool
 *   _first(ts)                 type*   smallest element
 *   _last(ts)                  type*   largest element
 *   _ceiling(ts, element)      type*   first element >= element
 *   _higher(ts, element)       type*   first element > element
 *   _floor(ts, element)        type*   last element <= element
 *   _lower(ts, element)        type*   last element < element
 *   _next(ts, **it)            type*   in-order iterator; NULL it to start,
 *                                      returns NULL when exhausted
 *   _clear(ts)                         free all nodes, reset to empty
 *   _size(ts)                  size_t
 *   _is_empty(ts)              bool
 *
 * cmp_fn(a, b) must return:
 *   < 0  if a < b,   0  if a == b,   > 0  if a > b
 *
 * CONFIGURATION (define before including)
 *   AB_TS_MALLOC   default: malloc
 *   AB_TS_FREE     default: free
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

#ifndef AB_TS_H
#define AB_TS_H

#include <stdlib.h>
#include <stddef.h>
#include <stdbool.h>

#ifndef AB_TS_MALLOC
#define AB_TS_MALLOC malloc
#endif

#ifndef AB_TS_FREE
#define AB_TS_FREE free
#endif

#ifdef AB_TS_IMPLEMENTATION

#define AB_TS_DEFINE_FUNCTIONS(type, name, cmp_fn)                          \
                                                                            \
    void name##_init(name *ts)                                              \
    {                                                                       \
        ts->nil.red = false;                                                \
        ts->nil.left = &ts->nil;                                            \
        ts->nil.right = &ts->nil;                                           \
        ts->nil.parent = &ts->nil;                                          \
        ts->root = &ts->nil;                                                \
        ts->size = 0;                                                       \
    }                                                                       \
                                                                            \
    static void name##_free_subtree(name *ts, name##_node *n)               \
    {                                                                       \
        if (n == &ts->nil)                                                  \
            return;                                                         \
        name##_free_subtree(ts, n->left);                                   \
        name##_free_subtree(ts, n->right);                                  \
        AB_TS_FREE(n);                                                      \
    }                                                                       \
                                                                            \
    void name##_free(name *ts)                                              \
    {                                                                       \
        if (!ts)                                                            \
            return;                                                         \
        name##_free_subtree(ts, ts->root);                                  \
        ts->root = &ts->nil;                                                \
        ts->size = 0;                                                       \
    }                                                                       \
                                                                            \
    static void name##_rotate_left(name *ts, name##_node *x)                \
    {                                                                       \
        name##_node *y = x->right;                                          \
        x->right = y->left;                                                 \
        if (y->left != &ts->nil)                                            \
            y->left->parent = x;                                            \
        y->parent = x->parent;                                              \
        if (x->parent == &ts->nil)                                          \
            ts->root = y;                                                   \
        else if (x == x->parent->left)                                      \
            x->parent->left = y;                                            \
        else                                                                \
            x->parent->right = y;                                           \
        y->left = x;                                                        \
        x->parent = y;                                                      \
    }                                                                       \
                                                                            \
    static void name##_rotate_right(name *ts, name##_node *x)               \
    {                                                                       \
        name##_node *y = x->left;                                           \
        x->left = y->right;                                                 \
        if (y->right != &ts->nil)                                           \
            y->right->parent = x;                                           \
        y->parent = x->parent;                                              \
        if (x->parent == &ts->nil)                                          \
            ts->root = y;                                                   \
        else if (x == x->parent->right)                                     \
            x->parent->right = y;                                           \
        else                                                                \
            x->parent->left = y;                                            \
        y->right = x;                                                       \
        x->parent = y;                                                      \
    }                                                                       \
                                                                            \
    static void name##_insert_fixup(name *ts, name##_node *z)               \
    {                                                                       \
        while (z->parent->red)                                              \
        {                                                                   \
            if (z->parent == z->parent->parent->left)                       \
            {                                                               \
                name##_node *y = z->parent->parent->right;                  \
                if (y->red)                                                 \
                {                                                           \
                    z->parent->red = false;                                 \
                    y->red = false;                                         \
                    z->parent->parent->red = true;                          \
                    z = z->parent->parent;                                  \
                }                                                           \
                else                                                        \
                {                                                           \
                    if (z == z->parent->right)                              \
                    {                                                       \
                        z = z->parent;                                      \
                        name##_rotate_left(ts, z);                          \
                    }                                                       \
                    z->parent->red = false;                                 \
                    z->parent->parent->red = true;                          \
                    name##_rotate_right(ts, z->parent->parent);             \
                }                                                           \
            }                                                               \
            else                                                            \
            {                                                               \
                name##_node *y = z->parent->parent->left;                   \
                if (y->red)                                                 \
                {                                                           \
                    z->parent->red = false;                                 \
                    y->red = false;                                         \
                    z->parent->parent->red = true;                          \
                    z = z->parent->parent;                                  \
                }                                                           \
                else                                                        \
                {                                                           \
                    if (z == z->parent->left)                               \
                    {                                                       \
                        z = z->parent;                                      \
                        name##_rotate_right(ts, z);                         \
                    }                                                       \
                    z->parent->red = false;                                 \
                    z->parent->parent->red = true;                          \
                    name##_rotate_left(ts, z->parent->parent);              \
                }                                                           \
            }                                                               \
        }                                                                   \
        ts->root->red = false;                                              \
    }                                                                       \
                                                                            \
    bool name##_add(name *ts, type element)                                 \
    {                                                                       \
        if (!name##_valid(ts))                                              \
            return false;                                                   \
        name##_node *z = AB_TS_MALLOC(sizeof(name##_node));                 \
        if (!z)                                                             \
            return false;                                                   \
        z->data = element;                                                  \
        z->red = true;                                                      \
        z->left = &ts->nil;                                                 \
        z->right = &ts->nil;                                                \
        z->parent = &ts->nil;                                               \
                                                                            \
        name##_node *y = &ts->nil;                                          \
        name##_node *x = ts->root;                                          \
        while (x != &ts->nil)                                               \
        {                                                                   \
            y = x;                                                          \
            int c = cmp_fn(element, x->data);                               \
            if (c < 0)                                                      \
                x = x->left;                                                \
            else if (c > 0)                                                 \
                x = x->right;                                               \
            else                                                            \
            {                                                               \
                AB_TS_FREE(z);                                              \
                return false;                                               \
            }                                                               \
        }                                                                   \
        z->parent = y;                                                      \
        if (y == &ts->nil)                                                  \
            ts->root = z;                                                   \
        else if (cmp_fn(element, y->data) < 0)                              \
            y->left = z;                                                    \
        else                                                                \
            y->right = z;                                                   \
                                                                            \
        name##_insert_fixup(ts, z);                                         \
        ts->size++;                                                         \
        return true;                                                        \
    }                                                                       \
                                                                            \
    static name##_node *name##_minimum(name *ts, name##_node *x)            \
    {                                                                       \
        while (x->left != &ts->nil)                                         \
            x = x->left;                                                    \
        return x;                                                           \
    }                                                                       \
                                                                            \
    static name##_node *name##_maximum(name *ts, name##_node *x)            \
    {                                                                       \
        while (x->right != &ts->nil)                                        \
            x = x->right;                                                   \
        return x;                                                           \
    }                                                                       \
                                                                            \
    static void name##_transplant(name *ts, name##_node *u, name##_node *v) \
    {                                                                       \
        if (u->parent == &ts->nil)                                          \
            ts->root = v;                                                   \
        else if (u == u->parent->left)                                      \
            u->parent->left = v;                                            \
        else                                                                \
            u->parent->right = v;                                           \
        v->parent = u->parent;                                              \
    }                                                                       \
                                                                            \
    static void name##_delete_fixup(name *ts, name##_node *x)               \
    {                                                                       \
        while (x != ts->root && !x->red)                                    \
        {                                                                   \
            if (x == x->parent->left)                                       \
            {                                                               \
                name##_node *w = x->parent->right;                          \
                if (w->red)                                                 \
                {                                                           \
                    w->red = false;                                         \
                    x->parent->red = true;                                  \
                    name##_rotate_left(ts, x->parent);                      \
                    w = x->parent->right;                                   \
                }                                                           \
                if (!w->left->red && !w->right->red)                        \
                {                                                           \
                    w->red = true;                                          \
                    x = x->parent;                                          \
                }                                                           \
                else                                                        \
                {                                                           \
                    if (!w->right->red)                                     \
                    {                                                       \
                        w->left->red = false;                               \
                        w->red = true;                                      \
                        name##_rotate_right(ts, w);                         \
                        w = x->parent->right;                               \
                    }                                                       \
                    w->red = x->parent->red;                                \
                    x->parent->red = false;                                 \
                    w->right->red = false;                                  \
                    name##_rotate_left(ts, x->parent);                      \
                    x = ts->root;                                           \
                }                                                           \
            }                                                               \
            else                                                            \
            {                                                               \
                name##_node *w = x->parent->left;                           \
                if (w->red)                                                 \
                {                                                           \
                    w->red = false;                                         \
                    x->parent->red = true;                                  \
                    name##_rotate_right(ts, x->parent);                     \
                    w = x->parent->left;                                    \
                }                                                           \
                if (!w->right->red && !w->left->red)                        \
                {                                                           \
                    w->red = true;                                          \
                    x = x->parent;                                          \
                }                                                           \
                else                                                        \
                {                                                           \
                    if (!w->left->red)                                      \
                    {                                                       \
                        w->right->red = false;                              \
                        w->red = true;                                      \
                        name##_rotate_left(ts, w);                          \
                        w = x->parent->left;                                \
                    }                                                       \
                    w->red = x->parent->red;                                \
                    x->parent->red = false;                                 \
                    w->left->red = false;                                   \
                    name##_rotate_right(ts, x->parent);                     \
                    x = ts->root;                                           \
                }                                                           \
            }                                                               \
        }                                                                   \
        x->red = false;                                                     \
    }                                                                       \
                                                                            \
    bool name##_remove(name *ts, type element)                              \
    {                                                                       \
        if (!name##_valid(ts))                                              \
            return false;                                                   \
        name##_node *z = ts->root;                                          \
        while (z != &ts->nil)                                               \
        {                                                                   \
            int c = cmp_fn(element, z->data);                               \
            if (c < 0)                                                      \
                z = z->left;                                                \
            else if (c > 0)                                                 \
                z = z->right;                                               \
            else                                                            \
                break;                                                      \
        }                                                                   \
        if (z == &ts->nil)                                                  \
            return false;                                                   \
                                                                            \
        name##_node *y = z;                                                 \
        bool y_orig_red = y->red;                                           \
        name##_node *x;                                                     \
                                                                            \
        if (z->left == &ts->nil)                                            \
        {                                                                   \
            x = z->right;                                                   \
            name##_transplant(ts, z, z->right);                             \
        }                                                                   \
        else if (z->right == &ts->nil)                                      \
        {                                                                   \
            x = z->left;                                                    \
            name##_transplant(ts, z, z->left);                              \
        }                                                                   \
        else                                                                \
        {                                                                   \
            y = name##_minimum(ts, z->right);                               \
            y_orig_red = y->red;                                            \
            x = y->right;                                                   \
            if (y->parent == z)                                             \
            {                                                               \
                x->parent = y;                                              \
            }                                                               \
            else                                                            \
            {                                                               \
                name##_transplant(ts, y, y->right);                         \
                y->right = z->right;                                        \
                y->right->parent = y;                                       \
            }                                                               \
            name##_transplant(ts, z, y);                                    \
            y->left = z->left;                                              \
            y->left->parent = y;                                            \
            y->red = z->red;                                                \
        }                                                                   \
        AB_TS_FREE(z);                                                      \
        if (!y_orig_red)                                                    \
            name##_delete_fixup(ts, x);                                     \
        ts->size--;                                                         \
        return true;                                                        \
    }                                                                       \
                                                                            \
    bool name##_contains(name *ts, type element)                            \
    {                                                                       \
        if (!name##_valid(ts))                                              \
            return false;                                                   \
        name##_node *x = ts->root;                                          \
        while (x != &ts->nil)                                               \
        {                                                                   \
            int c = cmp_fn(element, x->data);                               \
            if (c < 0)                                                      \
                x = x->left;                                                \
            else if (c > 0)                                                 \
                x = x->right;                                               \
            else                                                            \
                return true;                                                \
        }                                                                   \
        return false;                                                       \
    }                                                                       \
                                                                            \
    type *name##_first(name *ts)                                            \
    {                                                                       \
        if (!name##_valid(ts) || ts->root == &ts->nil)                      \
            return NULL;                                                    \
        return &name##_minimum(ts, ts->root)->data;                         \
    }                                                                       \
                                                                            \
    type *name##_last(name *ts)                                             \
    {                                                                       \
        if (!name##_valid(ts) || ts->root == &ts->nil)                      \
            return NULL;                                                    \
        return &name##_maximum(ts, ts->root)->data;                         \
    }                                                                       \
                                                                            \
    type *name##_ceiling(name *ts, type element)                            \
    {                                                                       \
        if (!name##_valid(ts))                                              \
            return NULL;                                                    \
        name##_node *result = &ts->nil, *x = ts->root;                      \
        while (x != &ts->nil)                                               \
        {                                                                   \
            int c = cmp_fn(element, x->data);                               \
            if (c <= 0)                                                     \
            {                                                               \
                result = x;                                                 \
                x = x->left;                                                \
            }                                                               \
            else                                                            \
                x = x->right;                                               \
        }                                                                   \
        return result == &ts->nil ? NULL : &result->data;                   \
    }                                                                       \
                                                                            \
    type *name##_higher(name *ts, type element)                             \
    {                                                                       \
        if (!name##_valid(ts))                                              \
            return NULL;                                                    \
        name##_node *result = &ts->nil, *x = ts->root;                      \
        while (x != &ts->nil)                                               \
        {                                                                   \
            if (cmp_fn(element, x->data) < 0)                               \
            {                                                               \
                result = x;                                                 \
                x = x->left;                                                \
            }                                                               \
            else                                                            \
                x = x->right;                                               \
        }                                                                   \
        return result == &ts->nil ? NULL : &result->data;                   \
    }                                                                       \
                                                                            \
    type *name##_floor(name *ts, type element)                              \
    {                                                                       \
        if (!name##_valid(ts))                                              \
            return NULL;                                                    \
        name##_node *result = &ts->nil, *x = ts->root;                      \
        while (x != &ts->nil)                                               \
        {                                                                   \
            int c = cmp_fn(element, x->data);                               \
            if (c >= 0)                                                     \
            {                                                               \
                result = x;                                                 \
                x = x->right;                                               \
            }                                                               \
            else                                                            \
                x = x->left;                                                \
        }                                                                   \
        return result == &ts->nil ? NULL : &result->data;                   \
    }                                                                       \
                                                                            \
    type *name##_lower(name *ts, type element)                              \
    {                                                                       \
        if (!name##_valid(ts))                                              \
            return NULL;                                                    \
        name##_node *result = &ts->nil, *x = ts->root;                      \
        while (x != &ts->nil)                                               \
        {                                                                   \
            if (cmp_fn(element, x->data) > 0)                               \
            {                                                               \
                result = x;                                                 \
                x = x->right;                                               \
            }                                                               \
            else                                                            \
                x = x->left;                                                \
        }                                                                   \
        return result == &ts->nil ? NULL : &result->data;                   \
    }                                                                       \
                                                                            \
    static name##_node *name##_inorder_next(name *ts, name##_node *n)       \
    {                                                                       \
        if (n->right != &ts->nil)                                           \
            return name##_minimum(ts, n->right);                            \
        name##_node *p = n->parent;                                         \
        while (p != &ts->nil && n == p->right)                              \
        {                                                                   \
            n = p;                                                          \
            p = p->parent;                                                  \
        }                                                                   \
        return p;                                                           \
    }                                                                       \
                                                                            \
    type *name##_next(name *ts, name##_node **it)                           \
    {                                                                       \
        if (!name##_valid(ts) || !it)                                       \
            return NULL;                                                    \
        if (*it == NULL)                                                    \
        {                                                                   \
            if (ts->root == &ts->nil)                                       \
                return NULL;                                                \
            *it = name##_minimum(ts, ts->root);                             \
        }                                                                   \
        else                                                                \
        {                                                                   \
            *it = name##_inorder_next(ts, *it);                             \
            if (*it == &ts->nil)                                            \
            {                                                               \
                *it = NULL;                                                 \
                return NULL;                                                \
            }                                                               \
        }                                                                   \
        return &(*it)->data;                                                \
    }                                                                       \
                                                                            \
    void name##_clear(name *ts)                                             \
    {                                                                       \
        if (!name##_valid(ts))                                              \
            return;                                                         \
        name##_free_subtree(ts, ts->root);                                  \
        ts->root = &ts->nil;                                                \
        ts->size = 0;                                                       \
    }                                                                       \
                                                                            \
    size_t name##_size(const name *ts)                                      \
    {                                                                       \
        if (!name##_valid(ts))                                              \
            return 0;                                                       \
        return ts->size;                                                    \
    }                                                                       \
                                                                            \
    bool name##_is_empty(const name *ts)                                    \
    {                                                                       \
        return name##_size(ts) == 0;                                        \
    }

#else

#define AB_TS_DEFINE_FUNCTIONS(type, name, cmp_fn) \
    void name##_init(name *ts);                    \
    void name##_free(name *ts);                    \
    bool name##_add(name *ts, type element);       \
    bool name##_remove(name *ts, type element);    \
    bool name##_contains(name *ts, type element);  \
    type *name##_first(name *ts);                  \
    type *name##_last(name *ts);                   \
    type *name##_ceiling(name *ts, type element);  \
    type *name##_higher(name *ts, type element);   \
    type *name##_floor(name *ts, type element);    \
    type *name##_lower(name *ts, type element);    \
    type *name##_next(name *ts, name##_node **it); \
    void name##_clear(name *ts);                   \
    size_t name##_size(const name *ts);            \
    bool name##_is_empty(const name *ts);

#endif

#define AB_TS_DEFINE(type, name, cmp_fn)            \
                                                    \
    typedef struct name##_node                      \
    {                                               \
        type data;                                  \
        bool red;                                   \
        struct name##_node *left, *right, *parent;  \
    } name##_node;                                  \
                                                    \
    typedef struct                                  \
    {                                               \
        name##_node *root;                          \
        name##_node nil;                            \
        size_t size;                                \
    } name;                                         \
                                                    \
    static inline bool name##_valid(const name *ts) \
    {                                               \
        return ts != NULL;                          \
    }                                               \
                                                    \
    AB_TS_DEFINE_FUNCTIONS(type, name, cmp_fn)

#endif