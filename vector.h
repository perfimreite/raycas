#include "meta.h"

#ifndef VECTOR_H
#define VECTOR_H

typedef struct {
	i32 x;
	i32 y;
} V2;

V2 v2_add(V2 a, V2 b);

V2 v2_sub(V2 a, V2 b);

V2 v2_mul(V2 a, V2 b);

V2 v2_div(V2 a, V2 b);

i32 v2_square_len(V2 a);

f32 v2_len(V2 a);

V2 v2_scale(V2 a, i32 k);

V2 v2_unit(V2 a);

V2 v2_normal(V2 a);

#endif // VECTOR_H
