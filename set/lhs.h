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
#define AB_LHS_INITIAL_CAPACITY 8 /* must be power of 2 */
#endif

#define AB_LHS_LOAD_FACTOR_NUM 3
#define AB_LHS_LOAD_FACTOR_DEN 4

#define AB_LHS_NULL_IDX SIZE_MAX /* sentinel for "no bucket" */

/*
 * hash_fn(element)  -> size_t
 * equals_fn(a, b)   -> bool
 *
 * Same conventions as hs.h.
 */

#ifdef AB_LHS_IMPLEMENTATION

#define AB_LHS_DEFINE_FUNCTIONS(type, name, hash_fn, equals_fn)               \
                                                                              \
    void name##_init(name *lhs)                                               \
    {                                                                         \
        lhs->buckets = NULL;                                                  \
        lhs->size = 0;                                                        \
        lhs->capacity = 0;                                                    \
        lhs->head_idx = AB_LHS_NULL_IDX;                                      \
        lhs->tail_idx = AB_LHS_NULL_IDX;                                      \
    }                                                                         \
                                                                              \
    void name##_free(name *lhs)                                               \
    {                                                                         \
        if (!lhs)                                                             \
            return;                                                           \
        AB_LHS_FREE(lhs->buckets);                                            \
        lhs->buckets = NULL;                                                  \
        lhs->size = 0;                                                        \
        lhs->capacity = 0;                                                    \
        lhs->head_idx = AB_LHS_NULL_IDX;                                      \
        lhs->tail_idx = AB_LHS_NULL_IDX;                                      \
    }                                                                         \
                                                                              \
    static size_t name##_next_pow2(size_t n)                                  \
    {                                                                         \
        if (n == 0)                                                           \
            return AB_LHS_INITIAL_CAPACITY;                                   \
        n--;                                                                  \
        n |= n >> 1;                                                          \
        n |= n >> 2;                                                          \
        n |= n >> 4;                                                          \
        n |= n >> 8;                                                          \
        n |= n >> 16;                                                         \
        if (sizeof(size_t) > 4)                                               \
            n |= n >> 32;                                                     \
        return n + 1;                                                         \
    }                                                                         \
                                                                              \
    static bool name##_needs_resize(const name *lhs)                          \
    {                                                                         \
        return lhs->size * AB_LHS_LOAD_FACTOR_DEN >                           \
               lhs->capacity * AB_LHS_LOAD_FACTOR_NUM;                        \
    }                                                                         \
                                                                              \
    /* Find the bucket index holding element, or AB_LHS_NULL_IDX if absent */ \
    static size_t name##_find_idx(const name *lhs, type element)              \
    {                                                                         \
        if (!lhs->buckets || lhs->capacity == 0)                              \
            return AB_LHS_NULL_IDX;                                           \
        size_t mask = lhs->capacity - 1;                                      \
        size_t i = hash_fn(element) & mask;                                   \
        while (lhs->buckets[i].occupied)                                      \
        {                                                                     \
            if (equals_fn(lhs->buckets[i].data, element))                     \
                return i;                                                     \
            i = (i + 1) & mask;                                               \
        }                                                                     \
        return AB_LHS_NULL_IDX;                                               \
    }                                                                         \
                                                                              \
    /* Thread a newly occupied bucket at index i onto the tail of the list */ \
    static void name##_list_append(name *lhs, size_t i)                       \
    {                                                                         \
        lhs->buckets[i].prev_idx = lhs->tail_idx;                             \
        lhs->buckets[i].next_idx = AB_LHS_NULL_IDX;                           \
        if (lhs->tail_idx != AB_LHS_NULL_IDX)                                 \
            lhs->buckets[lhs->tail_idx].next_idx = i;                         \
        else                                                                  \
            lhs->head_idx = i;                                                \
        lhs->tail_idx = i;                                                    \
    }                                                                         \
                                                                              \
    /* Unthread bucket at index i from the insertion-order list */            \
    static void name##_list_remove(name *lhs, size_t i)                       \
    {                                                                         \
        size_t p = lhs->buckets[i].prev_idx;                                  \
        size_t n = lhs->buckets[i].next_idx;                                  \
        if (p != AB_LHS_NULL_IDX)                                             \
            lhs->buckets[p].next_idx = n;                                     \
        else                                                                  \
            lhs->head_idx = n;                                                \
        if (n != AB_LHS_NULL_IDX)                                             \
            lhs->buckets[n].prev_idx = p;                                     \
        else                                                                  \
            lhs->tail_idx = p;                                                \
    }                                                                         \
                                                                              \
    static bool name##_rehash(name *lhs, size_t new_cap)                      \
    {                                                                         \
        name##_bucket *old_buckets = lhs->buckets;                            \
        size_t old_head = lhs->head_idx;                                      \
        size_t old_cap = lhs->capacity;                                       \
                                                                              \
        lhs->buckets = AB_LHS_MALLOC(new_cap * sizeof(name##_bucket));        \
        if (!lhs->buckets)                                                    \
        {                                                                     \
            lhs->buckets = old_buckets;                                       \
            return false;                                                     \
        }                                                                     \
        for (size_t i = 0; i < new_cap; i++)                                  \
            lhs->buckets[i].occupied = false;                                 \
        lhs->capacity = new_cap;                                              \
        lhs->size = 0;                                                        \
        lhs->head_idx = AB_LHS_NULL_IDX;                                      \
        lhs->tail_idx = AB_LHS_NULL_IDX;                                      \
                                                                              \
        size_t cur = old_head;                                                \
        while (cur != AB_LHS_NULL_IDX)                                        \
        {                                                                     \
            size_t next_cur = old_buckets[cur].next_idx;                      \
            name##_add(lhs, old_buckets[cur].data);                           \
            cur = next_cur;                                                   \
        }                                                                     \
                                                                              \
        AB_LHS_FREE(old_buckets);                                             \
        (void)old_cap;                                                        \
        return true;                                                          \
    }                                                                         \
                                                                              \
    bool name##_add(name *lhs, type element)                                  \
    {                                                                         \
        if (!name##_valid(lhs))                                               \
            return false;                                                     \
                                                                              \
        if (lhs->capacity == 0)                                               \
        {                                                                     \
            lhs->buckets = AB_LHS_MALLOC(                                     \
                AB_LHS_INITIAL_CAPACITY * sizeof(name##_bucket));             \
            if (!lhs->buckets)                                                \
                return false;                                                 \
            for (size_t i = 0; i < AB_LHS_INITIAL_CAPACITY; i++)              \
                lhs->buckets[i].occupied = false;                             \
            lhs->capacity = AB_LHS_INITIAL_CAPACITY;                          \
        }                                                                     \
                                                                              \
        if (name##_needs_resize(lhs))                                         \
            if (!name##_rehash(lhs, lhs->capacity * 2))                       \
                return false;                                                 \
                                                                              \
        size_t mask = lhs->capacity - 1;                                      \
        size_t i = hash_fn(element) & mask;                                   \
        while (lhs->buckets[i].occupied)                                      \
        {                                                                     \
            if (equals_fn(lhs->buckets[i].data, element))                     \
                return false;                                                 \
            i = (i + 1) & mask;                                               \
        }                                                                     \
                                                                              \
        lhs->buckets[i].data = element;                                       \
        lhs->buckets[i].occupied = true;                                      \
        name##_list_append(lhs, i);                                           \
        lhs->size++;                                                          \
        return true;                                                          \
    }                                                                         \
                                                                              \
    bool name##_contains(const name *lhs, type element)                       \
    {                                                                         \
        return name##_find_idx(lhs, element) != AB_LHS_NULL_IDX;              \
    }                                                                         \
                                                                              \
    bool name##_remove(name *lhs, type element)                               \
    {                                                                         \
        if (!name##_valid(lhs) || lhs->capacity == 0)                         \
            return false;                                                     \
        size_t mask = lhs->capacity - 1;                                      \
        size_t i = hash_fn(element) & mask;                                   \
        while (lhs->buckets[i].occupied)                                      \
        {                                                                     \
            if (equals_fn(lhs->buckets[i].data, element))                     \
                break;                                                        \
            i = (i + 1) & mask;                                               \
        }                                                                     \
        if (!lhs->buckets[i].occupied)                                        \
            return false;                                                     \
                                                                              \
        name##_list_remove(lhs, i);                                           \
        lhs->buckets[i].occupied = false;                                     \
        lhs->size--;                                                          \
                                                                              \
        size_t gap = i;                                                       \
        size_t j = (gap + 1) & mask;                                          \
        while (lhs->buckets[j].occupied)                                      \
        {                                                                     \
            size_t natural = hash_fn(lhs->buckets[j].data) & mask;            \
            bool should_shift =                                               \
                (j > gap)                                                     \
                    ? (natural <= gap || natural > j)                         \
                    : (natural <= gap && natural > j);                        \
                                                                              \
            if (should_shift)                                                 \
            {                                                                 \
                /* Fix up list neighbors to point to gap instead of j */      \
                size_t p = lhs->buckets[j].prev_idx;                          \
                size_t n = lhs->buckets[j].next_idx;                          \
                if (p != AB_LHS_NULL_IDX)                                     \
                    lhs->buckets[p].next_idx = gap;                           \
                else                                                          \
                    lhs->head_idx = gap;                                      \
                if (n != AB_LHS_NULL_IDX)                                     \
                    lhs->buckets[n].prev_idx = gap;                           \
                else                                                          \
                    lhs->tail_idx = gap;                                      \
                                                                              \
                lhs->buckets[gap] = lhs->buckets[j];                          \
                lhs->buckets[j].occupied = false;                             \
                gap = j;                                                      \
            }                                                                 \
            j = (j + 1) & mask;                                               \
        }                                                                     \
        return true;                                                          \
    }                                                                         \
                                                                              \
    /*                                                                        \
     * Iterate in insertion order. Pass AB_LHS_NULL_IDX to get the first      \
     * element. Returns AB_LHS_NULL_IDX when iteration is complete.           \
     *                                                                        \
     * Usage:                                                                 \
     *   size_t it = AB_LHS_NULL_IDX;                                         \
     *   type *elem;                                                          \
     *   while ((elem = MySet_next(&s, &it)) != NULL)                         \
     *       do_something(*elem);                                             \
     */                                                                       \
    type *name##_next(const name *lhs, size_t *it)                            \
    {                                                                         \
        if (!name##_valid(lhs) || !it)                                        \
            return NULL;                                                      \
        size_t cur = (*it == AB_LHS_NULL_IDX) ? lhs->head_idx                 \
                                              : lhs->buckets[*it].next_idx;   \
        if (cur == AB_LHS_NULL_IDX)                                           \
            return NULL;                                                      \
        *it = cur;                                                            \
        return &lhs->buckets[cur].data;                                       \
    }                                                                         \
                                                                              \
    void name##_clear(name *lhs)                                              \
    {                                                                         \
        if (!name##_valid(lhs) || !lhs->buckets)                              \
            return;                                                           \
        for (size_t i = 0; i < lhs->capacity; i++)                            \
            lhs->buckets[i].occupied = false;                                 \
        lhs->size = 0;                                                        \
        lhs->head_idx = AB_LHS_NULL_IDX;                                      \
        lhs->tail_idx = AB_LHS_NULL_IDX;                                      \
    }                                                                         \
                                                                              \
    size_t name##_size(const name *lhs)                                       \
    {                                                                         \
        if (!name##_valid(lhs))                                               \
            return 0;                                                         \
        return lhs->size;                                                     \
    }                                                                         \
                                                                              \
    bool name##_is_empty(const name *lhs)                                     \
    {                                                                         \
        return name##_size(lhs) == 0;                                         \
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

#define AB_LHS_DEFINE(type, name, hash_fn, equals_fn)                   \
                                                                        \
    typedef struct                                                      \
    {                                                                   \
        type data;                                                      \
        bool occupied;                                                  \
        size_t prev_idx;                                                \
        size_t next_idx;                                                \
    } name##_bucket;                                                    \
                                                                        \
    typedef struct                                                      \
    {                                                                   \
        name##_bucket *buckets;                                         \
        size_t size;                                                    \
        size_t capacity;                                                \
        size_t head_idx; /* first inserted, AB_LHS_NULL_IDX if empty */ \
        size_t tail_idx; /* last inserted,  AB_LHS_NULL_IDX if empty */ \
    } name;                                                             \
                                                                        \
    static inline bool name##_valid(const name *lhs)                    \
    {                                                                   \
        return lhs != NULL;                                             \
    }                                                                   \
                                                                        \
    AB_LHS_DEFINE_FUNCTIONS(type, name, hash_fn, equals_fn)

#endif // AB_LHS_H