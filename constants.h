#include "base.h"
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

#define DEFAULT_TEXTURE_WIDTH 64
#define DEFAULT_TEXTURE_HEIGHT 64
static_assert(DEFAULT_TEXTURE_WIDTH == DEFAULT_TEXTURE_HEIGHT);

#define WINDOW_CENTER_X (((WINDOW_WIDTH)  - 1.0f) / 2.0f)
#define WINDOW_CENTER_Y (((WINDOW_HEIGHT) - 1.0f) / 2.0f)

#define COLS 16
#define ROWS 12
static_assert(WINDOW_WIDTH / COLS == WINDOW_HEIGHT / ROWS);
#define CELL_SIZE ((f32)(WINDOW_WIDTH) / (COLS))

#define OVERLAY_TEXT_SIZE 128

#define MAP_COUNT 2

#define FONT_FILE "fonts/CascadiaMono.ttf"
#define TEXTURES_PATH "assets/wolfenstein_textures/"
#define FIRST_TEXTURE_ID 2

#endif // CONSTANTS_H

