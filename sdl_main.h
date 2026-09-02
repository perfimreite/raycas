#include "base.h"

#include <SDL2/SDL_mouse.h>
#include <SDL2/SDL_pixels.h>
#include <SDL2/SDL_rect.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

typedef struct {
    const char *path;
    i32 ptsize;
    TTF_Font *font;
} Font;

typedef struct {
    Font *items;
    u64 count;
    u64 capacity;
} Fonts;

typedef struct {
    f64 start;
    f64 end;
    f64 dt;
} Frame_Time;

