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

/* === helper macros for static arrays === */
#define ARRAY_COUNT(a) (sizeof((a)) / sizeof((a)[0]))

#endif // BASE_H
