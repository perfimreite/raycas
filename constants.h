#include "meta.h"
#include <assert.h>

#ifndef CONSTANTS_H
#define CONSTANTS_H

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define COLS 16
#define ROWS 12
#define CELL_SIZE ((WINDOW_WIDTH) / (COLS))

static_assert(WINDOW_WIDTH / COLS == WINDOW_HEIGHT / ROWS);

#endif // CONSTANTS_H

