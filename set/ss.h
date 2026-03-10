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

/*
 * cmp_fn(a, b) must return:
 *   < 0  if a < b
 *     0  if a == b
 *   > 0  if a > b
 *
 * Example for int:
 *   #define int_cmp(a, b) ((a) - (b))
 *
 * Example for strings:
 *   #define str_cmp(a, b) strcmp((a), (b))
 */

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

#endif // AB_SS_H