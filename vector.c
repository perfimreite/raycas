#include "constants.h"
#include "vector.h"

#include "meta.h"
#include "math.h"

#include <assert.h>
#include <stdbool.h>

V2 zero_vector = { .x = 0, .y = 0 };

bool v2f_eq(V2f a, V2f b)
{
	return a.x == b.x && a.y == b.y;
}

V2f v2f_add(V2f a, V2f b)
{
	return (V2f){ .x = a.x + b.x, .y = a.y + b.y };
}

V2f v2f_sub(V2f a, V2f b)
{
	return (V2f){ .x = a.x - b.x, .y = a.y - b.y };
}

V2f v2f_mul(V2f a, V2f b)
{
	return (V2f){ .x = a.x * b.x, .y = a.y * b.y };
}

V2f v2f_div(V2f a, V2f b)
{
	return (V2f){ .x = a.x / b.x, .y = a.y / b.y };
}

V2f v2f_cell(V2f a)
{
    return v2f_scale(v2f_unit(a), (f32)CELL_SIZE);
}

V2f v2f_normal(V2f a)
{
	return (V2f){ .x = -a.y, .y = a.x };
}

V2f v2f_scale(V2f a, f32 k)
{
	return (V2f){ .x = a.x * k, .y = a.y * k };
}

bool v2f_zero(V2f a)
{
    return v2f_eq(a, v2_to_v2f(zero_vector));
}

f32 v2f_square_len(V2f a)
{
    return a.x * a.x + a.y * a.y;
}

f32 v2f_len(V2f a)
{
    return sqrt(v2f_square_len(a));
}

V2f v2f_unit(V2f a)
{
	// assert(!v2_zero(a));

    if (v2f_zero(a)) return a;

	f32 l = v2f_len(a);
	return v2f_scale(a, 1.0f / l);
}

V2f v2_to_v2f(V2 a)
{
    return (V2f){ .x = (f32)a.x, .y = (f32)a.y };
}

V2 v2f_to_v2(V2f a)
{
    return (V2){ .x = a.x, .y = a.y };
}

f64 v2f_dot_product(V2f a, V2f b)
{
	return a.x * b.x + a.y * b.y;
}

V2f v2f_rotate(V2f a, f32 angle)
{
    f32 c_angle = cos(angle);
    f32 s_angle = sin(angle);

    f32 x = a.x * c_angle - a.y * s_angle;
    f32 y = a.x * s_angle + a.y * c_angle;

    return (V2f){ .x = x, .y = y };
}

