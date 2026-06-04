#include "constants.h"
#include "vector.h"

#ifndef GAME_H
#define GAME_H

typedef struct {
    u32 x;
    u32 y;
    u32 w;
    u32 h;
} Rect;

typedef struct {
    u8 r;
    u8 g;
    u8 b;
    u8 a;
} Color;

typedef struct {
    const char *text;
    bool pressed;

    i32 ptsize;
    Rect rect;
    Color bg;
    Color fg;
    Color border;
} Button;

// typedef struct {
//     Button *items;
//     u32 count;
//     u32 capacity;
// } Buttons;

typedef struct {
    union {
        Button items[2];
        struct {
            Button start_button;
            Button quit_button;
        };
    };
} Buttons;

typedef struct {
    V2f pos;
    V2f dir;

    i32 vel;
    i32 rotation_vel;
    f32 fov;
    u32 radius;

    Color color;
} Player;

typedef struct {
    V2f pos;
    f64 perp_wall_dist;
    bool vertical;
    bool perim;
} Intersect;

typedef enum {
    OVERLAY_STATE_HIDDEN,
    OVERLAY_STATE_SHOWN,
    _overlay_state_count,
} Overlay_State;

typedef struct {
    Overlay_State state;
    char msg[128];
    const char *msg_format;
    i32 ptsize;
} Overlay;

typedef enum {
    STATE_GAME,
    STATE_MAP,
    STATE_MENU,
    _state_count
} State;

typedef struct {
    const char *title;

    u32 width;
    u32 height;

    Rect minimap_dims;

    V2f mouse;

    i32 map_index;

    State state;
    bool show_crosshair;
    Color crosshair_color;
    bool quit;
} Game;

Button make_button(const char *text, i32 ptsize, Rect rect, Color fg, Color bg, Color border);
Buttons buttons_init();
// void buttons_append(Buttons *buttons, Button button);

// Rect make_rect(u32 x, u32 y, u32 w, u32 h);

void platform_clear_backbuffer(Color color);
void platform_draw_point      (Color color, V2f a);
void platform_draw_line       (Color color, V2f a, V2f b);
void platform_draw_rect       (Color color, Rect rect);
void platform_draw_circle     (Color color, V2f a, i32 radius, bool filled);
void platform_draw_text       (Color fg, Color bg, Rect rect, const char *text, i32 ptsize);

Player player_init();

void game_init(const char *title);
void game_render(Player player, Overlay overlay, Buttons buttons);
// void game_update();

#endif // GAME_H
