#include "utils.h"

#if defined(_WIN32)
    #define _USE_MATH_DEFINES
#endif
#include <math.h>
#ifndef M_PI
    #define M_PI 3.14159265358979323846
#endif

#include <time.h>

f64 time_in_seconds(void)
{
    struct timespec ts;
    timespec_get(&ts, TIME_UTC);
    return ts.tv_sec + ts.tv_nsec / 1e9;
}

f32 radians_from_degrees(f32 angle)
{
    return angle * M_PI / 180.0f;
}

f32 degrees_from_radians(f32 angle)
{
    return angle * 180.0f / M_PI;
}

