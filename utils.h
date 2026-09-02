#include "base.h"

#if defined(_WIN32)
    #define _USE_MATH_DEFINES
#endif
#include <math.h>
#include <time.h>

#ifndef UTILS_H
#define UTILS_H

f64 time_in_seconds(void);
f32 radians_from_degrees(f32 angle);
f32 degrees_from_radians(f32 angle);

#endif // UTILS_H
