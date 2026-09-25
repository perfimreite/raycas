#include "base.h"

#ifndef VECTOR_H
#define VECTOR_H

typedef struct {
    f32 x;
    f32 y;
} V2f;

typedef struct {
    i32 x;
    i32 y;
} V2i;

V2f v2f(f32 x, f32 y);

b32 v2f_eq(V2f a, V2f b);
b32 v2f_zero(V2f a);

V2f v2f_add(V2f a, V2f b);
V2f v2f_sub(V2f a, V2f b);
V2f v2f_mul(V2f a, V2f b);
V2f v2f_div(V2f a, V2f b);
V2f v2f_unit(V2f a);
V2f v2f_normal(V2f a);
V2f v2f_scale(V2f a, f32 k);
V2f v2f_rotate(V2f a, f32 angle);
V2f v2f_floor(V2f a);
V2f v2f_from_v2i(V2i a);

f32 v2f_square_len(V2f a);
f32 v2f_len(V2f a);
f32 v2f_dot_product(V2f a, V2f b);

//

V2i v2i(i32 x, i32 y);

b32 v2i_eq(V2i a, V2i b);
b32 v2i_zero(V2i a);

V2i v2i_add(V2i a, V2i b);
V2i v2i_sub(V2i a, V2i b);
V2i v2i_mul(V2i a, V2i b);
V2i v2i_div(V2i a, V2i b);
V2i v2i_unit(V2i a);
V2i v2i_normal(V2i a);
V2i v2i_scale(V2i a, i32 k);
V2i v2i_rotate(V2i a, i32 angle);
V2i v2i_floor(V2i a);
V2i v2i_from_v2f(V2f a);

i32 v2i_square_len(V2i a);
i32 v2i_len(V2i a);
i32 v2i_dot_product(V2i a, V2i b);

#endif // VECTOR_H
