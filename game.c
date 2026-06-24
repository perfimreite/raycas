#include "game.h"
#include "map.h"

#define STB_IMAGE_IMPLEMENTATION
#include "thirdparty/stb_image.h"

#include "utils.h"
#include <stdio.h>

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

Buttons buttons = { 0 };
Overlay overlay = { 0 };
Player player   = { 0 };
Game game       = { 0 };

internal void set_texture_fallback()
{
    game.texture.width  = DEFAULT_TEXTURE_WIDTH;
    game.texture.height = DEFAULT_TEXTURE_HEIGHT;

    for (u64 y = 0; y < game.texture.height; y++) {
        for (u64 x = 0; x < game.texture.width; x++) {
            // i32 xor_color = (y * 256 / game.texture.width) ^ (x * 256 / game.texture.height);
            // i32 x_color   =  y * 256 / game.texture.width;
            // i32 y_color   =  x * 256 / game.texture.height;
            // i32 xy_color  =  x * 128 / game.texture.height + y * 128 / game.texture.width;

            // game.texture.data[game.texture.width * x + y] = 65536 * 254 * (y != x && y != game.texture.width - x); // flat red texture with black cross
            // game.texture.data[game.texture.width * x + y] = xycolor + 256 * xycolor + 65536 * xycolor;             // sloped greyscale
            // game.texture.data[game.texture.width * x + y] = 256 * xycolor + 65536 * xycolor;                       // sloped yellow gradient
            // game.texture.data[game.texture.width * x + y] = xorcolor + 256 * xorcolor + 65536 * xorcolor;          // xor greyscale
            // game.texture.data[game.texture.width * x + y] = 256 * xorcolor;                                        // xor green
            game.texture.data[game.texture.width * x + y] = 65536 * 192 * (x % 16 && y % 16);              // red bricks
            // game.texture.data[game.texture.width * x + y] = 65536 * ycolor;                                        // red gradient
            // game.texture.data[game.texture.width * x + y] = 128 + 256 * 128 + 65536 * 128;                         // flat grey texture
        }
    }
}

internal void texture_init();

internal void load_texture(const char *path)
{
    FILE *file = fopen(path, "rb");

    if (file != NULL) {
        i32 width;
        i32 height;
        i32 components_per_pixel;
        u8 *data = stbi_load_from_file(file, &width, &height, &components_per_pixel, STBI_rgb_alpha);
        if (data != NULL) {
            game.texture.data = (u32 *)data;
            game.texture.width = width;
            game.texture.height = height;

            for (u64 i = 0; i < game.texture.width * game.texture.height; i++) {
                // Swap blue and red bytes to match color interpretation of `Color`
                game.texture.data[i] =
                    (game.texture.data[i] & 0xFF00FF00) |
                    ((game.texture.data[i] & 0x00FF0000) >> 16)|
                    ((game.texture.data[i] & 0x000000FF) << 16);
            }
        }
        // stbi_image_free(data);

        fclose(file);
    } else {
        // Fallback in case of error trying to read from file
        set_texture_fallback();
    }
}

internal Rect make_rect(u32 x, u32 y, u32 w, u32 h)
{
    return (Rect) {
        .x = x,
        .y = y,
        .w = w,
        .h = h
    };
}

internal Button make_button(const char *text, i32 ptsize, Rect rect, Color fg, Color bg, Color border)
{
    return (Button) {
        .text = text,
        .pressed = false,
        .ptsize = ptsize,
        .rect = rect,
        .text_rect = platform_center_text_in_rect(rect, ptsize, text),
        .fg = fg,
        .bg = bg,
        .border = border
    };
}

void buttons_init(void)
{
    Button start_button = make_button("START", 32,
                                      make_rect(WINDOW_CENTER_X - 100, WINDOW_CENTER_Y - 120, 200, 80),
                                      white, blue, black);
    Button quit_button = make_button("QUIT", 32,
                                     make_rect(WINDOW_CENTER_X - 100, WINDOW_CENTER_Y + 40, 200, 80),
                                     white, red, black);

    buttons.items[0] = start_button;
    buttons.items[1] = quit_button;
}

void overlay_init(void)
{
    overlay.state = OVERLAY_STATE_HIDDEN;
    overlay.msg_format = "RESOLUTION:%dx%d FPS:%d POS:(%.1f, %.1f)";
    overlay.ptsize = 16;
}

internal inline void overlay_update_message(Overlay *overlay, f64 dt, V2f player_pos)
{
    u32 fps = 1.0f / dt;
    sprintf(overlay->msg, overlay->msg_format,
            game.width, game.height,
            fps, player_pos.x, player_pos.y);
}

internal Overlay_State overlay_next_state(Overlay_State state)
{
    return (state + 1) % _overlay_state_count;
}

void player_init(void)
{
    player.pos.x = 200;
    player.pos.y = 200;
    player.dir.x = 1;
    player.dir.y = 0;
    player.vel = 200;
    player.rotation_vel = 120;
    player.fov = M_PI_2;
    player.radius = 6;
    player.color = blue;
}

void player_move_forward(f64 dt)
{
    if (game.state == STATE_MENU) return;

    V2f old_pos = player.pos;

    player.pos = v2f_add(player.pos, v2f_scale(player.dir, dt * player.vel));
    if (is_perim(player.pos.x, player.pos.y) || is_wall(game.map_index, player.pos.x, player.pos.y)) {
        player.pos = old_pos;
    }
}

void player_move_backward(f64 dt)
{
    if (game.state == STATE_MENU) return;

    V2f old_pos = player.pos;

    player.pos = v2f_sub(player.pos, v2f_scale(player.dir, dt * player.vel));
    if (is_perim(player.pos.x, player.pos.y) || is_wall(game.map_index, player.pos.x, player.pos.y)) {
        player.pos = old_pos;
    }
}

void player_rotate_clockwise(f64 dt)
{
    if (game.state == STATE_MENU) return;

    player.dir = v2f_rotate(player.dir, radians_from_degrees(player.rotation_vel) * dt);
}

void player_rotate_counterclockwise(f64 dt)
{
    if (game.state == STATE_MENU) return;

    player.dir = v2f_rotate(player.dir, radians_from_degrees(-player.rotation_vel) * dt);
}

internal void draw_walls(Color color)
{
    for (u64 y = 0; y < game.height; y += CELL_SIZE) {
        for (u64 x = 0; x < game.width; x += CELL_SIZE) {
            if (is_wall(game.map_index, x, y)) {
                Rect rect = make_rect(x, y, CELL_SIZE, CELL_SIZE);
                platform_draw_rect(color, rect);
            } else if (is_texture(game.map_index, x, y)) {
                Rect rect = make_rect(x, y, CELL_SIZE, CELL_SIZE);
                platform_draw_rect(red, rect);
            }
        }
    }
}

internal void draw_grid(Color color)
{
    for (u64 y = CELL_SIZE; y < game.height; y += CELL_SIZE) {
        platform_draw_line(color, make_v2f(0, y), make_v2f(game.width, y));
    }

    for (u64 x = CELL_SIZE; x < game.width; x += CELL_SIZE) {
        platform_draw_line(color, make_v2f(x, 0), make_v2f(x, game.height));
    }
}

internal Intersect find_intersect(V2f pos, V2f dir)
{
    V2f delta_dist = make_v2f(0, 0);
    V2f step       = make_v2f(0, 0);
    V2f side_dist  = make_v2f(0, 0);

    delta_dist.x = dir.x == 0 ? 1e30 : fabs(1.0f / dir.x);
    delta_dist.y = dir.y == 0 ? 1e30 : fabs(1.0f / dir.y);

    V2f tile_relative_pos = v2f_scale(pos, 1.0f / CELL_SIZE);
    V2f map = v2f_floor(tile_relative_pos);

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
            intersect.map_square_kind = WALL;
            break;
        }
        if (is_texture_tile(game.map_index, map.x, map.y)) {
            intersect.map_square_kind = TEXTURE;
            break;
        }
    }

    intersect.perp_wall_dist =
        (intersect.vertical ? (side_dist.y - delta_dist.y) : (side_dist.x - delta_dist.x)) * CELL_SIZE;
    intersect.pos = v2f_add(pos, v2f_scale(dir, intersect.perp_wall_dist));
    return intersect;
}

internal void draw_crosshair(Color color)
{
    i32 l = 5;
    platform_draw_line(color, make_v2f(WINDOW_CENTER_X, WINDOW_CENTER_Y - l), make_v2f(WINDOW_CENTER_X, WINDOW_CENTER_Y + l));
    platform_draw_line(color, make_v2f(WINDOW_CENTER_X - l, WINDOW_CENTER_Y), make_v2f(WINDOW_CENTER_X + l, WINDOW_CENTER_Y));
}

Color color_from_u32(u32 color)
{
    // 0011 0101
    return (Color) {
        .b = (color>>(8*0))&0xFF,
        .g = (color>>(8*1))&0xFF,
        .r = (color>>(8*2))&0xFF,
        .a = (color>>(8*3))&0xFF,
    };
}

internal void draw_3d_view(Player player)
{
    f32 angle_curr  = -player.fov / 2.0f;
    f32 angle_end   =  player.fov / 2.0f;
    f32 angle_step  =  player.fov / (game.width - 1);
    for (u64 x = 0; angle_curr <= angle_end; angle_curr += angle_step, x++) {
        V2f curr_dir = v2f_rotate(player.dir, angle_curr);
        Intersect intersect = find_intersect(player.pos, curr_dir);
        f32 wall_height = game.height * WALL_HEIGHT_MULTIPLIER / intersect.perp_wall_dist;

        Color sky_color = light_blue;
        Color wall_color = intersect.perim ? black : (intersect.vertical ? gray : light_gray);

        f32 wall_top = CLAMP((-wall_height / 2.0f) + (game.height / 2.0f), 0.0f, game.height - 1.0f);
        V2f window_start = make_v2f(x, 0);
        V2f wall_start = make_v2f(x, wall_top);
        platform_draw_line(sky_color, window_start, wall_start);

        switch (intersect.map_square_kind) {
        case EMPTY:
        case WALL: {
            f32 wall_bottom = CLAMP(( wall_height / 2.0f) + (game.height / 2.0f), 0.0f, game.height - 1.0f);
            V2f wall_end = make_v2f(x, wall_bottom);
            platform_draw_line(wall_color, wall_start, wall_end);
        } break;
        case TEXTURE: {
            f32 wall_x =
                intersect.vertical ?
                player.pos.x + intersect.perp_wall_dist * curr_dir.x :
                player.pos.y + intersect.perp_wall_dist * curr_dir.y;
            wall_x -= floor(wall_x);

            V2f texture_index = make_v2f(0, 0);

            texture_index.x = (u32)(wall_x * game.texture.width);
            if ((!intersect.vertical && curr_dir.x > 0) || (intersect.vertical && curr_dir.y < 0)) {
                texture_index.x = game.texture.width - texture_index.x - 1;
            }

            f32 step = game.texture.height / wall_height;
            f32 texture_pos = (wall_top - game.height / 2.0 + wall_height / 2.0) * step;

            for (u64 y = 0; y < wall_height; y++, texture_pos += step) {
                texture_index.y = (u32)texture_pos & (game.texture.height - 1);

                u32 pixel = game.texture.data[game.texture.height * (u32)texture_index.x + (u32)texture_index.y];
                if (intersect.vertical) {
                    pixel = (pixel >> 1) & 8355711;
                }
                Color color = color_from_u32(pixel);

                platform_draw_point(color, make_v2f(x, wall_top + y));
            }
        } break;
        }
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
    f32 angle_step  =  player.fov / (game.width - 1) * beam_spread;
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
    draw_player_fov(black_transparent, player, 8);
}

internal void draw_minimap_walls(Color color, f32 scale)
{
    f32 scaled_cell_size = CELL_SIZE * scale;

    for (u64 y = game.minimap_dims.y; y < game.minimap_dims.y + game.minimap_dims.h; y += scaled_cell_size) {
        for (u64 x = game.minimap_dims.x; x < game.minimap_dims.x + game.minimap_dims.w; x += scaled_cell_size) {
            if (is_wall(game.map_index, (x - game.minimap_dims.x) / scale, (y - game.minimap_dims.y) / scale)) {
                Rect rect = make_rect(x, y, scaled_cell_size, scaled_cell_size);
                platform_draw_rect(color, rect);
            } else if (is_texture(game.map_index, (x - game.minimap_dims.x) / scale, (y - game.minimap_dims.y) / scale)) {
                Rect rect = make_rect(x, y, scaled_cell_size, scaled_cell_size);
                platform_draw_rect(red, rect);
            }
        }
    }
}

internal void draw_minimap_grid(Color color, f32 scale)
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
    f64 scale = (f64)game.minimap_dims.w / game.width;

    platform_draw_rect(green, game.minimap_dims);

    draw_minimap_walls(gray, scale);
    draw_minimap_grid(black, scale);

    V2f player_minimap_pos = v2f_add(v2f_scale(player.pos, scale), make_v2f(game.minimap_dims.x, game.minimap_dims.y));
    platform_draw_circle(blue, player_minimap_pos, 2, true);
}

internal void draw_button(Button button) {
    platform_draw_rect(button.bg, button.rect);
    platform_draw_text(button.fg, button.bg, button.text_rect, button.text, button.ptsize);
}

internal void draw_menu(Buttons buttons)
{
    for (u64 i = 0; i < ARRAY_LEN(buttons.items); i++) {
        draw_button(buttons.items[i]);
    }
}

internal void draw_overlay(Overlay overlay)
{
    V2f dims = platform_get_text_dims(overlay.ptsize, overlay.msg);
    overlay.rect = make_rect(0, 0, dims.x, dims.y);
    platform_draw_text(white, black_transparent, overlay.rect, overlay.msg, overlay.ptsize);
}

void game_init(const char *title)
{
    game.title = title;
    game.width = WINDOW_WIDTH;
    game.height = WINDOW_HEIGHT;

    game.minimap_dims = (Rect) {
         .x = game.width * ((f32)3/4),
         .y = 0,
         .h = game.height * ((f32)1/4),
         .w = game.width * ((f32)1/4)
     };

     game.mouse = make_v2f(0, 0);

     game.map_index = 0;

     game.state = STATE_MENU;
     game.show_crosshair = true;
     game.crosshair_color = white;
     game.quit = false;

     // game_texture_init();
     load_texture(TEXTURES_PATH"redbrick.png");
}

void game_render(f64 dt)
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
           overlay_update_message(&overlay, dt, player.pos);
           draw_overlay(overlay);
           // NOTE: can change to wrapped font rendering if msg gets too long
       } break;
    }
}

internal void game_state_toggle_map(void)
{
    if (game.state == STATE_GAME) {
        game.state = STATE_MAP;
    } else if (game.state == STATE_MAP) {
        game.state = STATE_GAME;
    }
}

internal void game_state_toggle_menu(void)
{
    // TODO: Play sound when entering menu
    if (game.state == STATE_GAME || game.state == STATE_MAP) {
        game.state = STATE_MENU;
    } else if (game.state == STATE_MENU) {
        game.state = STATE_GAME;
    }
}

internal void game_toggle_crosshair(void)
{
    game.show_crosshair = !game.show_crosshair;
}

// NOTE: returns false if quit button is pressed
bool game_process_mouse(void)
{
    if (game.state != STATE_MENU) {
        return true;
    }

    u32 mouse_button_state = platform_get_mouse_state(&game.mouse);

    if (platform_mouse_left_down(mouse_button_state)) {
        for (u64 i = 0; i < ARRAY_LEN(buttons.items); i++) {
            if (platform_point_in_rect(game.mouse, buttons.items[i].rect)) {
                buttons.items[i].pressed = true;
            } else {
                buttons.items[i].pressed = false;
            }
        }
    }

    if (buttons.start_button.pressed) {
        game_state_toggle_menu();
    }
    if (buttons.quit_button.pressed) {
        return false;
    }

    return true;
}

void game_process_key(Key key)
{
    switch (key) {
        default: {} break;
        case KEY_O: {
            overlay.state = overlay_next_state(overlay.state);
        } break;
        case KEY_M: {
            game_state_toggle_map();
        } break;
        case KEY_ESCAPE: {
            game_state_toggle_menu();
        } break;
        case KEY_C: {
            game_toggle_crosshair();
        } break;
        case KEY_N: {
            game.map_index = next_map_index(game.map_index);
        } break;
    }
}
