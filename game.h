#include "vector.h"

#ifndef GAME_H
#define GAME_H

#ifdef HIGH_RESOLUTION
    #define WINDOW_WIDTH 1600
    #define WINDOW_HEIGHT 1200
#else
    #define WINDOW_WIDTH 960
    #define WINDOW_HEIGHT 720
#endif

#define OVERLAY_TEXT_SIZE 128

#define DEFAULT_TEXTURE_WIDTH 64
#define DEFAULT_TEXTURE_HEIGHT 64
static_assert(DEFAULT_TEXTURE_WIDTH == DEFAULT_TEXTURE_HEIGHT, "Should be equal");

#define WINDOW_CENTER_X (((WINDOW_WIDTH)  - 1.0f) / 2.0f)
#define WINDOW_CENTER_Y (((WINDOW_HEIGHT) - 1.0f) / 2.0f)

#define COLS 16
#define ROWS 12
static_assert(WINDOW_WIDTH / COLS == WINDOW_HEIGHT / ROWS, "Should be equal");
#define CELL_SIZE ((f32)(WINDOW_WIDTH) / (COLS))

#define MAP_COUNT 2

#define FONT_FILE "fonts/CascadiaMono.ttf"

#define TEXTURES_PATH "assets/wolfenstein_textures/"
#define FIRST_TEXTURE_ID 2

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

typedef u32 Box_Style_Flag;
enum {
    BOX_STYLE_FLAG_ROUNDED = (1<<0),
    BOX_STYLE_FLAG_BORDER  = (1<<1),
    BOX_STYLE_FLAG_ALL     = 0x00FFFFFF,
};

typedef struct {
    const char *text;
    i32 ptsize;
    Rect rect;
    Box_Style_Flag style_flag;
    Color fg;
    Color bg;
    Color border_color;
    i32 border_radius;
    i32 border_thickness;
} Box;

typedef struct {
    b32 pressed;
    b32 hovered;
    Box box;
} Button;

typedef enum {
    OVERLAY_STATE_HIDDEN,
    OVERLAY_STATE_SHOWN,
    _overlay_state_count,
} Overlay_State;

typedef struct {
    Overlay_State state;
    char text[OVERLAY_TEXT_SIZE];
    const char *text_format;
    i32 ptsize;
    Rect rect;
} Overlay;

typedef struct {
    V2f pos;
    V2f dir;
    V2f camera_plane;

    i32 vel;
    i32 rotation_vel;
    f32 fov;
    u32 radius;

    Color color;
} Player;

typedef struct {
    V2f pos;
    f32 perp_wall_dist;
    f32 eucledian_dist;
    b32 horizontal;
    u32 map_tile_value;
} Intersect;

typedef enum {
    VIEW_GAME,
    VIEW_MAP,
    VIEW_MENU,
} View;

typedef struct {
    u32 id;
    Color tile_color;
    u32 *data;
    u32 width;
    u32 height;

    char *name;
} Texture;

typedef struct {
    Texture *items;
    size_t count;
    size_t capacity;
} Textures;

typedef enum {
    CURSOR_KIND_ARROW,
    CURSOR_KIND_HAND
} Cursor_Kind;

typedef struct {
    // for events (actions based on clicks)
    b32 is_down;
    b32 was_down;

    // for polling (actions based on holding down)
    b32 active;
} Key;

typedef struct {
    // Cursor position
    V2f pos;
    // Cursor kind
    Cursor_Kind active_cursor;

    // Mouse buttons
    Key key_left;
    Key key_middel;
    Key key_right;
} Mouse_State;

typedef union {
    Key v[9];

    struct {
        // Movement
        Key key_w;
        Key key_a;
        Key key_s;
        Key key_d;

        // Controls
        Key key_c;
        Key key_o;
        Key key_m;
        Key key_n;
        Key key_escape;

        // NOTE: All keys must come before `key_end`
        Key key_end;
    };
} Keyboard_State;

typedef struct {
    union {
        Button v[2];
        struct {
            Button start;
            Button quit;

            // NOTE: All buttons must come before `button_end`
            Button button_end;
        };
    } buttons;
    Color bg;
} Menu;

typedef struct {
    b32 quit;

    u32 width;
    u32 height;

    Rect minimap_dims;

    Mouse_State mouse_state;
    Keyboard_State keyboard_state;

    u32 map_index;

    View view;

    Color crosshair_color;
    b32 show_crosshair;
    Textures textures;

    Menu menu;
} Game;

Rect make_rect(u32 x, u32 y, u32 w, u32 h);

void overlay_init(void);

void player_init(void);

void game_init(void);
void game_update(f64 dt);
void game_render(f64 dt);

void platform_clear_backbuffer(Color color);
void platform_draw_point(Color color, V2f a);
void platform_draw_line(Color color, V2f a, V2f b);
void platform_draw_rect(Color color, Rect rect);
void platform_draw_rect_rounded(Color color, Rect rect, i32 radius);
void platform_draw_circle(Color color, V2f a, i32 radius, b32 filled);
V2f  platform_get_text_dims(i32 ptsize, const char *text);
Rect platform_center_text_in_rect(Rect rect, i32 ptsize, const char *text);
void platform_draw_text(Color fg, Color bg, Rect rect, const char *text, i32 ptsize);
b32  platform_point_in_rect(V2f point, Rect rect);
void platform_get_mouse_state(Mouse_State *mouse_state);
void platform_set_cursor(Cursor_Kind cursor);

#endif // GAME_H

