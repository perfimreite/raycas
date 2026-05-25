#include "constants.h"
#include "map.h"
#include "meta.h"
#include "utils.h"
#include "vector.h"

#include <stdbool.h>
#include <stdio.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_error.h>
#include <SDL2/SDL_mouse.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_ttf.h>

// none transparent colors
global const SDL_Color black       = { .r = 0  , .g = 0  , .b = 0  , .a = 255 };
global const SDL_Color white       = { .r = 255, .g = 255, .b = 255, .a = 255 };
global const SDL_Color gray        = { .r = 127, .g = 127, .b = 127, .a = 255 };
global const SDL_Color light_gray  = { .r = 63 , .g = 63 , .b = 63 , .a = 255 };
global const SDL_Color red         = { .r = 255, .g = 0  , .b = 0  , .a = 255 };
global const SDL_Color green       = { .r = 0  , .g = 255, .b = 0  , .a = 255 };
global const SDL_Color blue        = { .r = 0  , .g = 0  , .b = 255, .a = 255 };
global const SDL_Color light_blue  = { .r = 191, .g = 191, .b = 255, .a = 255 };
global const SDL_Color purple      = { .r = 255, .g = 0  , .b = 255, .a = 255 };
global const SDL_Color cyan        = { .r = 0  , .g = 255, .b = 255, .a = 255 };
global const SDL_Color orange      = { .r = 255, .g = 165, .b = 0  , .a = 255 };
global const SDL_Color yellow      = { .r = 255, .g = 255, .b = 0  , .a = 255 };

// transparent colors
global const SDL_Color black_trans = { .r = 0  , .g = 0  , .b = 0  , .a = 31 };

typedef struct {
    u32 height;
    u32 width;

    const char *path;
    TTF_Font *font;
} Font;

Font font = { 0 };

internal void init_font(Font *font, const char *path, u32 height)
{
    font->path = path;
    font->height = height;
    font->width = height / 2;

    font->font = TTF_OpenFont(path, height);
    if (!font->font) {
        fprintf(stderr, "ERROR: could not load font\n");
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

typedef struct {
    V2f pos;
    V2f dir;

    i32 vel;
    i32 rotation_vel;
    f32 fov;
    u32 radius;

    SDL_Color color;
} Player;

typedef struct {
    V2f center;
    V2f a;
    V2f b;

    u32 radius;
} Camera;

typedef struct {
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Event event;

    bool show_map;
    bool quit;
} Game;

Game game = { 0 };

internal void init_game(Game *game)
{
    game->window = SDL_CreateWindow("Raycas",
                                    SDL_WINDOWPOS_CENTERED,
                                    SDL_WINDOWPOS_CENTERED,
                                    WINDOW_WIDTH,
                                    WINDOW_HEIGHT,
                                    0);
    if (!game->window) {
        fprintf(stderr, "ERROR: could not create window\n");
        exit(1);
    }

    game->renderer = SDL_CreateRenderer(game->window, -1, 0);
    if (!game->renderer) {
        fprintf(stderr, "ERROR: could not create renderer\n");
        exit(1);
    }
}

internal void set_draw_color(SDL_Color color)
{
    if (SDL_SetRenderDrawColor(game.renderer, color.r, color.g, color.b, color.a) != 0) {
        fprintf(stderr, "ERROR: could not set render draw color\n");
        exit(1);
    }
}

internal void draw_walls(SDL_Color color)
{
    set_draw_color(color);

    for (u32 y = 0; y < WINDOW_HEIGHT; y += CELL_SIZE) {
        for (u32 x = 0; x < WINDOW_WIDTH; x += CELL_SIZE) {
            if (get_map_square(x, y) == WALL) {
                SDL_Rect rect = { .x = x, .y = y, .h = CELL_SIZE, .w = CELL_SIZE };
                if (SDL_RenderFillRect(game.renderer, &rect) != 0) exit(1);
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

internal void draw_circle(SDL_Color color, V2f a, u32 radius)
{
    f32 acc = radius * 2.0f;
    f32 radius_sq = radius * radius;

    for (i32 y = a.y - radius; y <= a.y + radius; y++) {
        for (i32 x = a.x - radius; x <= a.x + radius; x++) {
            V2f u = make_v2f(x, y);
            i32 l =  v2f_square_len(v2f_sub(u, a));

            if (radius_sq - acc <= l && l <= radius_sq + acc) {
                draw_line(color, a, u, 0);
            }
        }
    }
}

internal void draw_player(Player *player)
{
    draw_circle(player->color, player->pos, player->radius);
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
    V2f map        = make_v2f(0, 0);
    V2f step       = make_v2f(0, 0);
    V2f hit        = make_v2f(0, 0);

    delta_dist.x = dir.x == 0.0f ? 1e30 : fabs(1.0f / dir.x);
    delta_dist.y = dir.y == 0.0f ? 1e30 : fabs(1.0f / dir.y);

    map.x = floor(pos.x / CELL_SIZE);
    map.y = floor(pos.y / CELL_SIZE);

    if (dir.x > 0) {
        step.x = 1;
        hit.x = (map.x + 1.0f - pos.x) * delta_dist.x;
    } else {
        step.x = -1;
        hit.x = (pos.x - map.x) * delta_dist.x;
    }

    if (dir.y > 0) {
        step.y = 1;
        hit.y = (map.y + 1.0f - pos.y) * delta_dist.y;
    } else {
        step.y = -1;
        hit.y = (pos.y - map.y) * delta_dist.y;
    }

    // printf("%.2f, %.2f\n", hit.x, hit.y);

    for (; !is_wall(hit.x, hit.y) && !is_perim(hit.x, hit.y);) {
        // if (game.show_map) {
        //     draw_circle(red, hit, 2);
        // }

        if (hit.x > hit.y) {
            hit.x += delta_dist.x;
            map.x += step.x;
        } else {
            hit.y += delta_dist.y;
            map.y += step.y;
        }
    }

    Intersect intersect = { 0 };

    if (hit.x > hit.y) {
        intersect.vertical = true;
    }

    if (is_perim(hit.x, hit.y)) {
        intersect.perim = true;
    }

    intersect.pos = v2f_add(pos, hit);
    intersect.perp_wall_dist = intersect.vertical ?  (hit.y - delta_dist.y) : (hit.x - delta_dist.x);

    return intersect;
}

// internal Intersect find_intersect(V2f a, V2f b)
// {
//     V2f u = v2f_sub(b, a);
//     f32 l = v2f_len(u);
//
//     for (i32 i = 0;; i++) {
//         f32 t = i / l;
//         f32 x = a.x + t * u.x;
//         f32 y = a.y + t * u.y;
//
//         V2f pos = make_v2f(x, y);
//         draw_circle(orange, pos, 1);
//         Intersect intersect = { .pos = pos, .vertical = false, .wall = false };
//
//         // check for oob
//         if (0 > x || x >= WINDOW_WIDTH) {
//             intersect.vertical = true;
//             intersect.wall = true;
//             return intersect;
//         }
//         if (0 > y || y >= WINDOW_HEIGHT) {
//             intersect.wall = true;
//             return intersect;
//         }
//
//         if ((i32)x % (i32)CELL_SIZE == 0 && (is_wall(x, y) || (u.x < 0 && is_wall(x - 1, y)))) {
//             intersect.vertical = true;
//             return intersect;
//         }
//         if ((i32)y % (i32)CELL_SIZE == 0 && (is_wall(x, y) || (u.y < 0 && is_wall(x, y - 1)))) {
//             return intersect;
//         }
//     }
// }

internal void draw_intersect(SDL_Color color, V2f a, V2f b)
{
    Intersect intersect = find_intersect(a, b);
    u32 radius = 6;
    draw_circle(color, intersect.pos, radius);
}

internal void draw_3d_view(Player *player, Camera *camera)
{
    f32 theta = player->fov;
    f32 half_theta = theta / 2.0f;
    f32 angle_step = theta / (WINDOW_WIDTH - 1);

    // line equation: ax+by+c
    Line line = line_from_points(camera->a, camera->b);

    f32 angle = -half_theta;
    u32 camera_x = 0;
    for (; angle <= half_theta; angle += angle_step, camera_x++) {
        V2f curr_dir = v2f_rotate(player->dir, angle);
        Intersect intersect = find_intersect(player->pos, curr_dir);

        #define WALL_HEIGHT_MULTIPLIER 64
        f32 perp_wall_dist = distance_point_to_line(intersect.pos, line);
        i32 wall_height = WINDOW_HEIGHT * WALL_HEIGHT_MULTIPLIER / perp_wall_dist;

        i32 wall_top    = CLAMP((-wall_height / 2) + (WINDOW_HEIGHT / 2), 0, WINDOW_HEIGHT - 1);
        i32 wall_bottom = CLAMP(( wall_height / 2) + (WINDOW_HEIGHT / 2), 0, WINDOW_HEIGHT - 1);

        // NOTE: light gray lines in dark gray walls are probably caused by hitting
        // point that is both horisontal and vertical and one being prioritized
        SDL_Color color;
        if (intersect.perim) {
            color = black;
        } else {
            color = intersect.vertical ? gray : light_gray;
        }

        V2f wall_start = make_v2f(camera_x, wall_top);
        V2f wall_end   = make_v2f(camera_x, wall_bottom);

        draw_line(light_blue, make_v2f(camera_x, 0), wall_start, 0);
        draw_line(color,      wall_start, wall_end, 0);
        draw_line(green,      wall_end, make_v2f(camera_x, WINDOW_HEIGHT), 0);
    }
}

internal void draw_player_fov(Player *player, Camera *camera, u32 beam_spread)
{
    f32 theta = player->fov;
    f32 half_theta = theta / 2.0f;
    f32 step = theta / (WINDOW_WIDTH - 1);

    Line line = line_from_points(camera->a, camera->b);

    f32 angle = -half_theta;
    u32 wall_x = 0;
    for (; angle <= half_theta; angle += step, wall_x++) {
        if (wall_x % beam_spread == 0) {
            // V2f u = v2f_add(player->pos, v2f_rotate(player->dir, angle));
            // Intersect intersect = find_intersect(player->pos, u);

            // new find_intersect() takes pos and dir, not pos and pos+dir)
            V2f dir = v2f_rotate(player->dir, angle);
            Intersect intersect = find_intersect(player->pos, dir);
            draw_circle(cyan, intersect.pos, 1);
            f32 perp_wall_dist = distance_point_to_line(intersect.pos, line);

            // NOTE: only for debug
#if 1
            draw_line(orange, player->pos, intersect.pos, 0);
            draw_line(yellow, intersect.pos, v2f_sub(intersect.pos, v2f_scale(player->dir, perp_wall_dist)), 0);
#endif
            if (wall_x == 0 || wall_x == WINDOW_WIDTH - 1) {
                draw_line(purple, player->pos, intersect.pos, 1);
            }
        }
    }
}

internal void draw_map(Player *player, Camera *camera)
{
    draw_walls(gray);
    draw_grid(black);

    // draw player
    draw_player(player);

    // draw camera
    draw_circle(black, camera->center, camera->radius);
    draw_line(purple, player->pos, camera->center, 1);
    draw_line(blue, camera->a, camera->b, 1);
    draw_line(red, player->pos, camera->a, 1);
    draw_line(red, player->pos, camera->b, 1);
    // draw_player_fov(player, camera, 4);

    draw_intersect(blue, player->pos, player->dir);
}

internal inline void update_overlay_message(Overlay *overlay, f64 dt, V2f player_pos)
{
    u32 fps = 1 / dt;
    sprintf(overlay->msg, overlay->msg_format,
            WINDOW_WIDTH, WINDOW_HEIGHT,
            fps, player_pos.x, player_pos.y);
}

internal inline bool is_key_pressed(i32 key)
{
    return game.event.key.keysym.sym == key;
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

    init_game(&game);
    init_font(&font, "fonts/CascadiaMono.ttf", 24);

    Overlay overlay = { .state = OVERLAY_STATE_HIDDEN,
                        .msg_format = "RESOLUTION: %dx%d, FPS: %d, POS: (%.0f, %.0f)" };
    Player player = { .pos.x = 200, .pos.y = 200, .dir.x = 1, .dir.y = 0,
                      .vel = 200, .rotation_vel = 60, .fov = M_PI_2, .radius = 6, .color = blue };
    Camera camera = { .radius = 6 };
    f64 delta_time = 0;

    const u8 *keystate = SDL_GetKeyboardState(NULL);

    while (!game.quit) {
        f64 begin = time_in_seconds();

        // handle events
        while (SDL_PollEvent(&game.event)) {
            switch (game.event.type) {
                case SDL_QUIT: {
                    game.quit = true;
                } break;

                case SDL_KEYDOWN: {
                    if (is_key_pressed(SDLK_ESCAPE)) {
                        game.quit = true;
                    } else if (is_key_pressed(SDLK_o)) {
                        overlay.state = (overlay.state + 1) % _overlay_state_count;
                    } else if (is_key_pressed(SDLK_m)) {
                        game.show_map = !game.show_map;
                    }
                } break;
            }
        }

        // update player position
        {
            V2f old_pos = player.pos;

            if (keystate[SDL_SCANCODE_W]) {
                player.pos = v2f_add(player.pos, v2f_scale(player.dir, delta_time * player.vel));
                if (is_perim(player.pos.x, player.pos.y) ||
                    is_wall(player.pos.x, player.pos.y)) {
                    player.pos = old_pos;
                }
            }
            if (keystate[SDL_SCANCODE_A]) {
                player.dir = v2f_rotate(player.dir, radians_from_degrees(-player.rotation_vel) * delta_time);
            }
            if (keystate[SDL_SCANCODE_S]) {
                player.pos = v2f_sub(player.pos, v2f_scale(player.dir, delta_time * player.vel));
                if (is_perim(player.pos.x, player.pos.y) ||
                    is_wall(player.pos.x, player.pos.y)) {
                    player.pos = old_pos;
                }
            }
            if (keystate[SDL_SCANCODE_D]) {
                player.dir = v2f_rotate(player.dir, radians_from_degrees(player.rotation_vel) * delta_time);
            }
        }

        // compute camera coordinates
        {
            V2f dir = v2f_scale(player.dir, CELL_SIZE);

            camera.center = v2f_add(player.pos, dir);
            camera.a = v2f_add(camera.center, v2f_scale(v2f_normal(dir), -1));
            camera.b = v2f_add(camera.center, v2f_normal(dir));
        }

        // rendering
        set_draw_color(white);
        SDL_RenderClear(game.renderer);

        if (game.show_map) {
            draw_map(&player, &camera);
        } else {
            draw_3d_view(&player, &camera);
        }

        switch (overlay.state) {
            case OVERLAY_STATE_HIDDEN: {

            } break;
            case OVERLAY_STATE_SHOWN: { 
                SDL_Surface *surface = TTF_RenderText_Shaded(font.font, overlay.msg, black, black_trans);
                SDL_Texture *texture = SDL_CreateTextureFromSurface(game.renderer, surface);
                const SDL_Rect rect = { .x = 0, .y = 0, .w = font.width * strlen(overlay.msg), .h = font.height };

                SDL_RenderCopy(game.renderer, texture, NULL, &rect);

                SDL_FreeSurface(surface);
                SDL_DestroyTexture(texture);
            } break;
            case _overlay_state_count: {

            } break;
        }

        SDL_RenderPresent(game.renderer);

        // compute delta time and update overlay
        f64 now = time_in_seconds();
        delta_time = now - begin;

        if (overlay.state == OVERLAY_STATE_SHOWN) {
            update_overlay_message(&overlay, delta_time, player.pos);
        }
    }

    return 0;
}
