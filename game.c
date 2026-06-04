#include "game.h"
#include "map.h"
#include "utils.h"

#include <string.h>

global const Color black             = { .r = 0  , .g = 0  , .b = 0  , .a = 255 };
global const Color white             = { .r = 255, .g = 255, .b = 255, .a = 255 };
global const Color gray              = { .r = 127, .g = 127, .b = 127, .a = 255 };
global const Color light_gray        = { .r = 63 , .g = 63 , .b = 63 , .a = 255 };
global const Color red               = { .r = 255, .g = 0  , .b = 0  , .a = 255 };
global const Color green             = { .r = 0  , .g = 255, .b = 0  , .a = 255 };
global const Color blue              = { .r = 0  , .g = 0  , .b = 255, .a = 255 };
global const Color light_blue        = { .r = 191, .g = 191, .b = 255, .a = 255 };
global const Color magenta           = { .r = 255, .g = 0  , .b = 255, .a = 255 };
global const Color cyan              = { .r = 0  , .g = 255, .b = 255, .a = 255 };
global const Color orange            = { .r = 255, .g = 165, .b = 0  , .a = 255 };
global const Color yellow            = { .r = 255, .g = 255, .b = 0  , .a = 255 };
global const Color transparent       = { .r = 0  , .g = 0  , .b = 0  , .a = 0   };
global const Color black_transparent = { .r = 0  , .g = 0  , .b = 0  , .a = 127 };

internal Rect make_rect(u32 x, u32 y, u32 w, u32 h)
{
    return (Rect) {
        .x = x,
        .y = y,
        .w = w,
        .h = h
    };
}

Button make_button(const char *text, i32 ptsize, Rect rect, Color fg, Color bg, Color border)
{
    return (Button) {
        .text = text,
        .pressed = false,
        .ptsize = ptsize,
        .rect = rect,
        .fg = fg,
        .bg = bg,
        .border = border
    };
}

Buttons buttons_init()
{
    Button start_button = make_button("START", 32,
                                      make_rect(WINDOW_CENTER_X - 100, WINDOW_CENTER_Y - 120, 200, 80),
                                      white, blue, black);
    Button quit_button = make_button("QUIT", 32,
                                      make_rect(WINDOW_CENTER_X - 100, WINDOW_CENTER_Y + 40, 200, 80),
                                      white, red, black);

    return (Buttons) {
        .items[0] = start_button,
        .items[1] = quit_button
    };
}


// void buttons_append(Buttons *buttons, Button button) {
//     buttons->items[buttons->count++] = button;
//     assert(buttons->count <= buttons->capacity);
// }

global Game game = { 0 };

internal Intersect find_intersect(V2f pos, V2f dir)
{
    V2f delta_dist = make_v2f(0, 0);
    V2f step       = make_v2f(0, 0);
    V2f side_dist  = make_v2f(0, 0);
    V2f map        = make_v2f(0, 0);

    delta_dist.x = dir.x == 0 ? 1e30 : fabs(1.0f / dir.x);
    delta_dist.y = dir.y == 0 ? 1e30 : fabs(1.0f / dir.y);

    V2f tile_relative_pos = v2f_scale(pos, 1.0f / CELL_SIZE);
    map.x = floor(tile_relative_pos.x);
    map.y = floor(tile_relative_pos.y);

    if (dir.x < 0) {
        step.x = -1;
        side_dist.x = (tile_relative_pos.x - map.x) * delta_dist.x;
    } else {
        step.x = 1;
        side_dist.x = (map.x + 1.0f - tile_relative_pos.x) * delta_dist.x;
    }

    if (dir.y < 0) {
        step.y = -1;
        side_dist.y = (tile_relative_pos.y - map.y) * delta_dist.y;
    } else {
        step.y = 1;
        side_dist.y = (map.y + 1.0f - tile_relative_pos.y) * delta_dist.y;
    }

    Intersect intersect = { 0 };

    for (;;) {
        if (side_dist.x < side_dist.y) {
            side_dist.x += delta_dist.x;
            map.x += step.x;
            intersect.vertical = false;
        } else {
            side_dist.y += delta_dist.y;
            map.y += step.y;
            intersect.vertical = true;
        }

        if (is_perim_tile(map.x, map.y)) {
            intersect.perim = true;
            break;
        }
        if (is_wall_tile(game.map_index, map.x, map.y)) {
            break;
        }
    }

    intersect.perp_wall_dist =
        (intersect.vertical ? (side_dist.y - delta_dist.y) : (side_dist.x - delta_dist.x)) * CELL_SIZE;
    intersect.pos = v2f_add(pos, v2f_scale(dir, intersect.perp_wall_dist));
    return intersect;
}

internal void draw_walls(Color color)
{
    for (u64 y = 0; y < WINDOW_HEIGHT; y += CELL_SIZE) {
        for (u64 x = 0; x < WINDOW_WIDTH; x += CELL_SIZE) {
            if (is_wall(game.map_index, x, y)) {
                Rect rect = make_rect(x, y, CELL_SIZE, CELL_SIZE);
                platform_draw_rect(color, rect);
            }
        }
    }
}

internal void draw_grid(Color color)
{
    for (u64 y = CELL_SIZE; y < WINDOW_HEIGHT; y += CELL_SIZE) {
        platform_draw_line(color, make_v2f(0, y), make_v2f(WINDOW_WIDTH, y));
    }

    for (u64 x = CELL_SIZE; x < WINDOW_WIDTH; x += CELL_SIZE) {
        platform_draw_line(color, make_v2f(x, 0), make_v2f(x, WINDOW_HEIGHT));
    }
}

internal void draw_crosshair(Color color)
{
    i32 l = 5;
    platform_draw_line(color, make_v2f(WINDOW_CENTER_X, WINDOW_CENTER_Y - l), make_v2f(WINDOW_CENTER_X, WINDOW_CENTER_Y + l));
    platform_draw_line(color, make_v2f(WINDOW_CENTER_X - l, WINDOW_CENTER_Y), make_v2f(WINDOW_CENTER_X + l, WINDOW_CENTER_Y));
}

internal void draw_3d_view(Player player)
{
    f32 angle_curr  = -player.fov / 2.0f;
    f32 angle_end   =  player.fov / 2.0f;
    f32 angle_step  =  player.fov / (WINDOW_WIDTH - 1);
    for (u64 buffer_x; angle_curr <= angle_end; angle_curr += angle_step, buffer_x++) {
        V2f curr_dir = v2f_rotate(player.dir, angle_curr);
        Intersect intersect = find_intersect(player.pos, curr_dir);

        f32 wall_height = WINDOW_HEIGHT * WALL_HEIGHT_MULTIPLIER / intersect.perp_wall_dist;

        f32 wall_top    = CLAMP((-wall_height / 2.0f) + (WINDOW_HEIGHT / 2.0f), 0.0f, WINDOW_HEIGHT - 1.0f);
        f32 wall_bottom = CLAMP(( wall_height / 2.0f) + (WINDOW_HEIGHT / 2.0f), 0.0f, WINDOW_HEIGHT - 1.0f);

        Color sky_color = light_blue;
        Color wall_color = intersect.perim ? black : (intersect.vertical ? gray : light_gray);

        V2f window_start = make_v2f(buffer_x, 0);
        V2f wall_start   = make_v2f(buffer_x, wall_top);
        V2f wall_end     = make_v2f(buffer_x, wall_bottom);

        platform_draw_line(sky_color, make_v2f(window_start.x, window_start.y), make_v2f(wall_start.x, wall_start.y));

        platform_draw_line(wall_color, make_v2f(wall_start.x, wall_start.y), make_v2f(wall_end.x, wall_end.y));
    }

    if (game.show_crosshair) {
        draw_crosshair(game.crosshair_color);
    }
}

internal void draw_player(Player player)
{
    platform_draw_circle(player.color, player.pos, player.radius, true);
}

internal void draw_player_fov(Color color, Player player, u32 beam_spread)
{
    f32 angle_curr  = -player.fov / 2.0f;
    f32 angle_end   =  player.fov / 2.0f;
    f32 angle_step  =  player.fov / (WINDOW_WIDTH - 1) * beam_spread;
    for (u64 buffer_x; angle_curr <= angle_end; angle_curr += angle_step, buffer_x++) {
        V2f curr_dir = v2f_rotate(player.dir, angle_curr);
        Intersect intersect = find_intersect(player.pos, curr_dir);

        platform_draw_line(color, player.pos, intersect.pos);
    }
}

internal void draw_map(Player player)
{
    draw_walls(gray);
    draw_grid(black);

    draw_player(player);
    draw_player_fov(orange, player, 8);
}

internal void minimap_draw_walls(Color color, f32 scale)
{
    for (u64 y = 0; y < WINDOW_HEIGHT; y += CELL_SIZE) {
        for (u64 x = 0; x < WINDOW_WIDTH; x += CELL_SIZE) {
            if (is_wall(game.map_index, x, y)) {
                Rect rect = make_rect(x * scale + game.minimap_dims.x, y * scale + game.minimap_dims.y,
                                      CELL_SIZE * scale, CELL_SIZE * scale);
                platform_draw_rect(color, rect);
            }
        }
    }
}

internal void minimap_draw_grid(Color color, f32 scale)
{
    f32 scaled_cell_size = CELL_SIZE * scale;

    for (u64 y = game.minimap_dims.y + scaled_cell_size; y < game.minimap_dims.y + game.minimap_dims.h; y += scaled_cell_size) {
        platform_draw_line(color, make_v2f(game.minimap_dims.x, y), make_v2f(game.minimap_dims.x + game.minimap_dims.w, y));
    }

    for (u64 x = game.minimap_dims.x + scaled_cell_size ; x < game.minimap_dims.x + game.minimap_dims.w; x += scaled_cell_size) {
        platform_draw_line(color, make_v2f(x, game.minimap_dims.y), make_v2f(x, game.minimap_dims.y + game.minimap_dims.h));
    }
}

internal void draw_minimap(Player player) {
    f64 scale = (f64)game.minimap_dims.w / WINDOW_WIDTH;

    // check that the horisontal and vertical scale are equal
    {
        f64 scale_vertical = (f64)game.minimap_dims.h / WINDOW_HEIGHT;
        assert(scale == scale_vertical && "Horisontal and vertical scale should be equal in the minipap");
    }

    platform_draw_rect(green, game.minimap_dims);

    minimap_draw_walls(gray, scale);
    minimap_draw_grid(black, scale);

    V2f player_minimap_pos = v2f_add(v2f_scale(player.pos, scale), make_v2f(game.minimap_dims.x, game.minimap_dims.y));
    platform_draw_circle(blue, player_minimap_pos, 2, true);
}

internal Rect center_button_text_in_rect(Button button)
{
    return (Rect) {
        .x = button.rect.x + (button.rect.w - button.ptsize * strlen(button.text)) / 2,
        .y = button.rect.y + (button.rect.h - button.ptsize) / 2,
        .w = button.ptsize * strlen(button.text),
        .h = button.ptsize
    };
}

internal void draw_button(Button button) {
    platform_draw_rect(button.bg, button.rect);
    Rect rect = center_button_text_in_rect(button);
    platform_draw_text(button.fg, button.bg, button.rect, button.text, button.ptsize);
}

internal void draw_menu(Buttons buttons)
{
    u64 button_count = sizeof(buttons) / sizeof(buttons.items[0]);
    for (u64 i = 0; i < button_count; i++) {
        draw_button(buttons.items[i]);
    }
}

internal void draw_overlay(Overlay overlay)
{
    Rect rect = make_rect(0, 0, overlay.ptsize * strlen(overlay.msg), overlay.ptsize);
    platform_draw_text(white, black_transparent, rect, overlay.msg, overlay.ptsize);
}

Player player_init()
{
    return (Player) {
        .pos.x = 200,
        .pos.y = 200,
        .dir.x = 1,
        .dir.y = 0,
        .vel = 200,
        .rotation_vel = 120,
        .fov = M_PI_2,
        .radius = 6,
        .color = blue
    };
}

void game_init(const char *title)
{
    game.title = title;
    game.width = WINDOW_WIDTH;
    game.height= WINDOW_HEIGHT;

    game.minimap_dims = (Rect) {
         .x = WINDOW_WIDTH * ((f32)3/4),
         .y = 0,
         .h = WINDOW_HEIGHT * ((f32)1/4),
         .w = WINDOW_WIDTH * ((f32)1/4)
     };

     game.mouse = make_v2f(0, 0);

     game.map_index = 0;

     game.state = STATE_MENU;
     game.show_crosshair = true;
     game.crosshair_color = white;
     game.quit = false;
}

void game_render(Player player, Overlay overlay, Buttons buttons)
{
    platform_clear_backbuffer(green);

    switch (game.state) {
        default: {} break;
        case STATE_GAME: {
            draw_3d_view(player);
            draw_minimap(player);
        } break;
        case STATE_MAP: {
            draw_map(player);
        } break;
        case STATE_MENU: {
            draw_menu(buttons);
        } break;
    }

    switch (overlay.state) {
       default : {} break;
       case OVERLAY_STATE_HIDDEN: {} break;
       case OVERLAY_STATE_SHOWN: {
            draw_overlay(overlay);
           // NOTE: can change to wrapped font rendering if msg gets too long
           // WARNING: this code block cannot be factored out into a seperate function.
           // doing so will cause draw_3d_view() to not draw anything *unless* a print statement
           // including both a character and newline appear above the call to draw_3d_view()
       } break;
    }
}

void game_state_toggle_game(void)
{
    game.state = STATE_GAME;
}

void game_state_toggle_map(void)
{
    if (game.state == STATE_GAME || game.state == STATE_MENU) {
        game.state = STATE_MAP;
    } else if (game.state == STATE_MAP) {
        game.state = STATE_GAME;
    }
}

void game_state_toggle_menu(void)
{
    // TODO: Play sound when entering menu
    if (game.state == STATE_GAME || game.state == STATE_MAP) {
        game.state = STATE_MENU;
    } else if (game.state == STATE_MENU) {
        game.state = STATE_GAME;
    }
}

void player_rotate_clockwise(Player *player, f64 dt)
{
    player->dir = v2f_rotate(player->dir, radians_from_degrees(player->rotation_vel) * dt);
}

void player_rotate_counterclockwise(Player *player, f64 dt)
{
    player->dir = v2f_rotate(player->dir, radians_from_degrees(-player->rotation_vel) * dt);
}


Overlay_State overlay_next_state(Overlay_State state)
{
    return (state + 1) % _overlay_state_count;
}

internal inline void overlay_update_message(Overlay *overlay, f64 dt, V2f player_pos)
{
    u32 fps = 1 / dt;
    sprintf(overlay->msg, overlay->msg_format,
            WINDOW_WIDTH, WINDOW_HEIGHT,
            fps, player_pos.x, player_pos.y);
}

void game_next_map_index()
{
    game.map_index = next_map_index(game.map_index);
}

void game_toggle_crosshair()
{
    game.show_crosshair = !game.show_crosshair;
}

internal bool point_in_rect(V2f p, Rect r)
{
    return ((p.x >= r.x) && (p.x < (r.x + r.w)) && (p.y >= r.y) && (p.y < (r.y + r.h)));
}
