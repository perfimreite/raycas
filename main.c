#include "meta.h"
#include "vector.h"

#include <assert.h>
#include <stdbool.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_error.h>
#include <SDL2/SDL_mouse.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_ttf.h>

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define COLS 16
#define ROWS 12
#define CELL_SIZE (WINDOW_HEIGHT / ROWS)
#define INTERSECT_RADIUS 6

static const SDL_Color black = { .r = 0  , .g = 0  , .b = 0  , .a = 255 };
static const SDL_Color white = { .r = 255, .g = 255, .b = 255, .a = 255 };
static const SDL_Color gray  = { .r = 127, .g = 127, .b = 127, .a = 255 };
static const SDL_Color red   = { .r = 255, .g = 0  , .b = 0  , .a = 255 };
static const SDL_Color green = { .r = 0  , .g = 255, .b = 0  , .a = 255 };
static const SDL_Color blue  = { .r = 0  , .g = 0  , .b = 255, .a = 255 };

// map
#define EMPTY  0
#define WALL   1
#define PLAYER 2

static i32 map[ROWS][COLS] = {
  {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {1, 1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 1, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0},
  {0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0},
  {0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0},
};

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
} Camera;

typedef struct {
  bool quit;
  V2 pos;
} State;

void set_draw_color(SDL_Renderer *r, SDL_Color color) {
  if (SDL_SetRenderDrawColor(r, color.r, color.g, color.b, color.a) != 0) {
    printf("ERROR: could not set render draw color\n");
    exit(1);
  }
}

i32 get_map_square(i32 x, i32 y)
{
  return map[y/CELL_SIZE][x/CELL_SIZE];
}


void draw_walls(SDL_Renderer *r, SDL_Color color) {
  set_draw_color(r, color);
  for (u64 y = 0; y < WINDOW_HEIGHT; y += CELL_SIZE) {
    for (u64 x = 0; x < WINDOW_WIDTH; x += CELL_SIZE) {
      if (get_map_square(x, y) == WALL) {
        SDL_Rect rect = { .x = x+1, .y = y+1, .h = CELL_SIZE, .w = CELL_SIZE };
        if (SDL_RenderFillRect(r, &rect) != 0) exit(1);
      }
    }
  }
}

void draw_grid(SDL_Renderer *r, SDL_Color color)
{
  set_draw_color(r, color);
  for (u64 y = CELL_SIZE; y < WINDOW_HEIGHT; y += CELL_SIZE) {
    if (SDL_RenderDrawLine(r, 0, y, WINDOW_WIDTH, y) != 0)  exit(1);
  }
  for (u64 x = CELL_SIZE; x < WINDOW_WIDTH; x += CELL_SIZE) {
    if (SDL_RenderDrawLine(r, x, 0, x, WINDOW_HEIGHT) != 0) exit(1);
  }
}

void draw_line(SDL_Renderer *r, SDL_Color color, V2 a, V2 b, i32 padding)
{
  set_draw_color(r, color);
  for (i32 i = -1*padding; i <= padding; i++) {
    if (SDL_RenderDrawLine(r, a.x+i, a.y+i, b.x+i, b.y+i) != 0) exit(1);
  }
}

void draw_circle(SDL_Renderer *r, SDL_Color color, V2 a, i32 radius)
{
  f32 acc = radius*2;

  set_draw_color(r, color);
  for (i32 y = a.y - radius; y <= a.y + radius; y++) {
    for (i32 x = a.x - radius; x <= a.x + radius; x++) {
      V2 curr = { .x = x, .y = y };
      i32 hyp =  v2_square_len(v2_sub(curr, a));
      if (hyp >= radius*radius - acc && hyp <= radius*radius + acc) {
        if (SDL_RenderDrawLine(r, x, y, a.x, a.y) != 0) exit(1);
      }
    }
  }
}

void draw_intersect(SDL_Renderer *r, SDL_Color color, V2 a, V2 b)
{
  set_draw_color(r, color);

  V2 u = v2_sub(b, a);
  f64 l = v2_len(u);

  for (i32 i = 0; i < l; i++) {
    f64 t = i/l;
    i32 cx = a.x + t * u.x;
    i32 cy = a.y + t * u.y;

    // check for oob
    if ((0 > cx || cx > WINDOW_WIDTH) || (0 > cy || cy> WINDOW_HEIGHT)) return;

    i32 pos = get_map_square(cx, cy);

    if (
        (cx % CELL_SIZE == 0 && (pos == WALL || (u.x < 0 && get_map_square(cx-1, cy) == WALL))) || 
        (cy % CELL_SIZE == 0 && (pos == WALL || (u.y < 0 && get_map_square(cx, cy-1) == WALL)))
       ) {
      V2 intersect = { .x = cx, .y = cy };
      draw_circle(r, color, intersect, INTERSECT_RADIUS);
      return;
    }
  }
}

i32 main(void)
{

  assert(ROWS/WINDOW_HEIGHT == COLS/WINDOW_WIDTH);

  if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
    printf("ERROR: could not initialize window\n");
    return 1;
  }

  SDL_Window *window = SDL_CreateWindow("Raycas", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, 0);

  SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, 0);
  if (renderer == NULL) {
    printf("ERROR: could not create renderer\n");
    return 1;
  }
  
  if (TTF_Init() != 0) {
	  printf("ERROR: could not initialize ttf\n");
	  return 1;
  }

  TTF_Font* font = TTF_OpenFont("./fonts/consola.ttf", 16);
  if (font == NULL) {
    printf("ERROR: could not load font\n");
    return 1;
  }
  SDL_Surface *surface_msg = TTF_RenderText_Solid(font, "FPS: ", black);

  SDL_Texture *texture_msg = SDL_CreateTextureFromSurface(renderer, surface_msg);
  SDL_FreeSurface(surface_msg);
  SDL_Rect rect = { .x = 0, .y = 0, .w = 100, .h = 100 };

  SDL_Event event;
  State state = {0};
  Player player = { .pos.x = 200, .pos.y = 200, .color = blue, .velocity = 1.f, .radius = 6};
  Camera camera = {0};
  const u8 *keystate = SDL_GetKeyboardState(NULL);

  while (!state.quit) {
    while (SDL_PollEvent(&event)) {
      switch (event.type) {
        case SDL_QUIT: state.quit = true; break;
        case SDL_KEYDOWN: if (event.key.keysym.sym == SDLK_ESCAPE) state.quit = true; break;
      }
    }
  
    if (keystate[SDL_SCANCODE_W]) {
      player.pos.y -= player.velocity;
    }
    if (keystate[SDL_SCANCODE_A]) {
      player.pos.x -= player.velocity;
    }
    if (keystate[SDL_SCANCODE_S]) {
      player.pos.y += player.velocity;
    }
    if (keystate[SDL_SCANCODE_D]) {
      player.pos.x += player.velocity;
    }

    // get mouse state
    SDL_GetMouseState(&state.pos.x, &state.pos.y);
    player.dir = v2_unit(v2_sub(state.pos, player.pos));

    // center of camera
    camera.center = v2_add(player.pos, player.dir);
    camera.a = v2_normal(player.dir);
    camera.b = v2_scale(v2_normal(player.dir), -1);

    set_draw_color(renderer, white);
    SDL_RenderClear(renderer);

    draw_walls(renderer, gray);
    draw_grid(renderer, black);

    // draw player and intersects with grid
    draw_circle(renderer, player.color, player.pos, player.radius);

    // scale by 16 to ensure that it searches for intersects far away
    draw_intersect(renderer, green, player.pos, v2_add(player.pos, v2_scale(player.dir, 16)));
  
    // draw camera
    draw_circle(renderer, black, camera.center, INTERSECT_RADIUS);
    draw_line(renderer, black, player.pos, camera.center, 1);
    draw_line(renderer, blue, v2_add(camera.center, camera.a), v2_add(camera.center, camera.b), 1);
    draw_line(renderer, red, player.pos, v2_add(camera.center, camera.a), 1);
    draw_line(renderer, red, player.pos, v2_add(camera.center, camera.b), 1);

    SDL_RenderCopy(renderer, texture_msg, NULL, &rect);
    
    // draw to window
    SDL_RenderPresent(renderer);
    

  }

  SDL_DestroyTexture(texture_msg);

  return 0;
}

