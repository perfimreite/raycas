#include <SDL2/SDL_error.h>
#include <SDL2/SDL_mouse.h>
#include <SDL2/SDL_render.h>
#include <math.h>
#include <stdbool.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdint.h>

#define WINDOW_WIDTH 1200 
#define WINDOW_HEIGHT 1200
#define ROWS 12
#define COLS 12
#define CELL_SIZE (WINDOW_HEIGHT / ROWS)
#define INTERSECT_PADDING 8

// colors
typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
} Color;

static Color black = { .r = 0  , .g = 0  , .b = 0  , .a = 255 };
static Color white = { .r = 255, .g = 255, .b = 255, .a = 255 };
static Color gray  = { .r = 127, .g = 127, .b = 127, .a = 255 };

static Color red   = { .r = 255, .g = 0  , .b = 0  , .a = 255 };
static Color green = { .r = 0  , .g = 255, .b = 0  , .a = 255 };
static Color blue  = { .r = 0  , .g = 0  , .b = 255, .a = 255 };

// map
#define EMPTY  0
#define WALL   1
#define PLAYER 2

static bool hit_wall = false;

static int map[ROWS][COLS] = {
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0},
    {0, 1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0},
    {0, 1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0},
    {0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0},
    {0, 1, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0},
    {0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 1},
    {0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 1, 1},
};

typedef struct {
    int x;
    int y;
} Vec2;

typedef struct {
    Vec2 pos;
} State;

typedef struct {
    Vec2 pos;
    Vec2 dir;
    Color color;
    int vel;
    int radius;
} Player;

typedef struct {
    Vec2 a;
    Vec2 r;
} Camera;

bool set_draw_color(SDL_Renderer *r, Color color) {
    if (SDL_SetRenderDrawColor(r, color.r, color.g,color.b,color.a) == 0) return true;
    return false;
}

bool draw_walls(SDL_Renderer *r, Color color) {
    if (!set_draw_color(r, color)) return false;
    for (size_t y = 1; y < ROWS; y++) {
        for (size_t x = 1; x < COLS; x++) {
            if (map[y][x] == WALL) {
                SDL_Rect rect = {
                    .x = x*CELL_SIZE+1,
                    .y = y*CELL_SIZE+1,
                    .h = CELL_SIZE,
                    .w = CELL_SIZE
                };
                if (SDL_RenderFillRect(r, &rect) != 0) return false;
            }
        }
    }
    return true;
}

bool draw_grid(SDL_Renderer *r, Color color)
{
    if (!set_draw_color(r, color)) return false;
    for (size_t i = 1; i < ROWS; i++) {
        if (SDL_RenderDrawLine(r, 0, i*CELL_SIZE, WINDOW_WIDTH, i*CELL_SIZE) != 0) return false;
    }
    for (size_t i = 1; i < COLS; i++) {
        if (SDL_RenderDrawLine(r, i*CELL_SIZE, 0, i*CELL_SIZE, WINDOW_HEIGHT) != 0) return false;
    }
    return true;
}

bool draw_line(SDL_Renderer *r, Color color, Vec2 a, Vec2 b, int padding)
{
    if (!set_draw_color(r, color)) return false;

    for (int i = -1*padding; i <= padding; i++) {
        if (SDL_RenderDrawLine(r, a.x+i, a.y+i, b.x+i, b.y+i) != 0) return false;
    }
    return true;
}

bool draw_circle(SDL_Renderer *r, Color color, Vec2 a, int radius)
{
    if (!set_draw_color(r, color)) return false;

    for (int y = a.y-radius; y <= a.y+radius; y++) {
        for (int x = a.x-radius; x <= a.x+radius; x++) {
            int hyp = (x-a.x)*(x-a.x) + (y-a.y)*(y-a.y);
            float acc = radius*1.5;
            if (hyp >= radius*radius - acc &&
                hyp <= radius*radius + acc) {
                if (SDL_RenderDrawLine(r, x, y, a.x, a.y) != 0) return false;
            }
        }
    }
    return true;
}

bool draw_player(SDL_Renderer *r, Player p)
{
    return draw_circle(r, p.color, p.pos, p.radius);
}

bool draw_intersects(SDL_Renderer *r, Color color, Vec2 a, Vec2 b)
{
    if (!set_draw_color(r, color)) return false;

    double dx = b.x - a.x;
    double dy = b.y - a.y;
    double l = sqrt(dx*dx + dy*dy);

    for (int x = 5; x < l && !hit_wall; x++) {
        double t = x/l;
        int cx = (int)(a.x + t * dx);
        int cy = (int)(a.y + t * dy);

        if (cx % CELL_SIZE == 0 && 
           (map[cy/CELL_SIZE][cx/CELL_SIZE] == WALL || (dx < 0 && (map[cy/CELL_SIZE][cx/CELL_SIZE-1] == WALL)))) {
            if (!draw_circle(r, color, (Vec2){ .x = cx, .y = cy }, INTERSECT_PADDING)) return false;
            hit_wall = true;
        }
        if (cy % CELL_SIZE == 0 && 
           (map[cy/CELL_SIZE][cx/CELL_SIZE] == WALL || (dy < 0 && (map[cy/CELL_SIZE-1][cx/CELL_SIZE] == WALL)))) {
            if (!draw_circle(r, color, (Vec2){ .x = cx, .y = cy }, INTERSECT_PADDING)) return false;
            hit_wall = true;
        }
    }
    
    return true;
}

Vec2 vec2_add(Vec2 a, Vec2 b) { return (Vec2){ .x = a.x + b.x, .y = a.y + b.y }; }
Vec2 vec2_sub(Vec2 a, Vec2 b) { return (Vec2){ .x = a.x - b.x, .y = a.y - b.y }; }
Vec2 vec2_mul(Vec2 a, Vec2 b) { return (Vec2){ .x = a.x * b.x, .y = a.y * b.y }; }
Vec2 vec2_div(Vec2 a, Vec2 b) { return (Vec2){ .x = a.x / b.x, .y = a.y / b.y }; }

Vec2 vec2_scale(Vec2 a, int k) { return (Vec2){ .x = a.x * k, .y = a.y * k }; }

Vec2 vec2_unit(Vec2 a, Vec2 b)
{
    Vec2 u = vec2_sub(b, a);
    float l = sqrt(u.x*u.x + u.y*u.y) / 100;
    Vec2 v = { .x = u.x/l, u.y/l };
    int scalar = 1;
    return vec2_scale(v, scalar);
}

Vec2 vec2_norm(Vec2 a, Vec2 b)
{
    Vec2 u = vec2_sub(b, a);
    return (Vec2){ .x = -1*u.y, .y = u.x };
}

int main(void)
{
    // create window and renderer    
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) return 1;
    SDL_Window *window = SDL_CreateWindow("Raycas", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, 0);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, 0);
    if (renderer == NULL) return 1;

    // declare variables
    SDL_Event event;
    Color bg = white;
    State state = {0};
    Player player = { .pos.x = 300, .pos.y = 400, .color = blue, .vel = 4, .radius = 10};
    Camera camera = {0};

    // game loop
    bool quit = false;
    while (!quit) {
        hit_wall = false;
        // set background
        if (!set_draw_color(renderer, bg)) return 1;
        SDL_RenderClear(renderer);
        
        // draw walls and grid
        if (!draw_walls(renderer, gray)) return 1;
        if (!draw_grid(renderer, black)) return 1;
        
        // get mouse state
        SDL_GetMouseState(&state.pos.x, &state.pos.y);
        player.dir = vec2_unit(player.pos, state.pos);

        // center of camera
        Vec2 c = { .x = player.pos.x + player.dir.x, .y = player.pos.y + player.dir.y };

        camera.a = vec2_unit(vec2_norm(player.pos, player.dir), c);
        camera.r = vec2_unit(c, vec2_scale(vec2_norm(player.pos, player.dir), -1));
        // printf("l = (%d, %d), r = (%d, %d)\n", cam.l.x, cam.l.y, cam.r.x, cam.r.y);

        // draw player, line to mouse, and intersects with grid
        if (!draw_player(renderer, player)) return 1;
        if (!draw_intersects(renderer, green, player.pos, (Vec2){ .x = player.pos.x + player.dir.x * ROWS, .y = player.pos.y + player.dir.y * COLS })) return 1;
       
        // draw pos + dir
        if (!draw_circle(renderer, black, c, INTERSECT_PADDING)) return 1;
        if (!draw_line(renderer, black, player.pos, c, 1)) return 1;
        
        // draw camera plane
        if (!draw_line(renderer, blue, vec2_sub(c, camera.a), vec2_add(c, camera.r), 1)) return 1;

        // loop through events
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    quit = true;
                    break;
                case SDL_KEYDOWN:
                    if (event.key.keysym.sym == SDLK_ESCAPE) quit = true; 
                    if (event.key.keysym.sym == SDLK_w) player.pos.y -= player.vel;
                    if (event.key.keysym.sym == SDLK_a) player.pos.x -= player.vel;
                    if (event.key.keysym.sym == SDLK_s) player.pos.y += player.vel;
                    if (event.key.keysym.sym == SDLK_d) player.pos.x += player.vel;
                    break;
            }
        }
        
        // draw to window
        SDL_RenderPresent(renderer);
    }

    return 0;
}

