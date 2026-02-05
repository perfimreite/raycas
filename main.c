#include "constants.h"
#include "meta.h"
#include "vector.h"

#include <stdbool.h>
#include <stdio.h>
#include <time.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_error.h>
#include <SDL2/SDL_mouse.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_ttf.h>

static const SDL_Color black       = { .r = 0  , .g = 0  , .b = 0  , .a = 255 };
static const SDL_Color white       = { .r = 255, .g = 255, .b = 255, .a = 255 };
static const SDL_Color gray        = { .r = 127, .g = 127, .b = 127, .a = 255 };
static const SDL_Color red         = { .r = 255, .g = 0  , .b = 0  , .a = 255 };
static const SDL_Color green       = { .r = 0  , .g = 255, .b = 0  , .a = 255 };
static const SDL_Color blue        = { .r = 0  , .g = 0  , .b = 255, .a = 255 };

static const SDL_Color black_trans = { .r = 0  , .g = 0  , .b = 0  , .a = 31 };

static SDL_Window *w = NULL;
static SDL_Renderer *r = NULL;

// map
#define EMPTY 0
#define WALL  1

static i32 map[ROWS][COLS] = {
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

i32 map_square(i32 x, i32 y)
{
    return map[y/CELL_SIZE][x/CELL_SIZE];
}

typedef struct {
    i32 height;
    i32 width;

    const char *path;
    TTF_Font *font;
} Font;

void init_font(Font *font, i32 height, const char *path)
{
    font->height = height;
    font->width = height / 2;
    font->path = path;

    font->font = TTF_OpenFont(path, height);
    if (!font->font) {
        printf("ERROR: could not load font\n");
        exit(1);
    }
}

typedef enum {
    Debug_Overlay_Hidden,
    Debug_Overlay_Shown,
    // Debug_Overlay_Advanced
} Overlay_State;

typedef struct {
    Overlay_State state;
    char msg[1024];
    const char *msg_format;
    // char advanced[1024];
} Debug_Overlay;

typedef struct {
    V2 pos;
    V2 dir;
    SDL_Color color;
    f32 velocity;
    i32 radius;
} Player;

typedef struct {
    V2 center;
    V2 a;
    V2 b;

    i32 radius;
} Camera;

typedef struct {
    bool quit;
    V2 pos;
} State;

void set_draw_color(SDL_Color color)
{
    if (SDL_SetRenderDrawColor(r, color.r, color.g, color.b, color.a) != 0) {
        printf("ERROR: could not set render draw color\n");
        exit(1);
    }
}

void draw_walls(SDL_Color color)
{
    set_draw_color(color);

    for (u64 y = 0; y < WINDOW_HEIGHT; y += CELL_SIZE) {
        for (u64 x = 0; x < WINDOW_WIDTH; x += CELL_SIZE) {
            if (map_square(x, y) == WALL) {
                SDL_Rect rect = { .x = x+1, .y = y+1, .h = CELL_SIZE, .w = CELL_SIZE };
                if (SDL_RenderFillRect(r, &rect) != 0) exit(1);
            }
        }
    }
}

void draw_line(SDL_Color color, V2 a, V2 b, i32 padding)
{
    set_draw_color(color);

    for (i32 i = -padding; i <= padding; i++) {
        if (SDL_RenderDrawLine(r, a.x + i, a.y + i, b.x + i, b.y + i) != 0) exit(1);
    }
}

void draw_grid(SDL_Color color)
{
    for (u64 y = CELL_SIZE; y < WINDOW_HEIGHT; y += CELL_SIZE) {
        draw_line(color, (V2){ .x = 0, .y = y }, (V2){ .x = WINDOW_WIDTH, .y = y }, 0);
    }

    for (u64 x = CELL_SIZE; x < WINDOW_WIDTH; x += CELL_SIZE) {
        draw_line(color, (V2){ .x = x, .y = 0 }, (V2){ .x = x, .y = WINDOW_HEIGHT }, 0);
    }
}

void draw_circle(SDL_Color color, V2 a, i32 radius)
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

void draw_intersect(SDL_Color color, V2 a, V2 b)
{
    set_draw_color(color);

    V2 u = v2_sub(b, a);
    f32 l = v2_len(u);

    for (i32 i = 0; i < l; i++) {
        f32 t = i / l;
        i32 x = a.x + t * u.x;
        i32 y = a.y + t * u.y;

        // check for oob
        if ((0 > x || x > WINDOW_WIDTH) || (0 > y || y> WINDOW_HEIGHT)) return;

        i32 pos = map_square(x, y);

        if (
            (x % CELL_SIZE == 0 && (pos == WALL || (u.x < 0 && map_square(x - 1, y) == WALL))) || 
            (y % CELL_SIZE == 0 && (pos == WALL || (u.y < 0 && map_square(x, y - 1) == WALL)))
           ) {
            V2 intersect = { .x = x, .y = y };
            draw_circle(color, intersect, 6);
            return;
        }
    }
}

f64 time_in_seconds(void)
{
    struct timespec ts;
    timespec_get(&ts, TIME_UTC);
    return ts.tv_sec + ts.tv_nsec / 1e9;
}

i32 main(void)
{
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        printf("ERROR: could not initialize window\n");
        return 1;
    }

    w = SDL_CreateWindow("Raycas", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, 0);
    if (!w) {
        printf("ERROR: could not create window\n");
        return 1;
    }

    r = SDL_CreateRenderer(w, -1, 0);
    if (!r) {
        printf("ERROR: could not create renderer\n");
        return 1;
    }

    if (TTF_Init() != 0) {
        printf("ERROR: could not initialize ttf\n");
        return 1;
    }

    time_t begin = time_in_seconds();
    i32 prev_elapsed_time = 0;
    u64 frames = 0;
    
    Font font = { 0 };
    init_font(&font, 24, "fonts/CascadiaMono.ttf");

    Debug_Overlay overlay = { .state = Debug_Overlay_Hidden, .msg_format = "RES:%dx%d, FPS: %d" };

    SDL_Event event;
    State state = { 0 };
    Player player = { .pos.x = 200, .pos.y = 200, .color = blue, .velocity = 1.0f, .radius = 6};
    Camera camera = { .radius = 6 };
    const u8 *keystate = SDL_GetKeyboardState(NULL);

    while (!state.quit) {
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT: state.quit = true; break;
                case SDL_KEYDOWN: {
                    if      (event.key.keysym.sym == SDLK_ESCAPE) {
                        state.quit = true;
                    }
                    else if (event.key.keysym.sym == SDLK_o) {
                        // 3 is amount of overlays
                        overlay.state = (overlay.state + 1) % 2;
                    }
                    break;
                }
            }
        }

        if (keystate[SDL_SCANCODE_W]) player.pos.y -= player.velocity;
        if (keystate[SDL_SCANCODE_A]) player.pos.x -= player.velocity;
        if (keystate[SDL_SCANCODE_S]) player.pos.y += player.velocity;
        if (keystate[SDL_SCANCODE_D]) player.pos.x += player.velocity;

        // get mouse state
        SDL_GetMouseState(&state.pos.x, &state.pos.y);

        player.dir = v2_cell(v2_sub(state.pos, player.pos));

        // compute camera coordinates
        camera.center = v2_add(player.pos, player.dir);
        camera.a = v2_normal(player.dir);
        camera.b = v2_scale(v2_normal(player.dir), -1);

        set_draw_color(white);
        SDL_RenderClear(r);

        draw_walls(gray);
        draw_grid(black);

        // draw player and intersects with grid
        draw_circle(player.color, player.pos, player.radius);

        // scale by 16 to ensure that it searches for intersects far away
        draw_intersect(green, player.pos, v2_add(player.pos, v2_scale(player.dir, 16)));

        // draw camera
        draw_circle(black, camera.center, camera.radius);
        draw_line(black, player.pos, camera.center, 1);
        draw_line(blue, v2_add(camera.center, camera.a), v2_add(camera.center, camera.b), 1);
        draw_line(red, player.pos, v2_add(camera.center, camera.a), 1);
        draw_line(red, player.pos, v2_add(camera.center, camera.b), 1);
        
        // updates every frame, this has to be dogshit
        SDL_Surface *surface = TTF_RenderText_Shaded(font.font, overlay.msg, black, black_trans);
        SDL_Texture *texture = SDL_CreateTextureFromSurface(r, surface);
        SDL_Rect rect = { .x = 10, .y = 10, .w = font.width * strlen(overlay.msg), .h = font.height };
        SDL_FreeSurface(surface);
        
        switch (overlay.state) {
            case Debug_Overlay_Hidden:   break;
            case Debug_Overlay_Shown:    SDL_RenderCopy(r, texture, NULL, &rect); break;
            // case Debug_Overlay_Advanced: break;
        }
        
        time_t now = time_in_seconds();
        f64 elapsed_time = now - begin;

        // updates every 10ms
        if (elapsed_time > prev_elapsed_time + 0.01f) {
            f64 dt = elapsed_time - prev_elapsed_time;
            u64 fps = (f64)frames / dt;
            sprintf(overlay.msg, overlay.msg_format, WINDOW_WIDTH, WINDOW_HEIGHT, fps); 

            frames = 0;
            prev_elapsed_time = elapsed_time;
        }

        // draw to window
        SDL_RenderPresent(r);
        
        frames += 1;
    }

    // SDL_DestroyTexture(texture);

    return 0;
}

// TODO: find new formula for drawing line 
// TODO: find new formula for drawing circle
