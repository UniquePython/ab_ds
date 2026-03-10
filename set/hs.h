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
#define AB_HS_INITIAL_CAPACITY 8 /* must be power of 2 */
#endif

#define AB_HS_LOAD_FACTOR_NUM 3
#define AB_HS_LOAD_FACTOR_DEN 4

/*
 * hash_fn(element) -> size_t
 * equals_fn(a, b)  -> bool
 *
 * Example for int:
 *   #define int_hash(x)    ((size_t)(x) * 2654435761UL)   // Knuth multiplicative
 *   #define int_eq(a, b)   ((a) == (b))
 *
 * Example for strings:
 *   static size_t str_hash(const char *s) {
 *       size_t h = 14695981039346656037ULL;
 *       while (*s) { h ^= (unsigned char)*s++; h *= 1099511628211ULL; }
 *       return h;
 *   }
 *   #define str_eq(a, b) (strcmp((a), (b)) == 0)
 */

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
            */                                                       \
                bool should_shift =                                  \
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

#endif // AB_HS_H