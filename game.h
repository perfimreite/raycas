#include "constants.h"
#include "vector.h"
#include "map.h"

#ifndef GAME_H
#define GAME_H

typedef enum {
    KEY_UNKNOWN = 0,

    KEY_RETURN = '\r',
    KEY_ESCAPE = '\x1B',
    KEY_BACKSPACE = '\b',
    KEY_TAB = '\t',
    KEY_SPACE = ' ',
    KEY_EXCLAIM = '!',
    KEY_QUOTEDBL = '"',
    KEY_HASH = '#',
    KEY_PERCENT = '%',
    KEY_DOLLAR = '$',
    KEY_AMPERSAND = '&',
    KEY_QUOTE = '\'',
    KEY_LEFTPAREN = '(',
    KEY_RIGHTPAREN = ')',
    KEY_ASTERISK = '*',
    KEY_PLUS = '+',
    KEY_COMMA = ',',
    KEY_MINUS = '-',
    KEY_PERIOD = '.',
    KEY_SLASH = '/',

    KEY_0 = '0',
    KEY_1 = '1',
    KEY_2 = '2',
    KEY_3 = '3',
    KEY_4 = '4',
    KEY_5 = '5',
    KEY_6 = '6',
    KEY_7 = '7',
    KEY_8 = '8',
    KEY_9 = '9',

    KEY_COLON = ':',
    KEY_SEMICOLON = ';',
    KEY_LESS = '<',
    KEY_EQUALS = '=',
    KEY_GREATER = '>',
    KEY_QUESTION = '?',
    KEY_AT = '@',

    KEY_LEFTBRACKET = '[',
    KEY_BACKSLASH = '\\',
    KEY_RIGHTBRACKET = ']',
    KEY_CARET = '^',
    KEY_UNDERSCORE = '_',
    KEY_BACKQUOTE = '`',

    KEY_A = 'a',
    KEY_B = 'b',
    KEY_C = 'c',
    KEY_D = 'd',
    KEY_E = 'e',
    KEY_F = 'f',
    KEY_G = 'g',
    KEY_H = 'h',
    KEY_I = 'i',
    KEY_J = 'j',
    KEY_K = 'k',
    KEY_L = 'l',
    KEY_M = 'm',
    KEY_N = 'n',
    KEY_O = 'o',
    KEY_P = 'p',
    KEY_Q = 'q',
    KEY_R = 'r',
    KEY_S = 's',
    KEY_T = 't',
    KEY_U = 'u',
    KEY_V = 'v',
    KEY_W = 'w',
    KEY_X = 'x',
    KEY_Y = 'y',
    KEY_Z = 'z',
} Key;

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
    Rect text_rect;
    Color bg;
    Color fg;
    Color border;
} Button;

typedef struct {
    union {
        Button items[2];
        struct {
            Button start_button;
            Button quit_button;
        };
    };
} Buttons;

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
    Rect rect;
} Overlay;

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
    Map_Square_Kind map_square_kind;
} Intersect;

typedef enum {
    STATE_GAME,
    STATE_MAP,
    STATE_MENU,
    _state_count
} State;

typedef struct {
    u32 *data;
    u64 width;
    u64 height;
} Texture;

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
    Texture texture;
} Game;

void buttons_init(void);

void overlay_init(void);

void player_init(void);
void player_move_forward(f64 dt);
void player_move_backward(f64 dt);
void player_rotate_clockwise(f64 dt);
void player_rotate_counterclockwise(f64 dt);

void platform_clear_backbuffer   (Color color);
void platform_draw_point         (Color color, V2f a);
void platform_draw_line          (Color color, V2f a, V2f b);
void platform_draw_rect          (Color color, Rect rect);
void platform_draw_circle        (Color color, V2f a, i32 radius, bool filled);
V2f platform_get_text_dims       (i32 ptsize, const char *text);
Rect platform_center_text_in_rect(Rect rect, i32 ptsize, const char *text);
void platform_draw_text          (Color fg, Color bg, Rect rect, const char *text, i32 ptsize);
bool platform_point_in_rect      (V2f point, Rect rect);
bool platform_key_down           (Key key);
u32  platform_get_mouse_state    (V2f *mouse_pos);
bool platform_mouse_left_down    (u32 mouse_button_state);

void game_process_key(Key key);
bool game_process_mouse(void);

void game_init(const char *title);
void game_render(f64 dt);

#endif // GAME_H
