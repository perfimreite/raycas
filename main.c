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

bool draw_line(SDL_Renderer *r, Color color, int ax, int ay, int bx, int by, int padding)
{
    if (!set_draw_color(r, color)) return false;

    for (int i = -1*padding; i <= padding; i++) {
        if (SDL_RenderDrawLine(r, ax+i, ay+i, bx+i, by+i) != 0) return false;
    }
    return true;
}

bool draw_circle(SDL_Renderer *r, Color color, int a, int b, int rad)
{
    if (!set_draw_color(r, color)) return false;

    for (int y = b-rad; y <= b+rad; y++) {
        for (int x = a-rad; x <= a+rad; x++) {
            int hyp = (x-a)*(x-a) + (y-b)*(y-b);
            float acc = rad*1.5;
            if (hyp >= rad*rad - acc &&
                hyp <= rad*rad + acc) {
                if (SDL_RenderDrawLine(r, x, y, a, b) != 0) return false;
            }
        }
    }
    return true;
}

typedef struct {
    int x;
    int y;
} Vec2;

typedef struct {
    Vec2 pos;
} State;

typedef struct {
    Vec2 pos;
    Color color;
    int vel;
    int radius;
} Player;

bool draw_player(SDL_Renderer *r, Player p)
{
    return draw_circle(r, p.color, p.pos.x, p.pos.y, p.radius);
}

bool draw_intersects(SDL_Renderer *r, Color color, int ax, int ay, int bx, int by)
{
    if (!set_draw_color(r, color)) return false;

    double dx = bx - ax;
    double dy = by - ay;
    double l = sqrt(dx*dx + dy*dy);

    for (int x = 5; x < l && !hit_wall; x++) {
        double t = x/l;
        int cx = (int)(ax + t * dx);
        int cy = (int)(ay + t * dy);

        if (cx % CELL_SIZE == 0 && 
           ((map[cy/CELL_SIZE][cx/CELL_SIZE] == WALL) || dx < 0 && (map[cy/CELL_SIZE][cx/CELL_SIZE-1] == WALL))) {
            if (!draw_circle(r, color, cx, cy, INTERSECT_PADDING)) return false;
            hit_wall = true;
        }
        if (cy % CELL_SIZE == 0 && 
           ((map[cy/CELL_SIZE][cx/CELL_SIZE] == WALL) || dy < 0 && (map[cy/CELL_SIZE-1][cx/CELL_SIZE] == WALL))) {
            if (!draw_circle(r, color, cx, cy, INTERSECT_PADDING)) return false;
            hit_wall = true;
        }
    }
    
    return true;
}

int main(void)
{
    // create window and renderer    
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) return 1;
    SDL_Window *w = SDL_CreateWindow("Raycas", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, 0);
    SDL_Renderer *r = SDL_CreateRenderer(w, -1, 0);
    if (r == NULL) return 1;

    // declare variables
    SDL_Event event;
    Color bg = white;
    State s = {0};
    Player p = { .pos.x = 300, .pos.y = 400, .color = blue, .vel = 4, .radius = 10};

    // game loop
    bool quit = false;
    while (!quit) {
        hit_wall = false;
        // set background
        if (!set_draw_color(r, bg)) return 1;
        SDL_RenderClear(r);
        
        // draw walls and grid
        if (!draw_walls(r, gray)) return 1;
        if (!draw_grid(r, black)) return 1;
        
        // get mouse state
        SDL_GetMouseState(&s.pos.x, &s.pos.y);
        
        // draw player, line to mouse, and intersects with grid
        if (!draw_player(r, p)) return 1;
        if (!draw_line(r, black, p.pos.x, p.pos.y, s.pos.x, s.pos.y, 1)) return 1;
        if (!draw_intersects(r, green, p.pos.x, p.pos.y, s.pos.x, s.pos.y)) return 1;

        // loop through events
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    quit = true;
                    break;
                case SDL_KEYDOWN:
                    if (event.key.keysym.sym == SDLK_ESCAPE) quit = true; 
                    if (event.key.keysym.sym == SDLK_w) p.pos.y -= p.vel;
                    if (event.key.keysym.sym == SDLK_a) p.pos.x -= p.vel;
                    if (event.key.keysym.sym == SDLK_s) p.pos.y += p.vel;
                    if (event.key.keysym.sym == SDLK_d) p.pos.x += p.vel;
                    break;
            }
        }
        
        // draw to window
        SDL_RenderPresent(r);
    }

    return 0;
}

