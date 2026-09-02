#include "sdl_main.h"
#include "utils.h"
#include "vector.h"
#include "game.h"

global Fonts fonts = {0};

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
    LIST_FOR_EACH(Font, fonts, font) {
        if (font->ptsize == ptsize) {
            return font;
        }
    }

    LIST_PUSH(fonts, font_init(FONT_FILE, ptsize));
    return &LIST_LAST(fonts);
}

global SDL_Window *window = NULL;
global SDL_Renderer *renderer = NULL;

internal void platform_val_err(i32 code)
{
    if (code < 0) {
        fprintf(stderr, "SDL ERROR: %s\n", SDL_GetError());
        exit(1);
    }
}

internal void *platform_ptr_err(void *code)
{
    if (!code) {
        fprintf(stderr, "SDL ERROR: %s\n", SDL_GetError());
        exit(1);
    }
    return code;
}

void platform_update_mouse_state(Mouse_State *mouse_state)
{
    i32 x, y;
    u32 mouse_button_state = SDL_GetMouseState(&x, &y);

    mouse_state->pos.x = x;
    mouse_state->pos.y = y;

    mouse_state->key_left.is_down   = mouse_button_state & SDL_BUTTON_LMASK;
    mouse_state->key_middel.is_down = mouse_button_state & SDL_BUTTON_MMASK;
    mouse_state->key_right.is_down  = mouse_button_state & SDL_BUTTON_RMASK;
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
    platform_val_err(SDL_SetRenderDrawColor(renderer, sdl_color.r, sdl_color.g, sdl_color.b, sdl_color.a));
    platform_val_err(SDL_RenderDrawPoint(renderer, a.x, a.y));
}

void platform_draw_line(Color color, V2f a, V2f b)
{
    SDL_Color sdl_color = translate_color(color);
    platform_val_err(SDL_SetRenderDrawColor(renderer, sdl_color.r, sdl_color.g, sdl_color.b, sdl_color.a));
    platform_val_err(SDL_RenderDrawLine(renderer, a.x, a.y, b.x, b.y));
}

void platform_draw_rect(Color color, Rect rect)
{
    SDL_Color sdl_color = translate_color(color);
    platform_val_err(SDL_SetRenderDrawColor(renderer, sdl_color.r, sdl_color.g, sdl_color.b, sdl_color.a));
    SDL_Rect sdl_rect = translate_rect(rect);
    platform_val_err(SDL_RenderFillRect(renderer, &sdl_rect));
    platform_val_err(SDL_RenderDrawRect(renderer, &sdl_rect));
}

void platform_draw_circle(Color color, V2f center, i32 radius, b32 filled)
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

void platform_draw_rect_rounded(Color color, Rect rect, i32 radius)
{
    Rect i_rect = make_rect(rect.x + radius, rect.y + radius, rect.w - 2*radius, rect.h - 2*radius);
    Rect t_rect = make_rect(rect.x + radius, rect.y, rect.w - 2*radius, radius);
    Rect b_rect = make_rect(rect.x + radius, rect.y + rect.h - radius, rect.w - 2*radius, radius);
    Rect l_rect = make_rect(rect.x, rect.y + radius, radius, rect.h - 2*radius);
    Rect r_rect = make_rect(rect.x + rect.w - radius, rect.y + radius, radius, rect.h - 2*radius);

    platform_draw_rect(color, i_rect);
    platform_draw_rect(color, t_rect);
    platform_draw_rect(color, b_rect);
    platform_draw_rect(color, l_rect);
    platform_draw_rect(color, r_rect);

    platform_draw_circle(color, make_v2f(i_rect.x, i_rect.y), radius, true);
    platform_draw_circle(color, make_v2f(i_rect.x + i_rect.w - 1, i_rect.y), radius, true);
    platform_draw_circle(color, make_v2f(i_rect.x, i_rect.y + i_rect.h - 1), radius, true);
    platform_draw_circle(color, make_v2f(i_rect.x + i_rect.w - 1, i_rect.y + i_rect.h - 1), radius, true);
}

void platform_clear_backbuffer(Color color)
{
    platform_val_err(SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a));
    platform_val_err(SDL_RenderClear(renderer));
}

V2f platform_get_text_dims(i32 ptsize, const char *text)
{
    Font *font = get_font(ptsize);
    i32 w, h;
    platform_val_err(TTF_SizeUTF8(font->font, text, &w, &h));
    V2f dims = make_v2f(w, h);
    return dims;
}

Rect platform_center_text_in_rect(Rect rect, i32 ptsize, const char *text)
{
    V2f text_dims = platform_get_text_dims(ptsize, text);

    return (Rect) {
        .x = rect.x + (rect.w - text_dims.x) / 2,
        .y = rect.y + (rect.h - text_dims.y) / 2,
        .w = text_dims.x,
        .h = text_dims.y
    };
}

void platform_draw_text(Color fg, Color bg, Rect rect, const char *text, i32 ptsize)
{
    SDL_Color sdl_fg = translate_color(fg);
    SDL_Color sdl_bg = translate_color(bg);

    Font *font = get_font(ptsize);

    SDL_Surface *surface = platform_ptr_err(TTF_RenderUTF8_Shaded(font->font, text, sdl_fg, sdl_bg));
    SDL_Texture *texture = platform_ptr_err(SDL_CreateTextureFromSurface(renderer, surface));

    SDL_Rect sdl_rect = translate_rect(rect);

    platform_val_err(SDL_RenderCopy(renderer, texture, NULL, &sdl_rect));

    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
}

b32 platform_point_in_rect(V2f point, Rect rect)
{
    SDL_Point sdl_point = translate_point(point);
    SDL_Rect sdl_rect = translate_rect(rect);
    return SDL_PointInRect(&sdl_point, &sdl_rect);
}

internal void platform_process_key(Key *key, b32 repeat)
{
    key->was_down = repeat;
    key->is_down = true;
}

global SDL_Cursor *cursor_arrow = NULL;
global SDL_Cursor *cursor_hand = NULL;

void platform_set_cursor(Cursor_Kind cursor)
{
    switch (cursor) {
        default: {} break;

        case CURSOR_KIND_ARROW: {
            SDL_SetCursor(cursor_arrow);
        } break;

        case CURSOR_KIND_HAND: {
            SDL_SetCursor(cursor_hand);
        } break;
    }
}

extern Game game;

i32 main(void)
{
    platform_val_err(SDL_Init(SDL_INIT_EVERYTHING));
    platform_val_err(TTF_Init());

    window = platform_ptr_err(SDL_CreateWindow("Raycas", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                              WINDOW_WIDTH, WINDOW_HEIGHT, 0));
    renderer = platform_ptr_err(SDL_CreateRenderer(window, -1, 0));
    SDL_Event event = {0};
    const u8 *keystate = NULL;

    overlay_init();
    player_init();
    game_init();

    Frame_Time frame_time = {0};

    cursor_arrow = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW);
    cursor_hand = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_HAND);

    while (!game.quit) {
        frame_time.start = time_in_seconds();

        // reset keyboard keys
        for (u64 i = 0; i < ARRAY_COUNT(game.keyboard_state.v); i++) {
            Key *key = &game.keyboard_state.v[i];

            key->was_down = key->is_down;
            key->is_down = false;
            key->active = false;
        }

        // handle events
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                default: {} break;

                case SDL_QUIT: {
                    game.quit = true;
                } break;

                case SDL_KEYDOWN: {
                    SDL_KeyboardEvent key = event.key;

                    switch (key.keysym.sym) {
                        default: {} break;
                        case SDLK_o: {
                            platform_process_key(&game.keyboard_state.key_o, key.repeat);
                        } break;
                        case SDLK_m: {
                            platform_process_key(&game.keyboard_state.key_m, key.repeat);
                        } break;
                        case SDLK_ESCAPE: {
                            platform_process_key(&game.keyboard_state.key_escape, key.repeat);
                        } break;
                        case SDLK_c: {
                            platform_process_key(&game.keyboard_state.key_c, key.repeat);
                        } break;
                        case SDLK_n: {
                            platform_process_key(&game.keyboard_state.key_n, key.repeat);
                        } break;
                    }
                } break;
            }
        }

        keystate = SDL_GetKeyboardState(NULL);

        // update player position
        if (keystate[SDL_SCANCODE_W]) {
            game.keyboard_state.key_w.active = true;
        }
        if (keystate[SDL_SCANCODE_A]) {
            game.keyboard_state.key_a.active = true;
        }
        if (keystate[SDL_SCANCODE_S]) {
            game.keyboard_state.key_s.active = true;
        }
        if (keystate[SDL_SCANCODE_D]) {
            game.keyboard_state.key_d.active = true;
        }

        platform_update_mouse_state(&game.mouse_state);

        // rendering
        game_render(frame_time.dt);

        SDL_RenderPresent(renderer);

        // compute delta time
        frame_time.end = time_in_seconds();
        frame_time.dt = frame_time.end - frame_time.start;
    }

    TTF_Quit();
    SDL_Quit();

    return 0;
}

