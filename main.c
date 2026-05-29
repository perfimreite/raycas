#include "constants.h"
#include "map.h"
#include "meta.h"
#include "utils.h"
#include "vector.h"

#include <SDL2/SDL_pixels.h>
#include <SDL2/SDL_rect.h>
#include <stdbool.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

// none transparent colors
global const SDL_Color black      = { .r = 0  , .g = 0  , .b = 0  , .a = 255 };
global const SDL_Color white      = { .r = 255, .g = 255, .b = 255, .a = 255 };
global const SDL_Color gray       = { .r = 127, .g = 127, .b = 127, .a = 255 };
global const SDL_Color light_gray = { .r = 63 , .g = 63 , .b = 63 , .a = 255 };
global const SDL_Color red        = { .r = 255, .g = 0  , .b = 0  , .a = 255 };
global const SDL_Color green      = { .r = 0  , .g = 255, .b = 0  , .a = 255 };
global const SDL_Color blue       = { .r = 0  , .g = 0  , .b = 255, .a = 255 };
global const SDL_Color light_blue = { .r = 191, .g = 191, .b = 255, .a = 255 };
global const SDL_Color magenta    = { .r = 255, .g = 0  , .b = 255, .a = 255 };
global const SDL_Color cyan       = { .r = 0  , .g = 255, .b = 255, .a = 255 };
global const SDL_Color orange     = { .r = 255, .g = 165, .b = 0  , .a = 255 };
global const SDL_Color yellow     = { .r = 255, .g = 255, .b = 0  , .a = 255 };

// transparent colors
global const SDL_Color transparent       = { .r = 0  , .g = 0  , .b = 0  , .a = 0   };
global const SDL_Color black_transparent = { .r = 0  , .g = 0  , .b = 0  , .a = 127 };

typedef struct {
    u32 h;
    u32 w;

    const char *path;
    TTF_Font *font;
} Font;

Font font_16px = { 0 };
Font font_24px = { 0 };
Font font_32px = { 0 };

internal void init_font(Font *font, const char *path, u32 height)
{
    font->path = path;
    font->h = height;
    font->w = height / 2;

    font->font = TTF_OpenFont(path, height);
    if (!font->font) {
        fprintf(stderr, "ERROR: could not load font: %s\n", path);
        exit(1);
    }
}

typedef enum {
    OVERLAY_STATE_HIDDEN,
    OVERLAY_STATE_SHOWN,
    _overlay_state_count,
} Overlay_State;

typedef struct {
    Overlay_State state;
    char msg[128];
    const char *msg_format;
} Overlay;

internal Overlay_State next_overlay_state(Overlay_State state)
{
    return (state + 1) % _overlay_state_count;
}

internal inline void update_overlay_message(Overlay *overlay, f64 dt, V2f player_pos)
{
    u32 fps = 1 / dt;
    sprintf(overlay->msg, overlay->msg_format,
            WINDOW_WIDTH, WINDOW_HEIGHT,
            fps, player_pos.x, player_pos.y);
}

typedef struct {
    V2f pos;
    V2f dir;

    i32 vel;
    i32 rotation_vel;
    f32 fov;
    u32 radius;

    SDL_Color color;
} Player;

typedef enum {
    GAME_STATE_GAME,
    GAME_STATE_MAP,
    GAME_STATE_MENU,
    _game_state_count
} Game_State;

typedef struct {
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Event event;

    SDL_Rect minimap_dims;

    V2f cursor;

    i32 map_index;

    Game_State state;
    bool show_crosshair;
    bool quit;
} Game;

Game game = { 0 };

internal void init_game(Game *game, const char *title)
{
    game->window = SDL_CreateWindow(title,
                                    SDL_WINDOWPOS_CENTERED,
                                    SDL_WINDOWPOS_CENTERED,
                                    WINDOW_WIDTH,
                                    WINDOW_HEIGHT,
                                    0);
    if (!game->window) {
        fprintf(stderr, "ERROR: could not create window\n");
        exit(1);
    }

    // SDL_SetHint(SDL_HINT_RENDER_LINE_METHOD, "3");
    game->renderer = SDL_CreateRenderer(game->window, -1, 0);
    if (!game->renderer) {
        fprintf(stderr, "ERROR: could not create renderer\n");
        exit(1);
    }

    game->minimap_dims = (SDL_Rect) {
        .x = WINDOW_WIDTH * ((f32)3/4),
        .y = 0,
        .h = WINDOW_HEIGHT * ((f32)1/4),
        .w = WINDOW_WIDTH * ((f32)1/4)
    };

    game->cursor = make_v2f(0, 0);

    game->map_index = 0;

    game->state = GAME_STATE_MENU;
    game->show_crosshair = true;
    game->quit = false;
}

internal void game_state_toggle_game()
{
    game.state = GAME_STATE_GAME;
}

internal void game_state_toggle_map()
{
    if (game.state == GAME_STATE_GAME) {
        game.state = GAME_STATE_MAP;
    } else if (game.state == GAME_STATE_MAP) {
        game.state = GAME_STATE_GAME;
    }
}

internal void game_state_toggle_menu()
{
    // TODO: Play sound when entering menu
    if (game.state == GAME_STATE_GAME || game.state == GAME_STATE_MAP) {
        game.state = GAME_STATE_MENU;
    } else {
        game.state = GAME_STATE_GAME;
    }
}

internal void set_draw_color(SDL_Color color)
{
    if (SDL_SetRenderDrawColor(game.renderer, color.r, color.g, color.b, color.a) != 0) {
        fprintf(stderr, "ERROR: could not set render draw color\n");
        exit(1);
    }
}

internal void draw_rect(SDL_Color color, SDL_Rect *rect)
{
    set_draw_color(color);

    if (SDL_RenderFillRect(game.renderer, rect) != 0) exit(1);
    if (SDL_RenderDrawRect(game.renderer, rect) != 0) exit(1);
}

internal void draw_walls(SDL_Color color)
{
    for (u32 y = 0; y < WINDOW_HEIGHT; y += CELL_SIZE) {
        for (u32 x = 0; x < WINDOW_WIDTH; x += CELL_SIZE) {
            if (is_wall(game.map_index, x, y)) {
                SDL_Rect rect = { .x = x, .y = y, .w = CELL_SIZE, .h = CELL_SIZE };
                draw_rect(color, &rect);
            }
        }
    }
}

internal void draw_line(SDL_Color color, V2f a, V2f b, i32 padding)
{
    set_draw_color(color);

    for (i32 i = -padding; i <= padding; i++) {
        if (SDL_RenderDrawLineF(game.renderer, a.x + i, a.y + i, b.x + i, b.y + i) != 0) exit(1);
    }
}

internal void draw_grid(SDL_Color color)
{
    for (u32 y = CELL_SIZE; y < WINDOW_HEIGHT; y += CELL_SIZE) {
        draw_line(color, make_v2f(0, y), make_v2f(WINDOW_WIDTH, y), 0);
    }

    for (u32 x = CELL_SIZE; x < WINDOW_WIDTH; x += CELL_SIZE) {
        draw_line(color, make_v2f(x, 0), make_v2f(x, WINDOW_HEIGHT), 0);
    }
}

internal void draw_circle(SDL_Color color, V2f center, i32 radius, bool filled)
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
            draw_line(color, make_v2f(center.x + x, center.y + y), make_v2f(center.x - x, center.y + y), 0);
            draw_line(color, make_v2f(center.x + x, center.y - y), make_v2f(center.x - x, center.y - y), 0);
            draw_line(color, make_v2f(center.x + y, center.y + x), make_v2f(center.x - y, center.y + x), 0);
            draw_line(color, make_v2f(center.x + y, center.y - x), make_v2f(center.x - y, center.y - x), 0);
        }  else {
            SDL_RenderDrawPoint(game.renderer, center.x + x, center.y + y);
            SDL_RenderDrawPoint(game.renderer, center.x - x, center.y + y);
            SDL_RenderDrawPoint(game.renderer, center.x + x, center.y - y);
            SDL_RenderDrawPoint(game.renderer, center.x - x, center.y - y);
            SDL_RenderDrawPoint(game.renderer, center.x + y, center.y + x);
            SDL_RenderDrawPoint(game.renderer, center.x - y, center.y + x);
            SDL_RenderDrawPoint(game.renderer, center.x + y, center.y - x);
            SDL_RenderDrawPoint(game.renderer, center.x - y, center.y - x);
        }
    }
}

typedef struct {
    V2f pos;
    f64 perp_wall_dist;
    bool vertical;
    bool perim;
} Intersect;

internal Intersect find_intersect(V2f pos, V2f dir)
{
    V2f delta_dist = make_v2f(0, 0);
    V2f step       = make_v2f(0, 0);
    V2f side_dist  = make_v2f(0, 0);
    V2f map        = make_v2f(0, 0);

    delta_dist.x = dir.x == 0 ? 1e30 : fabs(1.0f / dir.x);
    delta_dist.y = dir.y == 0 ? 1e30 : fabs(1.0f / dir.y);

    V2f tile_relative_pos = v2f_scale(pos, 1 / CELL_SIZE);
    map.x = floor(tile_relative_pos.x);
    map.y = floor(tile_relative_pos.y);

    if (dir.x < 0) {
        step.x = -1;
        side_dist.x = (tile_relative_pos.x - map.x) * delta_dist.x;
    } else {
        step.x = 1;
        side_dist.x = (map.x + 1.0f - tile_relative_pos.x) * delta_dist.x;
    }

    if (dir.y < 0) {
        step.y = -1;
        side_dist.y = (tile_relative_pos.y - map.y) * delta_dist.y;
    } else {
        step.y = 1;
        side_dist.y = (map.y + 1.0f - tile_relative_pos.y) * delta_dist.y;
    }

    Intersect intersect = { 0 };

    for (;;) {
        if (side_dist.x < side_dist.y) {
            side_dist.x += delta_dist.x;
            map.x += step.x;
            intersect.vertical = false;
        } else {
            side_dist.y += delta_dist.y;
            map.y += step.y;
            intersect.vertical = true;
        }

        if (is_perim_tile(map.x, map.y)) {
            intersect.perim = true;
            break;
        }
        if (is_wall_tile(game.map_index, map.x, map.y)) {
            break;
        }
    }

    intersect.perp_wall_dist =
        (intersect.vertical ? (side_dist.y - delta_dist.y) : (side_dist.x - delta_dist.x)) * CELL_SIZE;
    intersect.pos = v2f_add(pos, v2f_scale(dir, intersect.perp_wall_dist));
    return intersect;
}

internal void draw_crosshair(void)
{
    i32 l = 5;
    draw_line(white, make_v2f(WINDOW_CENTER_X, WINDOW_CENTER_Y - l), make_v2f(WINDOW_CENTER_X, WINDOW_CENTER_Y + l), 0);
    draw_line(white, make_v2f(WINDOW_CENTER_X - l, WINDOW_CENTER_Y), make_v2f(WINDOW_CENTER_X + l, WINDOW_CENTER_Y), 0);
}

internal void draw_3d_view(Player *player)
{
    // TODO: surely we can shorten this and improve performance

    f32 angle_curr  = -player->fov / 2.0f;
    f32 angle_end   =  player->fov / 2.0f;
    f32 angle_step  =  player->fov / (WINDOW_WIDTH - 1);
    for (u32 buffer_x; angle_curr <= angle_end; angle_curr += angle_step, buffer_x++) {
        V2f curr_dir = v2f_rotate(player->dir, angle_curr);
        Intersect intersect = find_intersect(player->pos, curr_dir);

        f32 wall_height = WINDOW_HEIGHT * WALL_HEIGHT_MULTIPLIER / intersect.perp_wall_dist;

        f32 wall_top    = CLAMP((-wall_height / 2.0f) + (WINDOW_HEIGHT / 2.0f), 0.0f, WINDOW_HEIGHT - 1.0f);
        f32 wall_bottom = CLAMP(( wall_height / 2.0f) + (WINDOW_HEIGHT / 2.0f), 0.0f, WINDOW_HEIGHT - 1.0f);

        SDL_Color wall_color = intersect.perim ? black : (intersect.vertical ? gray : light_gray);

        V2f window_start = make_v2f(buffer_x, 0);
        V2f wall_start   = make_v2f(buffer_x, wall_top);
        V2f wall_end     = make_v2f(buffer_x, wall_bottom);
        // V2f window_end   = make_v2f(buffer_x, WINDOW_HEIGHT);

        draw_line(light_blue, window_start, wall_start, 0);
        draw_line(wall_color, wall_start,   wall_end,   0);
        // NOTE: Not drawing these lines saves ~100 FPS
        // draw_line(green,      wall_end,     window_end, 0);
    }

    if (game.show_crosshair) {
        draw_crosshair();
    }
}

internal void draw_player(Player *player)
{
    draw_circle(player->color, player->pos, player->radius, true);
}

internal void draw_player_fov(SDL_Color color, Player *player, u32 beam_spread)
{
    f32 angle_curr  = -player->fov / 2.0f;
    f32 angle_end   =  player->fov / 2.0f;
    f32 angle_step  =  player->fov / (WINDOW_WIDTH - 1) * beam_spread;
    for (u32 buffer_x; angle_curr <= angle_end; angle_curr += angle_step, buffer_x++) {
        V2f curr_dir = v2f_rotate(player->dir, angle_curr);
        Intersect intersect = find_intersect(player->pos, curr_dir);

        draw_line(color, player->pos, intersect.pos, 0);
    }
}

internal void draw_map(Player *player)
{
    draw_walls(gray);
    draw_grid(black);

    draw_player(player);
    draw_player_fov(orange, player, 8);
}

internal void minimap_draw_walls(SDL_Color color, f32 scale)
{
    for (u32 y = 0; y < WINDOW_HEIGHT; y += CELL_SIZE) {
        for (u32 x = 0; x < WINDOW_WIDTH; x += CELL_SIZE) {
            if (is_wall(game.map_index, x, y)) {
                SDL_Rect rect = {
                    .x = x * scale + game.minimap_dims.x,
                    .y = y * scale + game.minimap_dims.y,
                    .w = CELL_SIZE * scale,
                    .h = CELL_SIZE * scale
                };
                draw_rect(color, &rect);
            }
        }
    }
}

internal void minimap_draw_grid(SDL_Color color, f32 scale)
{
    f32 scaled_cell_size = CELL_SIZE * scale;

    for (i32 y = game.minimap_dims.y + scaled_cell_size; y < game.minimap_dims.y + game.minimap_dims.h; y += scaled_cell_size) {
        draw_line(color, make_v2f(game.minimap_dims.x, y), make_v2f(game.minimap_dims.x + game.minimap_dims.w, y), 0);
    }

    for (i32 x = game.minimap_dims.x + scaled_cell_size ; x < game.minimap_dims.x + game.minimap_dims.w; x += scaled_cell_size) {
        draw_line(color, make_v2f(x, game.minimap_dims.y), make_v2f(x, game.minimap_dims.y + game.minimap_dims.h), 0);
    }
}

internal void draw_minimap(Player *player) {
    f64 scale = (f64)game.minimap_dims.w / WINDOW_WIDTH;

    // check that the horisontal and vertical scale are equal
    {
        f64 scale_vertical = (f64)game.minimap_dims.h / WINDOW_HEIGHT;
        assert(scale == scale_vertical && "Horisontal and vertical scale should be equal in the minipap");
    }

    draw_rect(green, &game.minimap_dims);

    minimap_draw_walls(gray, scale);
    minimap_draw_grid(black, scale);

    V2f player_minimap_pos = v2f_add(v2f_scale(player->pos, scale), make_v2f(game.minimap_dims.x, game.minimap_dims.y));
    draw_circle(blue, player_minimap_pos, 2, true);
}

internal void clear_backbuffer(SDL_Color bg)
{
    set_draw_color(bg);
    SDL_RenderClear(game.renderer);
}

typedef struct {
    const char *text;
    Font font;
    bool pressed;

    SDL_Rect dims;
    SDL_Color bg;
    SDL_Color fg;
    SDL_Color border;
} Button;

internal Button *init_button() {
    return NULL;
}

typedef struct {
    Button *items;
    u32 count;
    u32 capacity;
} Buttons;

internal void buttons_append(Buttons *buttons, Button button) {
    buttons->items[buttons->count++] = button;
    assert(buttons->count <= buttons->capacity);
}

internal bool button_clicked(Button *button, V2f click) {
    return false;
}

internal void draw_button(Button *button) {
    draw_rect(button->bg, &button->dims);
    SDL_Surface *surface = TTF_RenderUTF8_Shaded(button->font.font, button->text, button->fg, button->bg);
    SDL_Texture *texture = SDL_CreateTextureFromSurface(game.renderer, surface);
    const SDL_Rect rect = {
        // TODO: This should be a seperate function. Center text in button rect
        .x = button->dims.x + (button->dims.w - button->font.w * strlen(button->text)) / 2,

        .y = button->dims.y + (button->dims.h - button->font.h) / 2,
        .w = button->font.w * strlen(button->text),
        .h = button->font.h
    };

    SDL_RenderCopy(game.renderer, texture, NULL, &rect);

    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
}

internal void draw_menu(Buttons buttons)
{
    for (u32 i = 0; i < buttons.count; i++) {
        draw_button(&buttons.items[i]);
    }
}

internal void draw_game(Player *player, Overlay *overlay, Buttons buttons)
{
    switch (game.state) {
        default: {} break;
        case GAME_STATE_GAME: {
            clear_backbuffer(green);
            draw_3d_view(player);
            draw_minimap(player);
        } break;
        case GAME_STATE_MAP: {
            clear_backbuffer(green);
            draw_map(player);
        } break;
        case GAME_STATE_MENU: {
            clear_backbuffer(white);
            draw_menu(buttons);
        } break;
        case _game_state_count: {} break;
    }

    switch (overlay->state) {
       default : {} break;
       case OVERLAY_STATE_HIDDEN: {} break;
       case OVERLAY_STATE_SHOWN: {
           // NOTE: can change to wrapped font rendering if msg gets too long
           // WARNING: this code block cannot be factored out into a seperate function.
           // doing so will cause draw_3d_view() to not draw anything *unless* a print statement
           // including both a character and newline appear above the call to draw_3d_view()
           SDL_Surface *surface = TTF_RenderUTF8_Shaded(font_16px.font, overlay->msg, white, black_transparent);
           SDL_Texture *texture = SDL_CreateTextureFromSurface(game.renderer, surface);
           const SDL_Rect rect = { .x = 0, .y = 0, .w = font_16px.w * strlen(overlay->msg), .h = font_16px.h };

           SDL_RenderCopy(game.renderer, texture, NULL, &rect);

           SDL_FreeSurface(surface);
           SDL_DestroyTexture(texture);
        } break;
        case _overlay_state_count: {} break;
    }

    SDL_RenderPresent(game.renderer);
}

internal inline bool is_key_pressed(i32 key)
{
    return game.event.key.keysym.sym == key;
}

internal inline void player_rotate_clockwise(Player *player, f64 dt)
{
    player->dir = v2f_rotate(player->dir, radians_from_degrees(player->rotation_vel) * dt);
}

internal inline void player_rotate_counterclockwise(Player *player, f64 dt)
{
    player->dir = v2f_rotate(player->dir, radians_from_degrees(-player->rotation_vel) * dt);
}

i32 main(void)
{
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        fprintf(stderr, "ERROR: could not initialize window\n");
        return 1;
    }

    if (TTF_Init() != 0) {
        fprintf(stderr, "ERROR: could not initialize ttf\n");
        return 1;
    }

    init_game(&game, "Raycas");
    init_font(&font_16px, "fonts/CascadiaMono.ttf", 16);
    init_font(&font_24px, "fonts/CascadiaMono.ttf", 24);
    init_font(&font_32px, "fonts/CascadiaMono.ttf", 32);

    Overlay overlay = {
        .state = OVERLAY_STATE_HIDDEN,
        .msg_format = "RESOLUTION:%dx%d FPS:%d POS:(%.2f, %.2f)"
    };
    Player player = {
        .pos.x = 200,
        .pos.y = 200,
        .dir.x = 1,
        .dir.y = 0,
        .vel = 200,
        .rotation_vel = 120,
        .fov = M_PI_2,
        .radius = 6,
        .color = blue
    };

    Button start_button = {
        .text = "START",
        .font = font_32px,
        .pressed = false,
        .dims = {
            .x = WINDOW_CENTER_X - 100,
            .y = WINDOW_CENTER_Y -  40 - 80,
            .w = 200,
            .h = 80 
        },
        .bg = blue,
        .fg = white,
        .border = black
    };

    Button quit_button = {
        .text = "QUIT",
        .font = font_32px,
        .pressed = false,
        .dims = {
            .x = WINDOW_CENTER_X - 100,
            .y = WINDOW_CENTER_Y -  40 + 80,
            .w = 200,
            .h = 80 
        },
        .bg = red,
        .fg = white,
        .border = black
    };

    Buttons buttons = {
        .items = malloc(32 * sizeof(Button)),
        .count = 0,
        .capacity = 32
    };

    buttons_append(&buttons, start_button);
    buttons_append(&buttons, quit_button);

    f64 frame_start      = 0;
    f64 frame_end        = 0;
    f64 frame_delta_time = 0;

    const u8 *keystate = SDL_GetKeyboardState(NULL);

    while (!game.quit) {
        frame_start = time_in_seconds();

        // handle events
        while (SDL_PollEvent(&game.event)) {
            switch (game.event.type) {
                case SDL_QUIT: {
                    game.quit = true;
                } break;

                case SDL_MOUSEBUTTONDOWN: {
                } break;

                case SDL_KEYDOWN: {
                    // if (is_key_pressed(SDLK_ESCAPE)) {
                    //     game.quit = true;
                    // }
                    if (is_key_pressed(SDLK_o)) {
                        overlay.state = next_overlay_state(overlay.state);
                    } else if (is_key_pressed(SDLK_m)) {
                        game_state_toggle_map();
                    }  else if (is_key_pressed(SDLK_ESCAPE)) {
                        game_state_toggle_menu();
                    } else if (is_key_pressed(SDLK_c)) {
                        game.show_crosshair = !game.show_crosshair;
                    } else if (is_key_pressed(SDLK_n)) {
                        game.map_index = next_map_index(game.map_index);
                    }
                } break;
            }
        }

        // Get cursor positon
        SDL_GetMouseState((i32 *)(&game.cursor.x), (i32 *)(&game.cursor.y));

        if (game.state == GAME_STATE_MENU) {
            if (SDL_GetMouseState((i32 *)&game.cursor.x, (i32 *)&game.cursor.y) & SDL_BUTTON_LMASK) {
                for (i32 i = 0; i < buttons.count; i++) {
                    SDL_FPoint p = {
                        .x = game.cursor.x,
                        .y = game.cursor.y
                    };
                    SDL_FRect r = {
                        .x = buttons.items[i].dims.x,
                        .y = buttons.items[i].dims.y,
                        .w = buttons.items[i].dims.w,
                        .h = buttons.items[i].dims.h
                    };
                    if (SDL_PointInFRect(&p, &r)) {
                        buttons.items[i].pressed = true;
                    }
                }
            }
        }

        if (buttons.items[0].pressed) {
            printf("pressed start");
        }

        // update player position
        if (game.state != GAME_STATE_MENU) {
            V2f old_pos = player.pos;

            if (keystate[SDL_SCANCODE_W]) {
                player.pos = v2f_add(player.pos, v2f_scale(player.dir, frame_delta_time * player.vel));
                if (is_perim(player.pos.x, player.pos.y) ||
                    is_wall(game.map_index, player.pos.x, player.pos.y)) {
                    player.pos = old_pos;
                }
            }
            if (keystate[SDL_SCANCODE_A]) {
                player_rotate_counterclockwise(&player, frame_delta_time);
            }
            if (keystate[SDL_SCANCODE_S]) {
                player.pos = v2f_sub(player.pos, v2f_scale(player.dir, frame_delta_time * player.vel));
                if (is_perim(player.pos.x, player.pos.y) ||
                    is_wall(game.map_index, player.pos.x, player.pos.y)) {
                    player.pos = old_pos;
                }
            }
            if (keystate[SDL_SCANCODE_D]) {
                player_rotate_clockwise(&player, frame_delta_time);
            }
        }

        //update overlay
        if (overlay.state == OVERLAY_STATE_SHOWN) {
            update_overlay_message(&overlay, frame_delta_time, player.pos);
        }

        // rendering
        draw_game(&player, &overlay, buttons);

        // compute delta time
        frame_end = time_in_seconds();
        frame_delta_time = frame_end - frame_start;
    }

    return 0;
}
