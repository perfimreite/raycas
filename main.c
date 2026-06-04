#include "constants.h"
#include "map.h"
#include "meta.h"
#include "utils.h"
#include "vector.h"
#include "game.h"

#include <stdbool.h>

#include <SDL2/SDL_pixels.h>
#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

internal void val_error(i32 code)
{
    if (code < 0) {
        fprintf(stderr, "SDL ERROR: %s\n", SDL_GetError());
        exit(1);
    }
}

internal void *ptr_error(void *code)
{
    if (!code) {
        fprintf(stderr, "SDL ERROR: %s\n", SDL_GetError());
        exit(1);
    }
    return code;
}

typedef struct {
    const char *path;
    i32 ptsize;
    TTF_Font *font;
} Font;

Font *fonts = NULL;
global u32 font_count = 0;

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
    for (u32 i = 0; i < font_count; i++) {
        if (fonts[i].ptsize == ptsize) {
            return &fonts[i];
        }
    }

    fonts[font_count++] = font_init(FONT_FILE, ptsize);
    return &fonts[font_count-1];
}

// global Font font_16px = { 0 };
// global Font font_24px = { 0 };
// global Font font_32px = { 0 };

global SDL_Window *window = NULL;
global SDL_Renderer *renderer = NULL;

internal u32 get_mouse_state(void)
{
    i32 x;
    i32 y;
    u32 mouse_button_state = SDL_GetMouseState(&x, &y);
    // game.mouse.x = x;
    // game.mouse.y = y;
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
    val_error(SDL_SetRenderDrawColor(renderer, sdl_color.r, sdl_color.g, sdl_color.b, sdl_color.a));
    val_error(SDL_RenderDrawPoint(renderer, a.x, a.y));
}

void platform_draw_line(Color color, V2f a, V2f b)
{
    SDL_Color sdl_color = translate_color(color);
    val_error(SDL_SetRenderDrawColor(renderer, sdl_color.r, sdl_color.g, sdl_color.b, sdl_color.a));
    val_error(SDL_RenderDrawLine(renderer, a.x, a.y, b.x, b.y));
}

void platform_draw_rect(Color color, Rect rect)
{
    SDL_Color sdl_color = translate_color(color);
    val_error(SDL_SetRenderDrawColor(renderer, sdl_color.r, sdl_color.g, sdl_color.b, sdl_color.a));
    SDL_Rect sdl_rect = translate_rect(rect);
    val_error(SDL_RenderFillRect(renderer, &sdl_rect));
    val_error(SDL_RenderDrawRect(renderer, &sdl_rect));
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
    val_error(SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a));
    val_error(SDL_RenderClear(renderer));
}

void platform_draw_text(Color fg, Color bg, Rect rect, const char *text, i32 ptsize)
{
    SDL_Color sdl_fg = translate_color(fg);
    SDL_Color sdl_bg = translate_color(bg);

    Font *font = get_font(ptsize);
    SDL_Surface *surface = ptr_error(TTF_RenderUTF8_Shaded(font->font, text, sdl_fg, sdl_bg));
    SDL_Texture *texture = ptr_error(SDL_CreateTextureFromSurface(renderer, surface));

    SDL_Rect sdl_rect = translate_rect(rect);

    val_error(SDL_RenderCopy(renderer, texture, NULL, &sdl_rect));

    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
}

i32 main(void)
{
    val_error(SDL_Init(SDL_INIT_EVERYTHING));
    val_error(TTF_Init());

    window = ptr_error(SDL_CreateWindow("Raycas", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, 0));
    renderer = ptr_error(SDL_CreateRenderer(window, -1, 0));

    fonts = malloc(32 * sizeof(Font));

    // font_init(&font_16px, FONT_FILE, 16);
    // font_init(&font_24px, FONT_FILE, 24);
    // font_init(&font_32px, FONT_FILE, 32);

    game_init("Raycas");

    Overlay overlay = {
        .state = OVERLAY_STATE_HIDDEN,
        .msg_format = "RESOLUTION:%dx%d FPS:%d POS:(%.2f, %.2f)",
        .ptsize = 16
    };

    Player player = player_init();

    Buttons buttons = buttons_init();

    // Buttons buttons = {
    //     .items = malloc(32 * sizeof(Button)),
    //     .count = 0,
    //     .capacity = 32
    // };
    //
    // buttons_append(&buttons, start_button);
    // buttons_append(&buttons, quit_button);

    f64 frame_start      = 0;
    f64 frame_end        = 0;
    f64 frame_delta_time = 0;

    const u8 *keystate = SDL_GetKeyboardState(NULL);

    bool quit = false;
    while (!quit) {
        frame_start = time_in_seconds();

        // handle events
        SDL_Event event = { 0 };
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT: {
                    quit = true;
                } break;

                case SDL_MOUSEBUTTONDOWN: {
                } break;

                // case SDL_KEYDOWN: {
                //     switch (event.key.keysym.sym) {
                //         case SDLK_o: {
                //             overlay.state = next_overlay_state(overlay.state);
                //         } break;
                //         case SDLK_m: {
                //             game_state_toggle_map();
                //         } break;
                //         case SDLK_ESCAPE: {
                //             game_state_toggle_menu();
                //         } break;
                //         case SDLK_c: {
                //             game_toggle_crosshair();
                //         } break;
                //         case SDLK_n: {
                //             game_next_map_index();
                //         } break;
                //     }
                // } break;
            }
        }

        // Get mouse positon
        u32 mouse_button_state = get_mouse_state();

        // if (game.state == GAME_STATE_MENU && mouse_button_state & SDL_BUTTON_LMASK) {
        //     for (u32 i = 0; i < buttons.count; i++) {
        //         SDL_Point p = {
        //             .x = game.mouse.x,
        //             .y = game.mouse.y
        //         };
        //         SDL_Rect r = {
        //             .x = buttons.items[i].rect.x,
        //             .y = buttons.items[i].rect.y,
        //             .w = buttons.items[i].rect.w,
        //             .h = buttons.items[i].rect.h
        //         };
        //         if (SDL_PointInRect(&p, &r)) {
        //             buttons.items[i].pressed = true;
        //         }
        //     }
        // }
        //
        // if (buttons.items[0].pressed) {
        //     game_state_toggle_game();
        // }
        // if (buttons.items[1].pressed) {
        //     quit = true;
        // }
        //
        // // update player position
        // if (game.state != GAME_STATE_MENU) {
        //     V2f old_pos = player.pos;
        //
        //     if (keystate[SDL_SCANCODE_W]) {
        //         player.pos = v2f_add(player.pos, v2f_scale(player.dir, frame_delta_time * player.vel));
        //         if (is_perim(player.pos.x, player.pos.y) ||
        //             is_wall(game.map_index, player.pos.x, player.pos.y)) {
        //             player.pos = old_pos;
        //         }
        //     }
        //     if (keystate[SDL_SCANCODE_A]) {
        //         player_rotate_counterclockwise(&player, frame_delta_time);
        //     }
        //     if (keystate[SDL_SCANCODE_S]) {
        //         player.pos = v2f_sub(player.pos, v2f_scale(player.dir, frame_delta_time * player.vel));
        //         if (is_perim(player.pos.x, player.pos.y) ||
        //             is_wall(game.map_index, player.pos.x, player.pos.y)) {
        //             player.pos = old_pos;
        //         }
        //     }
        //     if (keystate[SDL_SCANCODE_D]) {
        //         player_rotate_clockwise(&player, frame_delta_time);
        //     }
        // }

        //update overlay
        // if (overlay.state == OVERLAY_STATE_SHOWN) {
        //     update_overlay_message(&overlay, frame_delta_time, player.pos);
        // }
        //
        // rendering
        game_render(player, overlay, buttons);

        SDL_RenderPresent(renderer);

        // compute delta time
        frame_end = time_in_seconds();
        frame_delta_time = frame_end - frame_start;
    }

    TTF_Quit();
    SDL_Quit();

    return 0;
}
