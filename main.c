#include "meta.h"

#include <math.h>
#include <stdbool.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_error.h>
#include <SDL2/SDL_mouse.h>
#include <SDL2/SDL_render.h>

#define WINDOW_WIDTH 600 
#define WINDOW_HEIGHT 600
#define ROWS 12
#define COLS 12
#define CELL_SIZE (WINDOW_HEIGHT / ROWS)
#define INTERSECT_PADDING 8

// colors
typedef struct {
	u8 r;
	u8 g;
	u8 b;
	u8 a;
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

static i32 map[ROWS][COLS] = {
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
	i32 x;
	i32 y;
} Vec2;

typedef struct {
	Vec2 pos;
	bool quit;
} State;

typedef struct {
	Vec2 pos;
	Vec2 dir;
	Color color;
	i32 velocity;
	i32 radius;
} Player;

typedef struct {
	Vec2 a;
	Vec2 b;
} Camera;

/* === vector functions === */
Vec2 vec2_add(Vec2 a, Vec2 b) {
	return (Vec2){ .x = a.x + b.x, .y = a.y + b.y };
}

Vec2 vec2_sub(Vec2 a, Vec2 b) {
	return (Vec2){ .x = a.x - b.x, .y = a.y - b.y };
}

Vec2 vec2_mul(Vec2 a, Vec2 b) {
	return (Vec2){ .x = a.x*b.x, .y = a.y*b.y };
}

Vec2 vec2_div(Vec2 a, Vec2 b) {
	return (Vec2){ .x = a.x/b.x, .y = a.y/b.y };
}

i32 vec2_square_len(Vec2 a, Vec2 b) {
	f64 dx = b.x - a.x;
	f64 dy = b.y - a.y;
    return dx*dx + dy*dy;
}

f32 vec2_len(Vec2 a, Vec2 b) {
    return sqrt(vec2_square_len(a, b));
}

Vec2 vec2_scale(Vec2 a, i32 k) {
	return (Vec2){ .x = a.x*k, .y = a.y*k };
}

Vec2 vec2_unit(Vec2 a, Vec2 b)
{
	Vec2 u = vec2_sub(b, a);
	f32 l = sqrt(u.x*u.x + u.y*u.y) / 100;
	Vec2 v = { .x = u.x/l, u.y/l };
	return vec2_scale(v, 1);
}

Vec2 vec2_norm(Vec2 a, Vec2 b)
{
	Vec2 u = vec2_sub(b, a);
	return (Vec2){ .x = -1*u.y, .y = u.x };
}


/* === drawing functions === */
bool set_draw_color(SDL_Renderer *r, Color color) {
	if (SDL_SetRenderDrawColor(r, color.r, color.g, color.b, color.a) != 0) {
        printf("ERROR: could not set render draw color\n");
        return false;
    }
    return true;
}

bool draw_walls(SDL_Renderer *r, Color color) {
	if (!set_draw_color(r, color)) return false;
	for (u64 y = 1; y < ROWS; y++) {
		for (u64 x = 1; x < COLS; x++) {
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
	// assumes that there are equally many rows and columns
	for (u64 i = 1; i < ROWS; i++) {
		if (SDL_RenderDrawLine(r, 0, i*CELL_SIZE, WINDOW_WIDTH, i*CELL_SIZE) != 0) return false;
		if (SDL_RenderDrawLine(r, i*CELL_SIZE, 0, i*CELL_SIZE, WINDOW_HEIGHT) != 0) return false;
	}
	return true;
}

bool draw_line(SDL_Renderer *r, Color color, Vec2 a, Vec2 b, i32 padding)
{
	if (!set_draw_color(r, color)) return false;
	for (i32 i = -1*padding; i <= padding; i++) {
		if (SDL_RenderDrawLine(r, a.x+i, a.y+i, b.x+i, b.y+i) != 0) return false;
	}
	return true;
}

bool draw_circle(SDL_Renderer *r, Color color, Vec2 a, i32 radius)
{
	if (!set_draw_color(r, color)) return false;
	for (i32 y = a.y - radius; y <= a.y + radius; y++) {
		for (i32 x = a.x - radius; x <= a.x + radius; x++) {
			i32 hyp =  vec2_square_len(a, (Vec2){ .x = x, .y = y });
			f32 acc = radius*1.5;
			if (hyp >= radius*radius - acc &&
				hyp <= radius*radius + acc) {
				if (SDL_RenderDrawLine(r, x, y, a.x, a.y) != 0) return false;
			}
		}
	}
	return true;
}

bool draw_i32ersects(SDL_Renderer *r, Color color, Vec2 a, Vec2 b)
{
	if (!set_draw_color(r, color)) return false;

	f64 l = vec2_len(a, b);
    Vec2 a_to_b = vec2_sub(b, a);

	for (i32 x = 5; x < l && !hit_wall; x++) {
		f64 t = x/l;
		i32 cx = a.x + t * a_to_b.x;
		i32 cy = a.y + t * a_to_b.y;
		i32 current_pos = map[cy/CELL_SIZE][cx/CELL_SIZE];
		Vec2 intersect = { .x = cx, .y = cy };

		if (cx % CELL_SIZE == 0 && 
		   (current_pos == WALL || (a_to_b.x < 0 && map[cy/CELL_SIZE][cx/CELL_SIZE-1] == WALL))) {
			if (!draw_circle(r, color, intersect, INTERSECT_PADDING)) return false;
			hit_wall = true;
		} else if (cy % CELL_SIZE == 0 && 
				  (current_pos == WALL || (a_to_b.y < 0 && map[cy/CELL_SIZE-1][cx/CELL_SIZE] == WALL))) {
			if (!draw_circle(r, color, intersect, INTERSECT_PADDING)) return false;
			hit_wall = true;
		}
	}

	return true;
}

i32 main(void)
{
	// create window and renderer
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

	SDL_Event event;
	State state = {0};
	Player player = { .pos.x = 300, .pos.y = 400, .color = blue, .velocity = 4, .radius = 10};
	Camera camera = {0};

	while (!state.quit) {
		hit_wall = false;
		// set background
		if (!set_draw_color(renderer, white)) return 1;
		SDL_RenderClear(renderer);

		// draw walls and grid
		if (!draw_walls(renderer, gray)) return 1;
		if (!draw_grid(renderer, black)) return 1;

		// get mouse state
		SDL_GetMouseState(&state.pos.x, &state.pos.y);
		player.dir = vec2_unit(player.pos, state.pos);

		// center of camera
		Vec2 camera_line_middel = vec2_add(player.pos, player.dir);

		camera.a = vec2_unit(vec2_norm(player.pos, player.dir), camera_line_middel);
		camera.b = vec2_unit(camera_line_middel, vec2_scale(vec2_norm(player.pos, player.dir), -1));
		printf("a = (%d, %d), b = (%d, %d)\n", camera.a.x, camera.a.y, camera.b.x, camera.b.y);

		// draw player and i32ersects with grid
		if (!draw_circle(renderer, player.color, player.pos, player.radius)) return 1;
		// scale by 16 to ensure that it searches for i32ersects far away
		if (!draw_i32ersects(renderer, green, player.pos, vec2_add(player.pos, vec2_scale(player.dir, 16)))) return 1;

		// draw camera pos and dir
		if (!draw_circle(renderer, black, camera_line_middel, INTERSECT_PADDING)) return 1;
		if (!draw_line(renderer, black, player.pos, camera_line_middel, 1)) return 1;

		// draw camera plane
		if (!draw_line(renderer, blue, vec2_sub(camera_line_middel, camera.a), vec2_add(camera_line_middel, camera.b), 1)) return 1;

		// loop through events
		while (SDL_PollEvent(&event)) {
			switch (event.type) {
				case SDL_QUIT:
					state.quit = true;
					break;
				case SDL_KEYDOWN:
					if (event.key.keysym.sym == SDLK_ESCAPE) state.quit = true; 
					else if (event.key.keysym.sym == SDLK_w) player.pos.y -= player.velocity;
					else if (event.key.keysym.sym == SDLK_a) player.pos.x -= player.velocity;
					else if (event.key.keysym.sym == SDLK_s) player.pos.y += player.velocity;
					else if (event.key.keysym.sym == SDLK_d) player.pos.x += player.velocity;
					break;
			}
		}

		// draw to window
		SDL_RenderPresent(renderer);
	}

	return 0;
}

