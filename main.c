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
global const SDL_Color purple      = { .r = 255, .g = 0  , .b = 255, .a = 255 };
global const SDL_Color cyan        = { .r = 0  , .g = 255, .b = 255, .a = 255 };

// transparent colors
global const SDL_Color black_trans = { .r = 0  , .g = 0  , .b = 0  , .a = 31 };

typedef struct {
    i32 height;
    i32 width;

    const char *path;
    TTF_Font *font;
} Font;

Font font = { 0 };

internal void init_font(Font *font, const char *path, i32 height)
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
    char msg[1024];
    const char *msg_format;
} Overlay;

typedef struct {
    V2f pos;
    V2f dir;
    SDL_Color color;
    f32 velocity;
    i32 radius;
    f32 fov;
} Player;

typedef struct {
    V2f center;
    V2f a;
    V2f b;

    i32 radius;

} Camera;

typedef struct {
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Event event;
	
    bool show_map_fullscreen;
    SDL_Rect minimap_rect;

    bool quit;
    V2 cursor;
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

    game->minimap_rect.x = WINDOW_WIDTH - 160;
    game->minimap_rect.y = 0;
    game->minimap_rect.w = 160;
    game->minimap_rect.h = 120;
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
        draw_line(color, (V2f){ .x = 0, .y = y }, (V2f){ .x = WINDOW_WIDTH, .y = y }, 0);
    }

    for (u32 x = CELL_SIZE; x < WINDOW_WIDTH; x += CELL_SIZE) {
        draw_line(color, (V2f){ .x = x, .y = 0 }, (V2f){ .x = x, .y = WINDOW_HEIGHT }, 0);
    }
}

internal void draw_circle(SDL_Color color, V2f a, u32 radius)
{
    f32 acc = radius * 2;
    f32 radius_sq = radius * radius;

    for (i32 y = a.y - radius; y <= a.y + radius; y++) {
        for (i32 x = a.x - radius; x <= a.x + radius; x++) {
            V2f u = { .x = x, .y = y };
            i32 l =  v2f_square_len(v2f_sub(u, a));

            if (radius_sq - acc <= l && l <= radius_sq + acc) {
                draw_line(color, a, u, 0);
            }
        }
    }
}

typedef struct {
    V2 pos;
    bool vertical;
} Intersect;

internal Intersect find_intersect(V2f a, V2f b)
{
    V2f u = v2f_sub(b, a);
    f32 l = v2f_len(u);

    for (i32 i = 0;; i++) {
        f32 t = i / l;
        i32 x = a.x + t * u.x;
        i32 y = a.y + t * u.y;

        V2 pos = { .x = x, .y = y };
        Intersect intersect = { .pos = pos, .vertical = false };

        // check for oob
        if (0 > x || x >= WINDOW_WIDTH) {
            intersect.vertical = true;
            return intersect;
        }
        if (0 > y || y >= WINDOW_HEIGHT) {
            return intersect;
        }

        if (x % CELL_SIZE == 0 && (is_wall(x, y) || (u.x < 0 && is_wall(x - 1, y)))) {
            intersect.vertical = true;
            return intersect;
        }
        if (y % CELL_SIZE == 0 && (is_wall(x, y) || (u.y < 0 && is_wall(x, y - 1)))) {
            return intersect;
        }
    }
}

internal void draw_intersect(SDL_Color color, V2f a, V2f b)
{
    Intersect intersect = find_intersect(a, b);
    u32 radius = 6;
    draw_circle(color, v2_to_v2f(intersect.pos), radius);
}

internal inline void update_overlay_message(Overlay *overlay, f64 begin, f64 now)
{
    f64 dt = now - begin;
    u32 fps = 1 / dt;
    sprintf(overlay->msg, overlay->msg_format, WINDOW_WIDTH, WINDOW_HEIGHT, fps);
}

internal inline bool is_key_pressed(i32 key)
{
    return game.event.key.keysym.sym == key;
}

internal void draw_3d_buffer(Player player, Camera camera)
{
    f32 theta = player.fov;
    f32 half_theta = theta / 2.0f;
    f32 step = theta / (WINDOW_WIDTH - 1);
    
    // line equation: ax+by+c
    f32 a = camera.a.y - camera.b.y;
    f32 b = camera.b.x - camera.a.x;
    f32 c = camera.a.x * camera.b.y - camera.b.x * camera.a.y;

    f32 angle = -half_theta;
    u32 wall_x = 0;
    for (; angle <= half_theta; angle += step, wall_x++) {
        f32 c_angle = cosf(angle);
        f32 s_angle = sinf(angle);

        f32 x = player.pos.x + (player.dir.x * c_angle - player.dir.y * s_angle);
        f32 y = player.pos.y + (player.dir.x * s_angle + player.dir.y * c_angle);
        V2f u = { .x = x, .y = y };
        Intersect intersect = find_intersect(player.pos, u);

        f32 perp_wall_dist = distance_point_to_line(v2_to_v2f(intersect.pos), a, b, c);;
        u32 wall_height_multiplier = 32; 
        i32 wall_height = WINDOW_HEIGHT * wall_height_multiplier / perp_wall_dist;

        i32 wall_top    = CLAMP((-wall_height / 2) + (WINDOW_HEIGHT / 2), 0, WINDOW_HEIGHT - 1);
        i32 wall_bottom = CLAMP(( wall_height / 2) + (WINDOW_HEIGHT / 2), 0, WINDOW_HEIGHT - 1);
        
        SDL_Color color = intersect.vertical ? gray : light_gray;

        V2f wall_start = { .x = wall_x, wall_top };
        V2f wall_end = { .x = wall_x, wall_bottom };

        draw_line(color, wall_start, wall_end, 0);
    }
}

internal void draw_player_fov(SDL_Color color, Player player, Camera camera, u32 beam_spread)
{
    f32 theta = player.fov;
    f32 half_theta = theta / 2.0f;
    f32 step = theta / (WINDOW_WIDTH - 1);
    
    f32 a = camera.a.y - camera.b.y;
    f32 b = camera.b.x - camera.a.x;
    f32 c = camera.a.x * camera.b.y - camera.b.x * camera.a.y;

    f32 angle = -half_theta;
    u32 wall_x = 0;
    for (; angle <= half_theta; angle += step, wall_x++) {
        if (wall_x % beam_spread == 0) {
            f32 c_angle = cosf(angle);
            f32 s_angle = sinf(angle);

            f32 x = player.pos.x + (player.dir.x * c_angle - player.dir.y * s_angle);
            f32 y = player.pos.y + (player.dir.x * s_angle + player.dir.y * c_angle);
            V2f u = { .x = x, .y = y };
            Intersect intersect = find_intersect(player.pos, u);

            // draw_line(color, player.pos, v2_to_v2f(intersect.pos), 0);

            f32 perp_wall_dist = distance_point_to_line(v2_to_v2f(intersect.pos), a, b, c);
            draw_line(color, v2_to_v2f(intersect.pos), v2f_sub(v2_to_v2f(intersect.pos), v2f_scale(player.dir, perp_wall_dist)), 0);
            if (wall_x == 0 || wall_x == WINDOW_WIDTH - 1) {
                draw_line(purple, player.pos, v2_to_v2f(intersect.pos), 1);
            }
        }
    }
}

internal void draw_map(Player player, Camera camera)
{
    // game.show_map_fullscreen
    if (true) {
        draw_walls(gray);
        draw_grid(black);

        // draw player and intersects with grid
        draw_circle(player.color, player.pos, player.radius);

        draw_intersect(green, player.pos, v2f_add(player.pos, player.dir));

        // draw camera
        draw_circle(black, camera.center, camera.radius);
        draw_line(purple, player.pos, camera.center, 1);
        draw_line(blue, camera.a, camera.b, 1);
        draw_line(red, player.pos, camera.a, 1);
        draw_line(red, player.pos, camera.b, 1);
        draw_player_fov(light_gray, player, camera, 1);
	} else {
        draw_walls(gray);
        draw_grid(black);

        // draw player and intersects with grid
        f32 scale = 0;
        V2f player_pos = v2f_scale(player.pos, scale);
        player_pos = v2f_add(player.pos, (V2f){ .x = game.minimap_rect.x, .y = game.minimap_rect.y });

        V2f player_dir = v2f_scale(player.dir, scale);
        player_dir = v2f_add(player.dir, (V2f){ .x = game.minimap_rect.x, .y = game.minimap_rect.y });

        draw_circle(player.color, player.pos, player.radius);

        draw_intersect(green, player.pos, v2f_add(player.pos, player.dir));

        // draw camera
        draw_circle(black, camera.center, camera.radius);
        draw_line(purple, player.pos, camera.center, 1);
        draw_line(blue, camera.a, camera.b, 1);
        draw_line(red, player.pos, camera.a, 1);
        draw_line(red, player.pos, camera.b, 1);
        draw_player_fov(light_gray, player, camera, 1);
    }
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

    Overlay overlay = { .state = OVERLAY_STATE_HIDDEN, .msg_format = "RESOLUTION: %dx%d, FPS: %d" };

    Player player = { .pos.x = 200, .pos.y = 200, .dir.x = 1, .dir.y = 0, .color = blue, .velocity = 1.0f, .radius = 6, .fov = M_PI / 2.0};
    Camera camera = { .radius = 6 };
    const u8 *keystate = SDL_GetKeyboardState(NULL);

    while (!game.quit) {
        f64 begin = time_in_seconds();

        while (SDL_PollEvent(&game.event)) {
            switch (game.event.type) {
                case SDL_QUIT: {
                    game.quit = true;
                } break;

                case SDL_KEYDOWN: {
                    if (is_key_pressed(SDLK_ESCAPE)) {
                        game.quit = true;
                    }
                    else if (is_key_pressed(SDLK_o)) {
                        overlay.state = (overlay.state + 1) % _overlay_state_count;
                    }
                    else if (is_key_pressed(SDLK_m)) {
                        game.show_map_fullscreen = !game.show_map_fullscreen;
                    }
                } break;
            }
        }

        if (keystate[SDL_SCANCODE_W]) {
            player.pos = v2f_add(player.pos, player.dir);
        }
        if (keystate[SDL_SCANCODE_A]) {
            player.dir = v2f_rotate(player.dir, degrees_to_radians(-0.5f));
        }
        if (keystate[SDL_SCANCODE_S]) {
            player.pos = v2f_sub(player.pos, player.dir);
        }
        if (keystate[SDL_SCANCODE_D]) {
            player.dir = v2f_rotate(player.dir, degrees_to_radians(0.5f));
        }

        // get mouse state
        // SDL_GetMouseState(&game.cursor.x, &game.cursor.y);

        // compute camera coordinates
        {
            V2f dir = v2f_scale(player.dir, CELL_SIZE);

            camera.center = v2f_add(player.pos, dir);
            camera.a = v2f_add(camera.center, v2f_scale(v2f_normal(dir), -1));
            camera.b = v2f_add(camera.center, v2f_normal(dir));
        }

        set_draw_color(white);
        SDL_RenderClear(game.renderer);

        if (game.show_map_fullscreen) {
            draw_map(player, camera);
        }
        else {
            draw_3d_buffer(player, camera);
            // draw_map(player, camera);
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

        // draw to window
        SDL_RenderPresent(game.renderer);

        f64 now = time_in_seconds();

        update_overlay_message(&overlay, begin, now);
    }

    return 0;
}

// TODO: find new formula for drawing line 
// TODO: find new formula for drawing circle
