#include "vector.h"

#include "meta.h"
#include "math.h"

#include <assert.h>
#include <stdbool.h>

static V2 zero_vector = { .x = 0, .y = 0 };

bool vec2_eq(V2 a, V2 b)
{
	return a.x == b.x && a.y == b.y;
}

V2 vec2_add(V2 a, V2 b)
{
	return (V2){ .x = a.x + b.x, .y = a.y + b.y };
}

V2 vec2_sub(V2 a, V2 b)
{
	return (V2){ .x = a.x - b.x, .y = a.y - b.y };
}

V2 vec2_mul(V2 a, V2 b)
{
	return (V2){ .x = a.x*b.x, .y = a.y*b.y };
}

V2 vec2_div(V2 a, V2 b)
{
	return (V2){ .x = a.x/b.x, .y = a.y/b.y };
}

bool vec2_zero(V2 a)
{
    return vec2_eq(a, zero_vector);
}

i32 vec2_square_len(V2 a)
{
    return a.x*a.x + a.y*a.y;
}

f32 vec2_len(V2 a)
{
    return sqrt(vec2_square_len(a));
}

V2 vec2_scale(V2 a, i32 k)
{
	return (V2){ .x = a.x*k, .y = a.y*k };
}

V2 vec2_unit(V2 a)
{
	assert(!vec2_zero(a));
	i32 unit = 50.f;
	// NOTE: we first divide by unit instead of scaling
	// at the end in order to not lose precision
	f32 l = vec2_len(a) / unit;
	V2 v = { .x = (f32)a.x / l, .y = (f32)a.y / l};
	return v;
}

V2 vec2_normal(V2 a)
{
	return (V2){ .x = -1*a.y, .y = a.x };
}

