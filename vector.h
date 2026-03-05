#include "meta.h"

#ifndef VECTOR_H
#define VECTOR_H

typedef struct {
	i32 x;
	i32 y;
} V2;

typedef struct {
	f32 x;
	f32 y;
} V2f;

extern V2 zero_vector;

V2 v2f_to_v2(V2f a);
V2f v2f_add(V2f a, V2f b);
V2f v2f_sub(V2f a, V2f b);
V2f v2f_mul(V2f a, V2f b);
V2f v2f_div(V2f a, V2f b);
V2f v2f_cell(V2f a);
V2f v2f_normal(V2f a);
V2f v2f_scale(V2f a, f32 k);
V2f v2f_unit(V2f a);
V2f v2_to_v2f(V2 a);
f32 v2f_len(V2f a);
f32 v2f_square_len(V2f a);
f64 v2_dot_product(V2 a, V2 b);
V2f v2f_rotate(V2f a, f32 angle);

#endif // VECTOR_H
