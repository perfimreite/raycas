#include "meta.h"
#include "utils.h"
#include "vector.h"
#include "game.h"

#include <stdbool.h>

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

Fonts fonts = { 0 };

internal Font font_init(const char *path, i32 ptsize)
{
    return (Font) {
        .path = path,
        .ptsize = ptsize,
        .font = TTF_OpenFont(path, ptsize)
    };
}

internal Font *get_font(i32 ptsize)
{
    assert(fonts.count <= fonts.capacity);

    if (fonts.capacity == 0) {
        fonts.capacity = MAX_FONT_COUNT;
        fonts.items = malloc(fonts.capacity * sizeof(Font));
    }

    for (u32 i = 0; i < fonts.count; i++) {
        if (fonts.items[i].ptsize == ptsize) {
            return &fonts.items[i];
        }
    }

    fonts.items[fonts.count++] = font_init(FONT_FILE, ptsize);
    return &fonts.items[fonts.count-1];
}

global SDL_Window *window = NULL;
global SDL_Renderer *renderer = NULL;

internal void val_err(i32 code)
{
    if (code < 0) {
        fprintf(stderr, "SDL ERROR: %s\n", SDL_GetError());
        exit(1);
    }
}

internal void *ptr_err(void *code)
{
    if (!code) {
        fprintf(stderr, "SDL ERROR: %s\n", SDL_GetError());
        exit(1);
    }
    return code;
}

u32 platform_get_mouse_state(V2f *mouse_pos)
{
    i32 x;
    i32 y;
    u32 mouse_button_state = SDL_GetMouseState(&x, &y);
    mouse_pos->x = x;
    mouse_pos->y = y;
    return mouse_button_state;
}

internal SDL_Color translate_color(Color color)
{
     return (SDL_Color) {
        .r = color.r,
        .g = color.g,
        .b = color.b,
        .a = color.a
    };
}

internal SDL_Point translate_point(V2f point)
{
    return (SDL_Point) {
        .x = point.x,
        .y = point.y
    };
}

internal SDL_Rect translate_rect(Rect rect)
{
     return (SDL_Rect) {
        .x = rect.x,
        .y = rect.y,
        .w = rect.w,
        .h = rect.h
    };
}

void platform_draw_point(Color color, V2f a)
{
    SDL_Color sdl_color = translate_color(color);
    val_err(SDL_SetRenderDrawColor(renderer, sdl_color.r, sdl_color.g, sdl_color.b, sdl_color.a));
    val_err(SDL_RenderDrawPoint(renderer, a.x, a.y));
}

void platform_draw_line(Color color, V2f a, V2f b)
{
    SDL_Color sdl_color = translate_color(color);
    val_err(SDL_SetRenderDrawColor(renderer, sdl_color.r, sdl_color.g, sdl_color.b, sdl_color.a));
    val_err(SDL_RenderDrawLine(renderer, a.x, a.y, b.x, b.y));
}

void platform_draw_rect(Color color, Rect rect)
{
    SDL_Color sdl_color = translate_color(color);
    val_err(SDL_SetRenderDrawColor(renderer, sdl_color.r, sdl_color.g, sdl_color.b, sdl_color.a));
    SDL_Rect sdl_rect = translate_rect(rect);
    val_err(SDL_RenderFillRect(renderer, &sdl_rect));
    val_err(SDL_RenderDrawRect(renderer, &sdl_rect));
}

void platform_draw_circle(Color color, V2f center, i32 radius, bool filled)
{
    // NOTE: Midpoint circle algorithm: https://www.youtube.com/watch?v=hpiILbMkF9w
    for (i32 x = 0, y = -radius, p = -radius; x < -y; x++) {
        if (p > 0) {
            y += 1;
            p += 2 * (x + y) + 1;
        } else {
            p += 2 * x + 1;
        }

        if (filled) {
            platform_draw_line(color, make_v2f(center.x + x, center.y + y), make_v2f(center.x - x, center.y + y));
            platform_draw_line(color, make_v2f(center.x + x, center.y - y), make_v2f(center.x - x, center.y - y));
            platform_draw_line(color, make_v2f(center.x + y, center.y + x), make_v2f(center.x - y, center.y + x));
            platform_draw_line(color, make_v2f(center.x + y, center.y - x), make_v2f(center.x - y, center.y - x));
        }  else {
            platform_draw_point(color, make_v2f(center.x + x, center.y + y));
            platform_draw_point(color, make_v2f(center.x - x, center.y + y));
            platform_draw_point(color, make_v2f(center.x + x, center.y - y));
            platform_draw_point(color, make_v2f(center.x - x, center.y - y));
            platform_draw_point(color, make_v2f(center.x + y, center.y + x));
            platform_draw_point(color, make_v2f(center.x - y, center.y + x));
            platform_draw_point(color, make_v2f(center.x + y, center.y - x));
            platform_draw_point(color, make_v2f(center.x - y, center.y - x));
        }
    }
}

void platform_clear_backbuffer(Color color)
{
    val_err(SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a));
    val_err(SDL_RenderClear(renderer));
}

V2f platform_get_text_dims(i32 ptsize, const char *text)
{
    Font *font = get_font(ptsize);
    i32 w = 0;
    i32 h = 0;
    val_err(TTF_SizeUTF8(font->font, text, &w, &h));
    V2f dims = { .x = w, .y = h };
    return dims;
}

Rect platform_center_text_in_rect(Rect rect, i32 ptsize, const char *text)
{
    V2f dims = platform_get_text_dims(ptsize, text);

    return (Rect) {
        .x = rect.x + (rect.w - dims.x) / 2,
        .y = rect.y + (rect.h - dims.y) / 2,
        .w = dims.x,
        .h = dims.y
    };
}

void platform_draw_text(Color fg, Color bg, Rect rect, const char *text, i32 ptsize)
{
    SDL_Color sdl_fg = translate_color(fg);
    SDL_Color sdl_bg = translate_color(bg);

    Font *font = get_font(ptsize);

    SDL_Surface *surface = ptr_err(TTF_RenderUTF8_Shaded(font->font, text, sdl_fg, sdl_bg));
    SDL_Texture *texture = ptr_err(SDL_CreateTextureFromSurface(renderer, surface));

    SDL_Rect sdl_rect = translate_rect(rect);

    val_err(SDL_RenderCopy(renderer, texture, NULL, &sdl_rect));

    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
}

bool platform_point_in_rect(V2f point, Rect rect)
{
    SDL_Point sdl_point = translate_point(point);
    SDL_Rect sdl_rect = translate_rect(rect);
    return SDL_PointInRect(&sdl_point, &sdl_rect);
}

bool platform_mouse_left_down(u32 mouse_button_state)
{
    return mouse_button_state & SDL_BUTTON_LMASK;
}

typedef struct {
    f64 start;
    f64 end;
    f64 delta_time;
} Frame_Time;

i32 main(void)
{
    val_err(SDL_Init(SDL_INIT_EVERYTHING));
    val_err(TTF_Init());

    window = ptr_err(SDL_CreateWindow("Raycas", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, 0));
    renderer = ptr_err(SDL_CreateRenderer(window, -1, 0));
    SDL_Event event = { 0 };
    const u8 *keystate = NULL;

    buttons_init();
    overlay_init();
    player_init();
    game_init("Raycas");

    Frame_Time ft = { 0 };

    bool quit = false;
    while (!quit) {
        ft.start = time_in_seconds();

        // handle events
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                default: {} break;

                case SDL_QUIT: {
                    quit = true;
                } break;

                case SDL_MOUSEBUTTONDOWN: {
                    if (!game_process_mouse()) {
                        quit = true;
                    }
                } break;

                case SDL_KEYDOWN: {
                    game_process_key(event.key.keysym.sym);
                } break;
            }
        }

        keystate = SDL_GetKeyboardState(NULL);

        // update player position
        if (keystate[SDL_SCANCODE_W]) {
            player_move_forward(ft.delta_time);
        }
        if (keystate[SDL_SCANCODE_A]) {
            player_rotate_counterclockwise(ft.delta_time);
        }
        if (keystate[SDL_SCANCODE_S]) {
            player_move_backward(ft.delta_time);
        }
        if (keystate[SDL_SCANCODE_D]) {
            player_rotate_clockwise(ft.delta_time);
        }

        // rendering
        game_render(ft.delta_time);

        SDL_RenderPresent(renderer);

        // compute delta time
        ft.end = time_in_seconds();
        ft.delta_time = ft.end - ft.start;
    }

    TTF_Quit();
    SDL_Quit();

    return 0;
}
