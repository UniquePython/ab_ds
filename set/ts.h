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

/*
 * cmp_fn(a, b) -> int
 *   < 0  if a < b
 *     0  if a == b
 *   > 0  if a > b
 *
 * Example for int:
 *   #define int_cmp(a, b) ((a) > (b)) - ((a) < (b))
 *
 * Example for strings:
 *   #define str_cmp(a, b) strcmp((a), (b))
 *
 * Iterator usage (in-order traversal):
 *   name##_node *it = NULL;
 *   type *val;
 *   while ((val = MySet_next(&ts, &it)) != NULL)
 *       do_something(*val);
 */

#ifdef AB_TS_IMPLEMENTATION

#define AB_TS_DEFINE_FUNCTIONS(type, name, cmp_fn)                                \
                                                                                  \
    void name##_init(name *ts)                                                    \
    {                                                                             \
        ts->nil.red = false;                                                      \
        ts->nil.left = &ts->nil;                                                  \
        ts->nil.right = &ts->nil;                                                 \
        ts->nil.parent = &ts->nil;                                                \
        ts->root = &ts->nil;                                                      \
        ts->size = 0;                                                             \
    }                                                                             \
                                                                                  \
    static void name##_free_subtree(name *ts, name##_node *n)                     \
    {                                                                             \
        if (n == &ts->nil)                                                        \
            return;                                                               \
        name##_free_subtree(ts, n->left);                                         \
        name##_free_subtree(ts, n->right);                                        \
        AB_TS_FREE(n);                                                            \
    }                                                                             \
                                                                                  \
    void name##_free(name *ts)                                                    \
    {                                                                             \
        if (!ts)                                                                  \
            return;                                                               \
        name##_free_subtree(ts, ts->root);                                        \
        ts->root = &ts->nil;                                                      \
        ts->size = 0;                                                             \
    }                                                                             \
                                                                                  \
    static void name##_rotate_left(name *ts, name##_node *x)                      \
    {                                                                             \
        name##_node *y = x->right;                                                \
        x->right = y->left;                                                       \
        if (y->left != &ts->nil)                                                  \
            y->left->parent = x;                                                  \
        y->parent = x->parent;                                                    \
        if (x->parent == &ts->nil)                                                \
            ts->root = y;                                                         \
        else if (x == x->parent->left)                                            \
            x->parent->left = y;                                                  \
        else                                                                      \
            x->parent->right = y;                                                 \
        y->left = x;                                                              \
        x->parent = y;                                                            \
    }                                                                             \
                                                                                  \
    static void name##_rotate_right(name *ts, name##_node *x)                     \
    {                                                                             \
        name##_node *y = x->left;                                                 \
        x->left = y->right;                                                       \
        if (y->right != &ts->nil)                                                 \
            y->right->parent = x;                                                 \
        y->parent = x->parent;                                                    \
        if (x->parent == &ts->nil)                                                \
            ts->root = y;                                                         \
        else if (x == x->parent->right)                                           \
            x->parent->right = y;                                                 \
        else                                                                      \
            x->parent->left = y;                                                  \
        y->right = x;                                                             \
        x->parent = y;                                                            \
    }                                                                             \
                                                                                  \
    static void name##_insert_fixup(name *ts, name##_node *z)                     \
    {                                                                             \
        while (z->parent->red)                                                    \
        {                                                                         \
            if (z->parent == z->parent->parent->left)                             \
            {                                                                     \
                name##_node *y = z->parent->parent->right; /* uncle */            \
                if (y->red)                                                       \
                {                                                                 \
                    /* Case 1: uncle red : recolor, move up */                    \
                    z->parent->red = false;                                       \
                    y->red = false;                                               \
                    z->parent->parent->red = true;                                \
                    z = z->parent->parent;                                        \
                }                                                                 \
                else                                                              \
                {                                                                 \
                    if (z == z->parent->right)                                    \
                    {                                                             \
                        /* Case 2: uncle black, inner child : rotate to case 3 */ \
                        z = z->parent;                                            \
                        name##_rotate_left(ts, z);                                \
                    }                                                             \
                    /* Case 3: uncle black, outer child : rotate grandparent */   \
                    z->parent->red = false;                                       \
                    z->parent->parent->red = true;                                \
                    name##_rotate_right(ts, z->parent->parent);                   \
                }                                                                 \
            }                                                                     \
            else /* mirror image */                                               \
            {                                                                     \
                name##_node *y = z->parent->parent->left;                         \
                if (y->red)                                                       \
                {                                                                 \
                    z->parent->red = false;                                       \
                    y->red = false;                                               \
                    z->parent->parent->red = true;                                \
                    z = z->parent->parent;                                        \
                }                                                                 \
                else                                                              \
                {                                                                 \
                    if (z == z->parent->left)                                     \
                    {                                                             \
                        z = z->parent;                                            \
                        name##_rotate_right(ts, z);                               \
                    }                                                             \
                    z->parent->red = false;                                       \
                    z->parent->parent->red = true;                                \
                    name##_rotate_left(ts, z->parent->parent);                    \
                }                                                                 \
            }                                                                     \
        }                                                                         \
        ts->root->red = false;                                                    \
    }                                                                             \
                                                                                  \
    bool name##_add(name *ts, type element)                                       \
    {                                                                             \
        if (!name##_valid(ts))                                                    \
            return false;                                                         \
        name##_node *z = AB_TS_MALLOC(sizeof(name##_node));                       \
        if (!z)                                                                   \
            return false;                                                         \
        z->data = element;                                                        \
        z->red = true;                                                            \
        z->left = &ts->nil;                                                       \
        z->right = &ts->nil;                                                      \
        z->parent = &ts->nil;                                                     \
                                                                                  \
        name##_node *y = &ts->nil;                                                \
        name##_node *x = ts->root;                                                \
        while (x != &ts->nil)                                                     \
        {                                                                         \
            y = x;                                                                \
            int c = cmp_fn(element, x->data);                                     \
            if (c < 0)                                                            \
                x = x->left;                                                      \
            else if (c > 0)                                                       \
                x = x->right;                                                     \
            else                                                                  \
            {                                                                     \
                AB_TS_FREE(z);                                                    \
                return false;                                                     \
            }                                                                     \
        }                                                                         \
        z->parent = y;                                                            \
        if (y == &ts->nil)                                                        \
            ts->root = z;                                                         \
        else if (cmp_fn(element, y->data) < 0)                                    \
            y->left = z;                                                          \
        else                                                                      \
            y->right = z;                                                         \
                                                                                  \
        name##_insert_fixup(ts, z);                                               \
        ts->size++;                                                               \
        return true;                                                              \
    }                                                                             \
                                                                                  \
    static name##_node *name##_minimum(name *ts, name##_node *x)                  \
    {                                                                             \
        while (x->left != &ts->nil)                                               \
            x = x->left;                                                          \
        return x;                                                                 \
    }                                                                             \
                                                                                  \
    static name##_node *name##_maximum(name *ts, name##_node *x)                  \
    {                                                                             \
        while (x->right != &ts->nil)                                              \
            x = x->right;                                                         \
        return x;                                                                 \
    }                                                                             \
                                                                                  \
    static void name##_transplant(name *ts, name##_node *u, name##_node *v)       \
    {                                                                             \
        if (u->parent == &ts->nil)                                                \
            ts->root = v;                                                         \
        else if (u == u->parent->left)                                            \
            u->parent->left = v;                                                  \
        else                                                                      \
            u->parent->right = v;                                                 \
        v->parent = u->parent;                                                    \
    }                                                                             \
                                                                                  \
    static void name##_delete_fixup(name *ts, name##_node *x)                     \
    {                                                                             \
        while (x != ts->root && !x->red)                                          \
        {                                                                         \
            if (x == x->parent->left)                                             \
            {                                                                     \
                name##_node *w = x->parent->right; /* sibling */                  \
                if (w->red)                                                       \
                {                                                                 \
                    /* Case 1: sibling red : rotate to get black sibling */       \
                    w->red = false;                                               \
                    x->parent->red = true;                                        \
                    name##_rotate_left(ts, x->parent);                            \
                    w = x->parent->right;                                         \
                }                                                                 \
                if (!w->left->red && !w->right->red)                              \
                {                                                                 \
                    /* Case 2: sibling black, both nephews black : recolor */     \
                    w->red = true;                                                \
                    x = x->parent;                                                \
                }                                                                 \
                else                                                              \
                {                                                                 \
                    if (!w->right->red)                                           \
                    {                                                             \
                        /* Case 3: sibling black, far nephew black : rotate */    \
                        w->left->red = false;                                     \
                        w->red = true;                                            \
                        name##_rotate_right(ts, w);                               \
                        w = x->parent->right;                                     \
                    }                                                             \
                    /* Case 4: sibling black, far nephew red : rotate parent */   \
                    w->red = x->parent->red;                                      \
                    x->parent->red = false;                                       \
                    w->right->red = false;                                        \
                    name##_rotate_left(ts, x->parent);                            \
                    x = ts->root;                                                 \
                }                                                                 \
            }                                                                     \
            else /* mirror image */                                               \
            {                                                                     \
                name##_node *w = x->parent->left;                                 \
                if (w->red)                                                       \
                {                                                                 \
                    w->red = false;                                               \
                    x->parent->red = true;                                        \
                    name##_rotate_right(ts, x->parent);                           \
                    w = x->parent->left;                                          \
                }                                                                 \
                if (!w->right->red && !w->left->red)                              \
                {                                                                 \
                    w->red = true;                                                \
                    x = x->parent;                                                \
                }                                                                 \
                else                                                              \
                {                                                                 \
                    if (!w->left->red)                                            \
                    {                                                             \
                        w->right->red = false;                                    \
                        w->red = true;                                            \
                        name##_rotate_left(ts, w);                                \
                        w = x->parent->left;                                      \
                    }                                                             \
                    w->red = x->parent->red;                                      \
                    x->parent->red = false;                                       \
                    w->left->red = false;                                         \
                    name##_rotate_right(ts, x->parent);                           \
                    x = ts->root;                                                 \
                }                                                                 \
            }                                                                     \
        }                                                                         \
        x->red = false;                                                           \
    }                                                                             \
                                                                                  \
    bool name##_remove(name *ts, type element)                                    \
    {                                                                             \
        if (!name##_valid(ts))                                                    \
            return false;                                                         \
        name##_node *z = ts->root;                                                \
        while (z != &ts->nil)                                                     \
        {                                                                         \
            int c = cmp_fn(element, z->data);                                     \
            if (c < 0)                                                            \
                z = z->left;                                                      \
            else if (c > 0)                                                       \
                z = z->right;                                                     \
            else                                                                  \
                break;                                                            \
        }                                                                         \
        if (z == &ts->nil)                                                        \
            return false;                                                         \
                                                                                  \
        name##_node *y = z;                                                       \
        bool y_orig_red = y->red;                                                 \
        name##_node *x;                                                           \
                                                                                  \
        if (z->left == &ts->nil)                                                  \
        {                                                                         \
            x = z->right;                                                         \
            name##_transplant(ts, z, z->right);                                   \
        }                                                                         \
        else if (z->right == &ts->nil)                                            \
        {                                                                         \
            x = z->left;                                                          \
            name##_transplant(ts, z, z->left);                                    \
        }                                                                         \
        else                                                                      \
        {                                                                         \
            /* Two children: replace z with its in-order successor */             \
            y = name##_minimum(ts, z->right);                                     \
            y_orig_red = y->red;                                                  \
            x = y->right;                                                         \
            if (y->parent == z)                                                   \
            {                                                                     \
                x->parent = y; /* x may be nil; set parent for fixup */           \
            }                                                                     \
            else                                                                  \
            {                                                                     \
                name##_transplant(ts, y, y->right);                               \
                y->right = z->right;                                              \
                y->right->parent = y;                                             \
            }                                                                     \
            name##_transplant(ts, z, y);                                          \
            y->left = z->left;                                                    \
            y->left->parent = y;                                                  \
            y->red = z->red;                                                      \
        }                                                                         \
        AB_TS_FREE(z);                                                            \
        if (!y_orig_red)                                                          \
            name##_delete_fixup(ts, x);                                           \
        ts->size--;                                                               \
        return true;                                                              \
    }                                                                             \
                                                                                  \
    bool name##_contains(name *ts, type element)                                  \
    {                                                                             \
        if (!name##_valid(ts))                                                    \
            return false;                                                         \
        name##_node *x = ts->root;                                                \
        while (x != &ts->nil)                                                     \
        {                                                                         \
            int c = cmp_fn(element, x->data);                                     \
            if (c < 0)                                                            \
                x = x->left;                                                      \
            else if (c > 0)                                                       \
                x = x->right;                                                     \
            else                                                                  \
                return true;                                                      \
        }                                                                         \
        return false;                                                             \
    }                                                                             \
                                                                                  \
    type *name##_first(name *ts)                                                  \
    {                                                                             \
        if (!name##_valid(ts) || ts->root == &ts->nil)                            \
            return NULL;                                                          \
        return &name##_minimum(ts, ts->root)->data;                               \
    }                                                                             \
                                                                                  \
    type *name##_last(name *ts)                                                   \
    {                                                                             \
        if (!name##_valid(ts) || ts->root == &ts->nil)                            \
            return NULL;                                                          \
        return &name##_maximum(ts, ts->root)->data;                               \
    }                                                                             \
                                                                                  \
    type *name##_ceiling(name *ts, type element)                                  \
    {                                                                             \
        if (!name##_valid(ts))                                                    \
            return NULL;                                                          \
        name##_node *result = &ts->nil, *x = ts->root;                            \
        while (x != &ts->nil)                                                     \
        {                                                                         \
            int c = cmp_fn(element, x->data);                                     \
            if (c <= 0)                                                           \
            {                                                                     \
                result = x;                                                       \
                x = x->left;                                                      \
            }                                                                     \
            else                                                                  \
                x = x->right;                                                     \
        }                                                                         \
        return result == &ts->nil ? NULL : &result->data;                         \
    }                                                                             \
                                                                                  \
    type *name##_higher(name *ts, type element)                                   \
    {                                                                             \
        if (!name##_valid(ts))                                                    \
            return NULL;                                                          \
        name##_node *result = &ts->nil, *x = ts->root;                            \
        while (x != &ts->nil)                                                     \
        {                                                                         \
            if (cmp_fn(element, x->data) < 0)                                     \
            {                                                                     \
                result = x;                                                       \
                x = x->left;                                                      \
            }                                                                     \
            else                                                                  \
                x = x->right;                                                     \
        }                                                                         \
        return result == &ts->nil ? NULL : &result->data;                         \
    }                                                                             \
                                                                                  \
    type *name##_floor(name *ts, type element)                                    \
    {                                                                             \
        if (!name##_valid(ts))                                                    \
            return NULL;                                                          \
        name##_node *result = &ts->nil, *x = ts->root;                            \
        while (x != &ts->nil)                                                     \
        {                                                                         \
            int c = cmp_fn(element, x->data);                                     \
            if (c >= 0)                                                           \
            {                                                                     \
                result = x;                                                       \
                x = x->right;                                                     \
            }                                                                     \
            else                                                                  \
                x = x->left;                                                      \
        }                                                                         \
        return result == &ts->nil ? NULL : &result->data;                         \
    }                                                                             \
                                                                                  \
    type *name##_lower(name *ts, type element)                                    \
    {                                                                             \
        if (!name##_valid(ts))                                                    \
            return NULL;                                                          \
        name##_node *result = &ts->nil, *x = ts->root;                            \
        while (x != &ts->nil)                                                     \
        {                                                                         \
            if (cmp_fn(element, x->data) > 0)                                     \
            {                                                                     \
                result = x;                                                       \
                x = x->right;                                                     \
            }                                                                     \
            else                                                                  \
                x = x->left;                                                      \
        }                                                                         \
        return result == &ts->nil ? NULL : &result->data;                         \
    }                                                                             \
                                                                                  \
    static name##_node *name##_inorder_next(name *ts, name##_node *n)             \
    {                                                                             \
        if (n->right != &ts->nil)                                                 \
            return name##_minimum(ts, n->right);                                  \
        name##_node *p = n->parent;                                               \
        while (p != &ts->nil && n == p->right)                                    \
        {                                                                         \
            n = p;                                                                \
            p = p->parent;                                                        \
        }                                                                         \
        return p;                                                                 \
    }                                                                             \
                                                                                  \
    type *name##_next(name *ts, name##_node **it)                                 \
    {                                                                             \
        if (!name##_valid(ts) || !it)                                             \
            return NULL;                                                          \
        if (*it == NULL)                                                          \
        {                                                                         \
            if (ts->root == &ts->nil)                                             \
                return NULL;                                                      \
            *it = name##_minimum(ts, ts->root);                                   \
        }                                                                         \
        else                                                                      \
        {                                                                         \
            *it = name##_inorder_next(ts, *it);                                   \
            if (*it == &ts->nil)                                                  \
            {                                                                     \
                *it = NULL;                                                       \
                return NULL;                                                      \
            }                                                                     \
        }                                                                         \
        return &(*it)->data;                                                      \
    }                                                                             \
                                                                                  \
    void name##_clear(name *ts)                                                   \
    {                                                                             \
        if (!name##_valid(ts))                                                    \
            return;                                                               \
        name##_free_subtree(ts, ts->root);                                        \
        ts->root = &ts->nil;                                                      \
        ts->size = 0;                                                             \
    }                                                                             \
                                                                                  \
    size_t name##_size(const name *ts)                                            \
    {                                                                             \
        if (!name##_valid(ts))                                                    \
            return 0;                                                             \
        return ts->size;                                                          \
    }                                                                             \
                                                                                  \
    bool name##_is_empty(const name *ts)                                          \
    {                                                                             \
        return name##_size(ts) == 0;                                              \
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

#endif // AB_TS_H