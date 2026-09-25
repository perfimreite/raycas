#include "vector.h"

#if defined(_WIN32)
    #define _USE_MATH_DEFINES
#endif
#include <math.h>

V2f v2f(f32 x, f32 y)
{
    return (V2f){ .x = x, .y = y};
}

b32 v2f_eq(V2f a, V2f b)
{
    return a.x == b.x && a.y == b.y;
}

b32 v2f_zero(V2f a)
{
    return v2f_eq(a, v2f(0.0f, 0.0f));
}

V2f v2f_add(V2f a, V2f b)
{
    return v2f(a.x + b.x, a.y + b.y);
}

V2f v2f_sub(V2f a, V2f b)
{
    return v2f(a.x - b.x, a.y - b.y);
}

V2f v2f_mul(V2f a, V2f b)
{
    return v2f(a.x * b.x, a.y * b.y);
}

V2f v2f_div(V2f a, V2f b)
{
    ASSERT(b.x != 0.0f && b.y != 0.0f);
    return v2f(a.x / b.x, a.y / b.y);
}

V2f v2f_unit(V2f a)
{
    if (v2f_zero(a)) {
        return a;
    }

    f32 l = v2f_len(a);
    return v2f_scale(a, 1.0f / l);
}

V2f v2f_normal(V2f a)
{
    return v2f(-a.y, a.x);
}

V2f v2f_scale(V2f a, f32 k)
{
    return v2f(a.x * k, a.y * k);
}

V2f v2f_rotate(V2f a, f32 angle)
{
    f32 c_angle = cosf(angle);
    f32 s_angle = sinf(angle);
    return v2f(a.x * c_angle - a.y * s_angle, a.x * s_angle + a.y * c_angle);
}

V2f v2f_floor(V2f a)
{
    return v2f(floor(a.x), floor(a.y));
}

V2f v2f_from_v2i(V2i a)
{
    return v2f(a.x, a.y);
}

f32 v2f_square_len(V2f a)
{
    return a.x * a.x + a.y * a.y;
}

f32 v2f_len(V2f a)
{
    return sqrt(v2f_square_len(a));
}

f32 v2f_dot_product(V2f a, V2f b)
{
    return a.x * b.x + a.y * b.y;
}

//

V2i v2i(i32 x, i32 y)
{
    return (V2i){ .x = x, .y = y};
}

b32 v2i_eq(V2i a, V2i b)
{
    return a.x == b.x && a.y == b.y;
}

b32 v2i_zero(V2i a)
{
    return v2i_eq(a, v2i(0, 0));
}

V2i v2i_add(V2i a, V2i b)
{
    return v2i(a.x + b.x, a.y + b.y);
}

V2i v2i_sub(V2i a, V2i b)
{
    return v2i(a.x - b.x, a.y - b.y);
}

V2i v2i_mul(V2i a, V2i b)
{
    return v2i(a.x * b.x, a.y * b.y);
}

V2i v2i_div(V2i a, V2i b)
{
    ASSERT(b.x != 0 && b.y != 0);
    return v2i(a.x / b.x, a.y / b.y);
}

V2i v2i_unit(V2i a)
{
    if (v2i_zero(a)) {
        return a;
    }

    i32 l = v2i_len(a);
    return v2i_scale(a, 1 / l);
}

V2i v2i_normal(V2i a)
{
    return v2i(-a.y, a.x);
}

V2i v2i_scale(V2i a, i32 k)
{
    return v2i(a.x * k, a.y * k);
}

V2i v2i_rotate(V2i a, i32 angle)
{
    i32 c_angle = cosf(angle);
    i32 s_angle = sinf(angle);

    return v2i(a.x * c_angle - a.y * s_angle, a.x * s_angle + a.y * c_angle);
}

V2i v2i_floor(V2i a)
{
    return v2i(floor(a.x), floor(a.y));
}

V2i v2i_from_v2f(V2f a)
{
    return v2i(a.x, a.y);
}

i32 v2i_square_len(V2i a)
{
    return a.x * a.x + a.y * a.y;
}

i32 v2i_len(V2i a)
{
    return sqrt(v2i_square_len(a));
}

i32 v2i_dot_product(V2i a, V2i b)
{
    return a.x * b.x + a.y * b.y;
}

