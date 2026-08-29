#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>

#ifndef BASE_H
#define BASE_H

/* === Assert === */
#define DEBUG

#ifdef DEBUG
#define ASSERT(x) assert(x)
#else
#define Assert(x)
#endif // DEBUG

/* === types === */
typedef int8_t   i8;
typedef int16_t  i16;
typedef int32_t  i32;
typedef int64_t  i64;
typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef float    f32;
typedef double   f64;
typedef int16_t  b16;
typedef int32_t  b32;

/* === descriptive alternatives for `static` === */
#define internal      static
#define global        static
#define local_persist static

/* === useful macros whilst implementing new code === */
#define UNUSED(value) (void)(value)
#define TODO(message) do { fprintf(stderr, "%s:%d: TODO: %s\n", __FILE__, __LINE__, message); abort(); } while(0)
#define UNREACHABLE(message) do { fprintf(stderr, "%s:%d: UNREACHABLE: %s\n", __FILE__, __LINE__, message); abort(); } while(0)

/* === helper macros for math === */
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define CLAMP(x, min, max) MIN(MAX((x), (min)), (max))
#define SWAP(T, a, b) do {T t = a; a = b; b = t} while (0)

/* === helper macros for static arrays === */
#define ARRAY_COUNT(arr) (sizeof((arr)) / sizeof((arr)[0]))

#define ARRAY_PRINT(arr, fmt)                                 \
    do {                                                      \
        printf("%s = {", #arr);                               \
        for (size_t i = 0; i < ARRAY_COUNT((arr)) - 1; i++) { \
            printf(fmt", ", (arr)[i]);                        \
        }                                                     \
        printf(fmt"}\n", (arr)[ARRAY_COUNT((arr)) - 1]);      \
    } while (0)

#define LIST_PUSH(list, el)                                                            \
    do {                                                                               \
        if ((list).count >= (list).capacity) {                                         \
            if ((list).capacity == 0) {                                                \
                (list).capacity = 256;                                                 \
            } else {                                                                   \
                (list).capacity *= 2;                                                  \
            }                                                                          \
        }                                                                              \
        (list).items = realloc((list).items, (list).capacity * sizeof(*(list).items)); \
        (list).items[(list).count++] = (el);                                           \
    } while (0)

#define LIST_POP(list) \
    (list).items[--(list).count]

#define LIST_FOR_EACH(T, list, el) \
    for (T *(el) = (list).items; (el) < &(list).items[(list).count]; (el)++)

#endif // BASE_H
