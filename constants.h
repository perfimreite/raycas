#include "meta.h"
#include <assert.h>

#ifndef CONSTANTS_H
#define CONSTANTS_H

#ifdef HIGH_RESOLUTION
#define WINDOW_WIDTH 1600
#define WINDOW_HEIGHT 1200
#else
#define WINDOW_WIDTH 960
#define WINDOW_HEIGHT 720
#endif

#define WINDOW_CENTER_X (((WINDOW_WIDTH)  - 1.0f) / 2.0f)
#define WINDOW_CENTER_Y (((WINDOW_HEIGHT) - 1.0f) / 2.0f)

#define COLS 16
#define ROWS 12
#define CELL_SIZE ((f32)(WINDOW_WIDTH) / (COLS))
#define WALL_HEIGHT_MULTIPLIER ((f32)(WINDOW_HEIGHT) / 16)

static_assert(WINDOW_WIDTH / COLS == WINDOW_HEIGHT / ROWS);

#define MAP_COUNT 2

#define FONT_FILE "fonts/CascadiaMono.ttf"
#define MAX_FONT_COUNT 32

#endif // CONSTANTS_H

