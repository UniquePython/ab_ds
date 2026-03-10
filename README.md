# ab_ds

A typed, generic data structures library for C, implemented as single-file headers.
Inspired by the stb-style single-header approach — drop a file in, define one macro, go.

---

## Design

- **Typed generics via macros.** No `void*`, no casting. Every container is generated for a specific type at the call site.
- **stb-style implementation guard.** Define `AB_<NAME>_IMPLEMENTATION` in exactly one `.c` file before including.
- **Consistent API shape.** Every container has `_init`, `_free`, `_clear`, `_size`, `_is_empty`, `_contains`. Return types signal success/failure — no sentinel values disguised as data.
- **Configurable allocator.** Every header exposes `AB_<NAME>_MALLOC`/`FREE`/`REALLOC` overrides.

---

## Quick Start

```c
// In ONE .c file:
#define AB_AL_IMPLEMENTATION
#include "list/al.h"

// Define a typed list anywhere (header or .c):
#define int_eq(a, b) ((a) == (b))
AB_AL_DEFINE(int, IntList, int_eq)

// Use it:
IntList list;
IntList_init(&list);
IntList_push_back(&list, 1);
IntList_push_back(&list, 2);

int *val = IntList_get(&list, 0);  // => 1
IntList_free(&list);
```

To include everything at once:

```c
#define AB_DS_IMPLEMENTATION
#include "ab_ds.h"
```

Individual macros (`AB_AL_IMPLEMENTATION`, `AB_HS_IMPLEMENTATION`, etc.) still work
if you only want specific containers.

---

## Containers

### Lists — `list/`

| Header | Type             | Backing       | Notes                                        |
| ------ | ---------------- | ------------- | -------------------------------------------- |
| `al.h` | ArrayList        | Dynamic array | O(1) amortised push_back, O(n) insert/remove |
| `sl.h` | SinglyLinkedList | Linked nodes  | O(1) push_front, O(n) pop_back               |
| `dl.h` | DoublyLinkedList | Linked nodes  | O(1) push/pop both ends, O(n/2) get/insert   |

**Define macro:** `AB_AL_DEFINE(type, name, equals_fn)`
**Common API:**
```
_init        _free        _clear
_push_back   _push_front  (sl, dl only)
_pop_back    _pop_front   (sl, dl only)
_insert(index, element)
_remove(index, *out) → bool
_get(index)  → type*
_set(index, element, *out) → bool
_size        _is_empty
_index_of    _contains
```

---

### Queue — `queue/`

| Header | Type  | Backing     | Notes                                       |
| ------ | ----- | ----------- | ------------------------------------------- |
| `dq.h` | Deque | Ring buffer | O(1) push/pop both ends, O(1) random access |

**Define macro:** `AB_DQ_DEFINE(type, name, equals_fn)`
**API:**
```
_init          _free        _clear
_push_back     _push_front
_pop_back      _pop_front   → bool
_peek_front    _peek_back   → type*
_get(index)    _set(index, element, *out) → bool
_size          _is_empty
_index_of      _contains
```

---

### Sets — `set/`

| Header  | Type          | Backing                         | Notes                                          |
| ------- | ------------- | ------------------------------- | ---------------------------------------------- |
| `ss.h`  | SortedSet     | Sorted array                    | O(log n) lookup, O(n) insert; NavigableSet API |
| `hs.h`  | HashSet       | Open addressing, linear probing | O(1) avg; backward-shift deletion              |
| `lhs.h` | LinkedHashSet | HashSet + insertion-order list  | O(1) avg; insertion-order iteration            |
| `ts.h`  | TreeSet       | Red-black tree                  | O(log n) all ops; NavigableSet API             |

**Define macros:**
```c
AB_SS_DEFINE(type, name, cmp_fn)              // cmp_fn(a,b) → int
AB_HS_DEFINE(type, name, hash_fn, equals_fn)
AB_LHS_DEFINE(type, name, hash_fn, equals_fn)
AB_TS_DEFINE(type, name, cmp_fn)
```

**Common API (all sets):**
```
_init    _free    _clear
_add(element)     → bool   (false if duplicate)
_remove(element)  → bool   (false if not found)
_contains         → bool
_size             _is_empty
```

**NavigableSet API (ss, ts only):**
```
_first     _last
_ceiling   _higher    (first element >= / > query)
_floor     _lower     (last element <= / < query)
_get(index)           (ss only — index into sorted order)
```

**Iteration:**
```c
// LinkedHashSet — insertion order:
size_t it = AB_LHS_NULL_IDX;
type *elem;
while ((elem = MySet_next(&s, &it)) != NULL) { ... }

// TreeSet — sorted order:
MyTreeSet_node *it = NULL;
type *elem;
while ((elem = MySet_next(&ts, &it)) != NULL) { ... }
```

---

## Common Functions — `ab_ds_common.h`

Included automatically via `ab_ds.h`. Can also be included standalone — no implementation guard needed.

| Category    | Functions                                                                                              |
| ----------- | ------------------------------------------------------------------------------------------------------ |
| Equality    | `ab_eq_i8/16/32/64`, `ab_eq_u8/16/32/64`, `ab_eq_f32/64`, `ab_eq_char`, `ab_eq_ptr`, `ab_eq_str`       |
| Comparators | `ab_cmp_i8/16/32/64`, `ab_cmp_u8/16/32/64`, `ab_cmp_f32/64`, `ab_cmp_char`, `ab_cmp_ptr`, `ab_cmp_str` |
| Hash        | `ab_hash_i8/16/32/64`, `ab_hash_u8/16/32/64`, `ab_hash_char`, `ab_hash_ptr`, `ab_hash_str`             |

```c
// Integer hash set:
AB_HS_DEFINE(int32_t, IntSet, ab_hash_i32, ab_eq_i32)

// String tree set:
AB_TS_DEFINE(const char*, StrTreeSet, ab_cmp_str)

// Float sorted set (exact equality — define your own for epsilon comparison):
AB_SS_DEFINE(float, FloatSet, ab_cmp_f32)
```

---



```c
// Comparator — same convention as strcmp / qsort:
#define int_cmp(a, b) ((a) > (b)) - ((a) < (b))
#define str_cmp(a, b) strcmp((a), (b))

// Equality:
#define int_eq(a, b)  ((a) == (b))
#define str_eq(a, b)  (strcmp((a), (b)) == 0)

// Hash:
#define int_hash(x)   ((size_t)(x) * 2654435761UL)   // Knuth multiplicative

static size_t str_hash(const char *s) {
    size_t h = 14695981039346656037ULL;               // FNV-1a
    while (*s) { h ^= (unsigned char)*s++; h *= 1099511628211ULL; }
    return h;
}
```

---

## Allocator Override

Any allocator can be substituted before including a header:

```c
#define AB_AL_REALLOC  my_realloc
#define AB_AL_FREE     my_free
#include "list/al.h"
```

Each header has its own prefix (`AB_AL_`, `AB_SL_`, `AB_DL_`, `AB_DQ_`, `AB_SS_`, `AB_HS_`, `AB_LHS_`, `AB_TS_`).

---

## License

MIT — see individual file headers.

## Version

0.1.0