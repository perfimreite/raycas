#include "constants.h"
#include "vector.h"

#include "meta.h"
#include "math.h"

#include <assert.h>
#include <stdbool.h>

static const V2 zero_vector = { .x = 0, .y = 0 };

bool v2_eq(V2 a, V2 b)
{
	return a.x == b.x && a.y == b.y;
}

V2 v2_add(V2 a, V2 b)
{
	return (V2){ .x = a.x + b.x, .y = a.y + b.y };
}

V2 v2_sub(V2 a, V2 b)
{
	return (V2){ .x = a.x - b.x, .y = a.y - b.y };
}

V2 v2_mul(V2 a, V2 b)
{
	return (V2){ .x = a.x*b.x, .y = a.y*b.y };
}

V2 v2_div(V2 a, V2 b)
{
	return (V2){ .x = a.x/b.x, .y = a.y/b.y };
}

V2 v2_cell(V2 a)
{
    return v2f_to_v2(v2f_scale(v2f_unit(a), CELL_SIZE_F32));
}

V2 v2_normal(V2 a)
{
	return (V2){ .x = -1*a.y, .y = a.x };
}

V2  v2_scale(V2 a, i32 k)
{
	return (V2){ .x = a.x * k, .y = a.y * k };
}

bool v2_zero(V2 a)
{
    return v2_eq(a, zero_vector);
}

i32 v2_square_len(V2 a)
{
    return a.x*a.x + a.y*a.y;
}

f32 v2_len(V2 a)
{
    return sqrt(v2_square_len(a));
}

V2f v2f_scale(V2f a, f32 k)
{
	return (V2f){ .x = (f32)a.x * k, .y = (f32)a.y * k };
}

V2f v2f_unit(V2 a)
{
	// assert(!v2_zero(a));

    if (v2_zero(a)) return v2_to_v2f(a);

	f32 l = v2_len(a);
	return v2f_scale(v2_to_v2f(a), 1.0f / l);
}

V2f v2_to_v2f(V2 a)
{
    return (V2f){ .x = (f32)a.x, .y = (f32)a.y };
}

V2 v2f_to_v2(V2f a)
{
    return (V2){ .x = a.x, .y = a.y };
}
