#include "meta.h"

#include <stdbool.h>

#ifndef VECTOR_H
#define VECTOR_H

typedef struct {
    f32 x;
    f32 y;
} V2f;

bool v2f_zero(V2f a);
bool v2f_eq(V2f a, V2f b);

V2f make_v2f(f32 x, f32 y);
V2f v2f_add(V2f a, V2f b);
V2f v2f_sub(V2f a, V2f b);
V2f v2f_unit(V2f a);
V2f v2f_cell(V2f a);
V2f v2f_normal(V2f a);
V2f v2f_scale(V2f a, f32 k);
V2f v2f_rotate(V2f a, f32 angle);
V2f v2f_floor(V2f a);

f32 v2f_square_len(V2f a);
f32 v2f_len(V2f a);
f32 v2f_dot_product(V2f a, V2f b);

#endif // VECTOR_H
