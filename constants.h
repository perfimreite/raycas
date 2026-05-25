#include "meta.h"
#include <assert.h>

#ifndef CONSTANTS_H
#define CONSTANTS_H

#define WINDOW_WIDTH 1600
#define WINDOW_HEIGHT 1200
#define COLS 16
#define ROWS 12
#define CELL_SIZE ((f32)(WINDOW_WIDTH) / (COLS))

static_assert(WINDOW_WIDTH / COLS == WINDOW_HEIGHT / ROWS);

#endif // CONSTANTS_H

