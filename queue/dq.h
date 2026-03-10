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

#endif // AB_DQ_H