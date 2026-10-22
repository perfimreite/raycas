#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_mouse.h>
#include <SDL3/SDL_pixels.h>
#include <SDL3/SDL_rect.h>
#include <SDL3_ttf/SDL_ttf.h>

#include "base.h"

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
    SDL_AudioStream **items;
    u64 count;
    u64 capacity;
} Audio_Streams;

typedef struct {
    f64 start;
    f64 end;
    f64 dt;
} Frame_Time;

