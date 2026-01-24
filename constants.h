#include "meta.h"
#include <assert.h>

#ifndef CONSTANTS_H
#define CONSTANTS_H

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define COLS 16
#define ROWS 12
#define CELL_SIZE (WINDOW_WIDTH / COLS)
#define CELL_SIZE_F32 ((f32)WINDOW_WIDTH / COLS)

static_assert((f32)WINDOW_WIDTH / COLS == (f32)WINDOW_HEIGHT / ROWS);

#endif // CONSTANTS_H

