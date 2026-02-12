#include "constants.h"
#include "meta.h"
#include "vector.h"

#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <time.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_error.h>
#include <SDL2/SDL_mouse.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_ttf.h>

#define PI 3.14159265358979323846

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

// map
#define EMPTY 0
#define WALL  1

global i32 map[ROWS][COLS] = {
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0},
    {1, 1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 1, 1, 1, 0, 0},
    {0, 1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 1, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0},
    {0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1},
    {0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 1, 0, 0, 0, 1},
    {0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 1},
};

internal i32 get_map_square(i32 x, i32 y)
{
    return map[y/CELL_SIZE][x/CELL_SIZE];
}

internal bool is_wall(i32 x, i32 y)
{
    return get_map_square(x, y) == WALL;
}

typedef struct {
    i32 height;
    i32 width;

    const char *path;
    TTF_Font *font;
} Font;

internal void init_font(Font *font, const char *path, i32 height)
{
    font->path = path;
    font->height = height;
    font->width = height / 2;

    font->font = TTF_OpenFont(path, height);
    if (!font->font) {
        printf("ERROR: could not load font\n");
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
    V2 pos;
    V2 dir;
    SDL_Color color;
    f32 velocity;
    i32 radius;
    f32 fov;
} Player;

typedef struct {
    V2 center;
    V2 a;
    V2 b;

    V2 a_intersect;
    V2 b_intersect;

    i32 radius;
} Camera;

typedef struct {
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Event event;

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
        printf("ERROR: could not create window\n");
        exit(1);
    }

    game->renderer = SDL_CreateRenderer(game->window, -1, 0);
    if (!game->renderer) {
        printf("ERROR: could not create renderer\n");
        exit(1);
    }
}

internal void set_draw_color(SDL_Color color)
{
    if (SDL_SetRenderDrawColor(game.renderer, color.r, color.g, color.b, color.a) != 0) {
        printf("ERROR: could not set render draw color\n");
        exit(1);
    }
}

internal void draw_walls(SDL_Color color)
{
    set_draw_color(color);

    for (u64 y = 0; y < WINDOW_HEIGHT; y += CELL_SIZE) {
        for (u64 x = 0; x < WINDOW_WIDTH; x += CELL_SIZE) {
            if (get_map_square(x, y) == WALL) {
                SDL_Rect rect = { .x = x, .y = y, .h = CELL_SIZE, .w = CELL_SIZE };
                if (SDL_RenderFillRect(game.renderer, &rect) != 0) exit(1);
            }
        }
    }
}

internal void draw_line(SDL_Color color, V2 a, V2 b, i32 padding)
{
    set_draw_color(color);

    for (i32 i = -padding; i <= padding; i++) {
        if (SDL_RenderDrawLine(game.renderer, a.x + i, a.y + i, b.x + i, b.y + i) != 0) exit(1);
    }
}

internal void draw_grid(SDL_Color color)
{
    for (u64 y = CELL_SIZE; y < WINDOW_HEIGHT; y += CELL_SIZE) {
        draw_line(color, (V2){ .x = 0, .y = y }, (V2){ .x = WINDOW_WIDTH, .y = y }, 0);
    }

    for (u64 x = CELL_SIZE; x < WINDOW_WIDTH; x += CELL_SIZE) {
        draw_line(color, (V2){ .x = x, .y = 0 }, (V2){ .x = x, .y = WINDOW_HEIGHT }, 0);
    }
}

internal void draw_circle(SDL_Color color, V2 a, i32 radius)
{
    f32 acc = radius * 2;
    f32 radius_sq = radius * radius;

    for (i32 y = a.y - radius; y <= a.y + radius; y++) {
        for (i32 x = a.x - radius; x <= a.x + radius; x++) {
            V2 u = { .x = x, .y = y };
            i32 l =  v2_square_len(v2_sub(u, a));

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

internal Intersect find_intersect(V2 a, V2 b)
{
    V2 u = v2_sub(b, a);
    f32 l = v2_len(u);

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

internal void draw_intersect(SDL_Color color, V2 a, V2 b)
{
    Intersect intersect = find_intersect(a, b);
    u32 radius = 6;
    draw_circle(color, intersect.pos, radius);
}

internal inline f64 time_in_seconds(void)
{
    struct timespec ts;
    timespec_get(&ts, TIME_UTC);
    return ts.tv_sec + ts.tv_nsec / 1e9;
}

internal inline void update_overlay_message(Overlay *overlay, f64 begin, f64 now)
{
    f64 dt = now - begin;
    u64 fps = 1 / dt;
    sprintf(overlay->msg, overlay->msg_format, WINDOW_WIDTH, WINDOW_HEIGHT, fps);
}

internal inline bool is_key_pressed(i32 key)
{
    return game.event.key.keysym.sym == key;
}

internal void draw_3d_buffer(Player player, Camera camera) // only need player pos and dir really...
{
    f64 theta = player.fov;
    f64 step = theta / (WINDOW_WIDTH - 1);
    f64 half_theta = theta / 2.0;
    
    i32 abs_x = player.dir.x;
    i32 abs_y = player.dir.y;

    f64 angle;
    i32 wall_x;

    f32 a = camera.a.y - camera.b.y;
    f32 b = camera.b.x - camera.a.x;
    f32 c = camera.a.x * camera.b.y - camera.b.x * camera.a.y;

    for (angle = -half_theta, wall_x = 0; angle < half_theta; angle += step, wall_x++) {
        int x = abs_x * cos(angle) - abs_y * sin(angle);
        int y = abs_x * sin(angle) + abs_y * cos(angle);
        V2 u = { .x = x, .y = y };
        Intersect intersect = find_intersect(player.pos, (v2_add(player.pos, v2_scale(u, 16))));

        f32 perp_wall_dist = fabs(a * intersect.pos.x + b * intersect.pos.y + c) / sqrt(a * a + b * b);

        u32 wall_height_multiplier = 16; 
        i32 wall_height = WINDOW_HEIGHT * wall_height_multiplier / perp_wall_dist;

        i32 wall_top = (-wall_height / 2) + (WINDOW_HEIGHT / 2);
        if (wall_top < 0) wall_top = 0;

        i32 wall_bottom = (wall_height / 2) + (WINDOW_HEIGHT / 2);
        if (wall_bottom >= WINDOW_HEIGHT) wall_top = WINDOW_HEIGHT - 1;
        
        SDL_Color color = intersect.vertical ? gray : light_gray;

        V2 wall_start = { .x = wall_x, wall_top };
        V2 wall_end = { .x = wall_x, wall_bottom };
                
        draw_line(color, wall_start, wall_end, 0);
    }
}

i32 main(void)
{
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        printf("ERROR: could not initialize window\n");
        return 1;
    }

    if (TTF_Init() != 0) {
        printf("ERROR: could not initialize ttf\n");
        return 1;
    }

    init_game(&game);

    Font font = { 0 };
    init_font(&font, "fonts/CascadiaMono.ttf", 24);

    Overlay overlay = { .state = OVERLAY_STATE_HIDDEN, .msg_format = "RESOLUTION: %dx%d, FPS: %d" };

    Player player = { .pos.x = 200, .pos.y = 200, .color = blue, .velocity = 1.0f, .radius = 6, .fov = PI / 2.0};
    Camera camera = { .radius = 6 };
    const u8 *keystate = SDL_GetKeyboardState(NULL);
    bool show_map = true;

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
                        show_map = !show_map;
                    }
                } break;
            }
        }

        if (keystate[SDL_SCANCODE_W]) player.pos.y -= player.velocity;
        if (keystate[SDL_SCANCODE_A]) player.pos.x -= player.velocity;
        if (keystate[SDL_SCANCODE_S]) player.pos.y += player.velocity;
        if (keystate[SDL_SCANCODE_D]) player.pos.x += player.velocity;

        // get mouse state
        SDL_GetMouseState(&game.cursor.x, &game.cursor.y);

        player.dir = v2_cell(v2_sub(game.cursor, player.pos));

        // compute camera coordinates
        camera.center = v2_add(player.pos, player.dir);
        camera.a = v2_add(camera.center, v2_scale(v2_normal(player.dir), -1));
        camera.b = v2_add(camera.center, v2_normal(player.dir));
        
        // NOTE: We only need the position of the intersects when drawing to the 2D map
        camera.a_intersect = find_intersect(camera.a, v2_add(camera.a, v2_scale(v2_sub(camera.a, player.pos), 16))).pos;
        camera.b_intersect = find_intersect(camera.b, v2_add(camera.b, v2_scale(v2_sub(camera.b, player.pos), 16))).pos;

        set_draw_color(white);
        SDL_RenderClear(game.renderer);

        if (show_map) {
            draw_walls(gray);
            draw_grid(black);

            // draw player and intersects with grid
            draw_circle(player.color, player.pos, player.radius);

            // scale by 16 to ensure that it searches for intersects far away
            draw_intersect(green, player.pos, v2_add(player.pos, v2_scale(player.dir, 16)));

            // draw camera
            draw_circle(black, camera.center, camera.radius);
            draw_line(black, player.pos, camera.center, 1);
            draw_line(blue, camera.a, camera.b, 1);
            draw_line(red, player.pos, camera.a, 1);
            draw_line(red, player.pos, camera.b, 1);

            draw_line(purple, camera.a, camera.a_intersect, 1);
            draw_line(purple, camera.b, camera.b_intersect, 1);
            // draw_3d_buffer(player, camera);
        }
        else {
            draw_3d_buffer(player, camera);
        }

        SDL_Surface *surface = TTF_RenderText_Shaded(font.font, overlay.msg, black, black_trans);
        SDL_Texture *texture = SDL_CreateTextureFromSurface(game.renderer, surface);
        const SDL_Rect rect = { .x = 0, .y = 0, .w = font.width * strlen(overlay.msg), .h = font.height };
        SDL_FreeSurface(surface);

        switch (overlay.state) {
            case OVERLAY_STATE_HIDDEN:  break;
            case OVERLAY_STATE_SHOWN:   SDL_RenderCopy(game.renderer, texture, NULL, &rect); break;
            case _overlay_state_count: break;
        }

        // draw to window
        SDL_RenderPresent(game.renderer);
        
        f64 now = time_in_seconds();

        update_overlay_message(&overlay, begin, now);
    }

    // SDL_DestroyTexture(texture);

    return 0;
}

// TODO: find new formula for drawing line 
// TODO: find new formula for drawing circle
