#include "constants.h"
#include "vector.h"

#include <math.h>

V2f zero_vector = { .x = 0, .y = 0 };

bool v2f_zero(V2f a)
{
    return v2f_eq(a, zero_vector);
}

bool v2f_eq(V2f a, V2f b)
{
    return a.x == b.x && a.y == b.y;
}

V2f make_v2f(f32 x, f32 y)
{
    return (V2f){ .x = x, .y = y};
}

V2f v2f_add(V2f a, V2f b)
{
    return (V2f){ .x = a.x + b.x, .y = a.y + b.y };
}

V2f v2f_sub(V2f a, V2f b)
{
    return (V2f){ .x = a.x - b.x, .y = a.y - b.y };
}

V2f v2f_unit(V2f a)
{
    if (v2f_zero(a)) {
        return a;
    }

    f32 l = v2f_len(a);
    return v2f_scale(a, 1.0f / l);
}

V2f v2f_cell(V2f a)
{
    return v2f_scale(v2f_unit(a), CELL_SIZE);
}

V2f v2f_normal(V2f a)
{
    return (V2f){ .x = -a.y, .y = a.x };
}

V2f v2f_scale(V2f a, f32 k)
{
    return (V2f){ .x = a.x * k, .y = a.y * k };
}

V2f v2f_rotate(V2f a, f32 angle)
{
    f32 c_angle = cosf(angle);
    f32 s_angle = sinf(angle);

    f32 x = a.x * c_angle - a.y * s_angle;
    f32 y = a.x * s_angle + a.y * c_angle;

    return (V2f){ .x = x, .y = y };
}

V2f v2f_floor(V2f a)
{
    return (V2f){ .x = floor(a.x), .y = floor(a.y)};
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

