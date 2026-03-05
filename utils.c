#include "utils.h" 

f64 time_in_seconds(void)
{
    struct timespec ts;
    timespec_get(&ts, TIME_UTC);
    return ts.tv_sec + ts.tv_nsec / 1e9;
}

f32 degrees_to_radians(f32 angle)
{
    return angle * M_PI / 180.0f;
}

