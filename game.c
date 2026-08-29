#include "game.h"
#include "utils.h"

#define STB_IMAGE_IMPLEMENTATION
#include "thirdparty/stb_image.h"

#include <stdio.h>

global const Color black             = { .r = 0  , .g = 0  , .b = 0  , .a = 255 };
global const Color white             = { .r = 255, .g = 255, .b = 255, .a = 255 };
global const Color gray              = { .r = 127, .g = 127, .b = 127, .a = 255 };
global const Color light_gray        = { .r = 63 , .g = 63 , .b = 63 , .a = 255 };
global const Color red               = { .r = 255, .g = 0  , .b = 0  , .a = 255 };
global const Color green             = { .r = 0  , .g = 255, .b = 0  , .a = 255 };
global const Color blue              = { .r = 0  , .g = 0  , .b = 255, .a = 255 };
global const Color light_blue        = { .r = 191, .g = 191, .b = 255, .a = 255 };
// global const Color magenta           = { .r = 255, .g = 0  , .b = 255, .a = 255 };
// global const Color cyan              = { .r = 0  , .g = 255, .b = 255, .a = 255 };
// global const Color orange            = { .r = 255, .g = 165, .b = 0  , .a = 255 };
// global const Color yellow            = { .r = 255, .g = 255, .b = 0  , .a = 255 };
// global const Color transparent       = { .r = 0  , .g = 0  , .b = 0  , .a = 0   };
global const Color black_transparent = { .r = 0  , .g = 0  , .b = 0  , .a = 127 };

global Overlay overlay = {0};
global Player player   = {0};
Game game              = {0};

global i32 map[MAP_COUNT][ROWS][COLS] = {
    {
        {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0},
        {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0},
        {1, 1, 0, 0, 0, 0, 2, 2, 0, 0, 0, 1, 1, 1, 0, 0},
        {0, 1, 0, 0, 0, 0, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 1, 0, 0, 0, 0, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 1, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 1, 0, 0, 0, 0, 0, 2, 2, 0, 0, 0, 0, 0, 0, 0},
        {0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1},
        {0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 1, 0, 0, 0, 1},
        {0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 1},
    },
    {
        {1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0},
        {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0},
        {1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {1, 1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0},
        {1, 1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1},
        {0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 1},
        {0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1},
    }
};

internal void next_map_index()
{
    game.map_index = (game.map_index + 1) % MAP_COUNT;
}

internal Map_Tile_Kind get_map_tile(i32 map_index, f32 x, f32 y)
{
    i32 cy = y / CELL_SIZE;
    i32 cx = x / CELL_SIZE;
    assert(cy >= 0);
    assert(cx >= 0);
    return map[map_index][cy][cx];
}

internal b32 is_wall(i32 map_index, f32 x, f32 y)
{
    return get_map_tile(map_index, x, y) == MAP_TILE_WALL;
}

internal b32 is_texture(i32 map_index, f32 x, f32 y)
{
    return get_map_tile(map_index, x, y) == MAP_TILE_TEXTURE;
}

internal b32 is_perim(f32 x, f32 y)
{
    return (0.0f > y || y >= WINDOW_HEIGHT) || (0.0f > x || x >= WINDOW_WIDTH);
}

internal Texture get_fallback_texture()
{
    Texture texture = {0};

    texture.width = DEFAULT_TEXTURE_WIDTH;
    texture.height = DEFAULT_TEXTURE_HEIGHT;
    texture.data = malloc(sizeof(u32) * DEFAULT_TEXTURE_WIDTH * DEFAULT_TEXTURE_HEIGHT);

    for (u64 y = 0; y < texture.height; y++) {
        for (u64 x = 0; x < texture.width; x++) {
            // u32 xor_color = (y * 256 / texture.width) ^ (x * 256 / texture.height);
            // u32 x_color   =  y * 256 / texture.width;
            u32 y_color   =  x * 256 / texture.height;
            // u32 xy_color  =  x * 128 / texture.height + y * 128 / texture.width;

            // texture.data[texture.width * x + y] = 65536 * 254 * (y != x && y != texture.width - x); // flat red texture with black cross
            // texture.data[texture.width * x + y] = xy_color + 256 * xy_color + 65536 * xy_color;     // sloped greyscale
            // texture.data[texture.width * x + y] = 256 * xy_color + 65536 * xy_color;                // sloped yellow gradient
            // texture.data[texture.width * x + y] = xor_color + 256 * xor_color + 65536 * xor_color;  // xor greyscale
            // texture.data[texture.width * x + y] = 256 * xor_color;                                  // xor green
            // texture.data[texture.width * x + y] = 65536 * 192 * (x % 16 && y % 16);                 // red bricks
            texture.data[texture.width * x + y] = (u32)65536 * y_color;                                  // red gradient
            // texture.data[texture.width * x + y] = 128 + 256 * 128 + 65536 * 128;                    // flat grey texture
        }
    }

    return texture;
}

internal Texture load_texture_from_file(const char *file_path)
{
    Texture texture = {0};

    FILE *file = fopen(file_path, "rb");

    if (file != NULL) {
        i32 width, height, components;
        u8 *data = stbi_load_from_file(file, &width, &height, &components, STBI_rgb_alpha);

        if (data != NULL) {
            texture.data = (u32 *)data;
            texture.width = width;
            texture.height = height;

            for (u64 i = 0; i < texture.width * texture.height; i++) {
                // Swap blue and red bytes to match color interpretation of `Color`
                texture.data[i] =
                    ( texture.data[i] & 0xFF00FF00) |
                    ((texture.data[i] & 0x00FF0000) >> 16) |
                    ((texture.data[i] & 0x000000FF) << 16);
            }
        }

        fclose(file);
    } else {
        fprintf(stderr, "ERROR: Failed to load texture: `%s`\n", file_path);

        // Fallback in case of error trying to read from file
        texture = get_fallback_texture();
    }

    return texture;
}

Rect make_rect(u32 x, u32 y, u32 w, u32 h)
{
    return (Rect) {
        .x = x,
        .y = y,
        .w = w,
        .h = h
    };
}

internal Box make_box(const char *text, i32 ptsize, Rect rect,
                      Box_Style box_style, Color fg, Color bg)
{
    return (Box) {
        .text = text,
        .ptsize = ptsize,

        .rect = rect,
        .style = box_style,
        .fg = fg,
        .bg = bg,
    };
}

internal Button make_button(const char *text, i32 ptsize, Rect rect,
                            Box_Style box_style, Color fg, Color bg)
{
    return (Button) {
        .pressed = false,
        .box = make_box(text, ptsize, rect, box_style, fg, bg)
    };
}

internal Buttons create_menu_buttons(void)
{
    Button start = make_button("START", 32,
                               make_rect(WINDOW_CENTER_X - 100, WINDOW_CENTER_Y - 120, 200, 80),
                               BOX_STYLE_ROUNDED,
                               white, blue);
    Button quit = make_button("QUIT", 32,
                              make_rect(WINDOW_CENTER_X - 100, WINDOW_CENTER_Y + 40, 200, 80),
                              BOX_STYLE_ROUNDED,
                              white, red);

    Buttons buttons = {0};
    buttons.v[0] = start;
    buttons.v[1] = quit;

    // Check that right amount of buttons are in union
    ASSERT(&buttons.button_end - &buttons.v[0] == ARRAY_COUNT(buttons.v));

    return buttons;
}

void overlay_init(void)
{
    overlay.state = OVERLAY_STATE_HIDDEN;
    overlay.text_format = "RESOLUTION:%dx%d FPS:%d POS:(%.1f, %.1f)";
    overlay.ptsize = 16;
}

internal inline void overlay_update_message(Overlay *overlay, u32 fps, V2f player_pos)
{
    i32 bytes_written = sprintf(overlay->text, overlay->text_format,
                                game.width, game.height,
                                fps, player_pos.x, player_pos.y);

    ASSERT(bytes_written < OVERLAY_TEXT_SIZE);
}

internal void overlay_next_state()
{
    overlay.state = (overlay.state + 1) % _overlay_state_count;
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
    V2f old_pos = player.pos;

    player.pos = v2f_add(player.pos, v2f_scale(player.dir, dt * player.vel));
    if (is_perim(player.pos.x, player.pos.y) || is_wall(game.map_index, player.pos.x, player.pos.y)) {
        player.pos = old_pos;
    }
}

void player_move_backward(f64 dt)
{
    V2f old_pos = player.pos;

    player.pos = v2f_sub(player.pos, v2f_scale(player.dir, dt * player.vel));
    if (is_perim(player.pos.x, player.pos.y) || is_wall(game.map_index, player.pos.x, player.pos.y)) {
        player.pos = old_pos;
    }
}

void player_rotate_clockwise(f64 dt)
{
    player.dir = v2f_rotate(player.dir, radians_from_degrees(player.rotation_vel) * dt);
}

void player_rotate_counterclockwise(f64 dt)
{
    player.dir = v2f_rotate(player.dir, radians_from_degrees(-player.rotation_vel) * dt);
}

internal void draw_walls(Color color)
{
    for (u64 y = 0; y < game.height; y += CELL_SIZE) {
        for (u64 x = 0; x < game.width; x += CELL_SIZE) {
            Rect rect = make_rect(x, y, CELL_SIZE, CELL_SIZE);
            if (is_wall(game.map_index, x, y)) {
                platform_draw_rect(color, rect);
            } else if (is_texture(game.map_index, x, y)) {
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

// TODO: make this function faster as
// it takes up much of the frame time
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

    Intersect intersect = {0};

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

        V2f map_pos = v2f_scale(map, CELL_SIZE);
        if (is_perim(map_pos.x, map_pos.y)) {
            intersect.perim = true;
            break;
        }
        if (is_wall(game.map_index, map_pos.x, map_pos.y)) {
            intersect.map_tile_kind = MAP_TILE_WALL;
            break;
        }
        if (is_texture(game.map_index, map_pos.x, map_pos.y)) {
            intersect.map_tile_kind = MAP_TILE_TEXTURE;
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
    platform_draw_line(color,
                       make_v2f(WINDOW_CENTER_X, WINDOW_CENTER_Y - l),
                       make_v2f(WINDOW_CENTER_X, WINDOW_CENTER_Y + l));
    platform_draw_line(color,
                       make_v2f(WINDOW_CENTER_X - l, WINDOW_CENTER_Y),
                       make_v2f(WINDOW_CENTER_X + l, WINDOW_CENTER_Y));
}

internal inline Color color_from_u32(u32 color)
{
    return (Color) {
        .b = color >> (8*0) & 0xFF,
        .g = color >> (8*1) & 0xFF,
        .r = color >> (8*2) & 0xFF,
        .a = color >> (8*3) & 0xFF,
    };
}

internal inline u32 u32_from_color(Color color)
{
    // u32 color format: 0xAARRGGBB
    return (u32)color.b << (8*0) |
           (u32)color.g << (8*1) |
           (u32)color.r << (8*2) |
           (u32)color.a << (8*3);
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

        switch (intersect.map_tile_kind) {
            case MAP_TILE_EMPTY:
            case MAP_TILE_WALL: {
                f32 wall_bottom = CLAMP(( wall_height / 2.0f) + (game.height / 2.0f), 0.0f, game.height - 1.0f);
                V2f wall_end = make_v2f(x, wall_bottom);
                platform_draw_line(wall_color, wall_start, wall_end);
            } break;

            // TODO: make this case faster as
            // it takes up much of the frame time
            //
            // rotate texture matrix and switch x and y when selecting pixel?
            case MAP_TILE_TEXTURE: {
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

                    // FIXME: pixels are being pulled out so that the texture is rotated 90 degrees
                    // Solution might be to switch x and why in the loop
                    //
                    // DONE(?)
                    // u32 pixel = game.texture.data[game.texture.height * (u32)texture_index.x + (u32)texture_index.y];
                    u32 pixel = game.texture.data[game.texture.height * (u32)texture_index.y + (u32)texture_index.x];
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

internal void draw_minimap(Player player)
{
    f64 scale = (f64)game.minimap_dims.w / game.width;

    platform_draw_rect(green, game.minimap_dims);

    draw_minimap_walls(gray, scale);
    draw_minimap_grid(black, scale);

    V2f player_minimap_pos = v2f_add(v2f_scale(player.pos, scale), make_v2f(game.minimap_dims.x, game.minimap_dims.y));
    platform_draw_circle(blue, player_minimap_pos, 2, true);
}

internal void draw_box(Box box)
{
    switch (box.style) {
        default: {} break;

        case BOX_STYLE_SQUARED: {
            platform_draw_rect(box.bg, box.rect);
        } break;

        case BOX_STYLE_ROUNDED: {
            platform_draw_rect_rounded(box.bg, box.rect, 20);
        } break;
    }

    Rect text_rect = platform_center_text_in_rect(box.rect, box.ptsize, box.text);
    platform_draw_text(box.fg, box.bg, text_rect, box.text, box.ptsize);
}

internal void draw_menu()
{
    for (u64 i = 0; i < ARRAY_COUNT(game.menu.buttons.v); i++) {
        draw_box(game.menu.buttons.v[i].box);
    }
}

internal void draw_overlay(Overlay overlay)
{
    V2f dims = platform_get_text_dims(overlay.ptsize, overlay.text);
    overlay.rect = make_rect(0, 0, dims.x, dims.y);
    platform_draw_text(white, black_transparent, overlay.rect, overlay.text, overlay.ptsize);
}

void game_init()
{
    game.quit = false;

    game.width = WINDOW_WIDTH;
    game.height = WINDOW_HEIGHT;

    game.minimap_dims = (Rect) {
        .x = game.width * ((f32)3/4),
        .y = 0,
        .h = game.height * ((f32)1/4),
        .w = game.width * ((f32)1/4)
    };

    game.mouse_state = (Mouse_State){0};
    game.keyboard_state = (Keyboard_State){0};

    // Check that right amount of keys are in union
    ASSERT(&game.keyboard_state.key_end - &game.keyboard_state.v[0] == ARRAY_COUNT(game.keyboard_state.v));

    game.map_index = 0;

    game.view = VIEW_MENU;
    game.show_crosshair = true;
    game.crosshair_color = white;
    game.quit = false;

    game.texture = load_texture_from_file(TEXTURES_PATH"redbrick.png");

    game.menu.buttons = create_menu_buttons();
    game.menu.bg = gray;
}

internal void game_view_toggle_map(void)
{
    if (game.view == VIEW_GAME || game.view == VIEW_MENU) {
        game.view = VIEW_MAP;
    } else if (game.view == VIEW_MAP) {
        game.view = VIEW_GAME;
    }
}

internal void game_view_toggle_menu(void)
{
    // TODO: Play sound when entering menu
    if (game.view == VIEW_GAME || game.view == VIEW_MAP) {
        game.view = VIEW_MENU;
    } else if (game.view == VIEW_MENU) {
        game.view = VIEW_GAME;
    }
}

internal void game_toggle_crosshair(void)
{
    game.show_crosshair = !game.show_crosshair;
}

// NOTE: returns false if quit button is pressed
void game_process_mouse(void)
{
    for (u64 i = 0; i < ARRAY_COUNT(game.menu.buttons.v); i++) {
        Button *button = &game.menu.buttons.v[i];

        b32 was_hovered = button->hovered;
        b32 is_hovered = game.view == VIEW_MENU && // NOTE: Buttons can only be hovered if they are in view
                         platform_point_in_rect(game.mouse_state.pos, button->box.rect);

        // Only update cursor if its state has changed
        if (was_hovered != is_hovered) {
            button->hovered = is_hovered;
            game.mouse_state.active_cursor = button->hovered ? CURSOR_KIND_HAND : CURSOR_KIND_ARROW;
            platform_set_cursor(game.mouse_state.active_cursor);
        }

        button->pressed = button->hovered && game.mouse_state.key_left.is_down;
    }

    if (game.menu.buttons.start.pressed) {
        game_view_toggle_menu();
    }
    if (game.menu.buttons.quit.pressed) {
        game.quit = true;
    }
}

internal void game_handle_pressed_keys(f64 dt)
{
    if (game.keyboard_state.key_o.is_down && !game.keyboard_state.key_o.was_down) {
        overlay_next_state();
    }
    if (game.keyboard_state.key_m.is_down && !game.keyboard_state.key_m.was_down) {
        game_view_toggle_map();
    }
    if (game.keyboard_state.key_escape.is_down && !game.keyboard_state.key_escape.was_down) {
        game_view_toggle_menu();
    }

    //

    if (game.view != VIEW_MENU) {
        if (game.keyboard_state.key_n.is_down && !game.keyboard_state.key_n.was_down) {
            next_map_index();
        }
        if (game.keyboard_state.key_c.is_down && !game.keyboard_state.key_c.was_down) {
            game_toggle_crosshair();
        }

        //

        if (game.keyboard_state.key_w.active) {
            player_move_forward(dt);
        }
        if (game.keyboard_state.key_a.active) {
            player_rotate_counterclockwise(dt);
        }
        if (game.keyboard_state.key_s.active) {
            player_move_backward(dt);
        }
        if (game.keyboard_state.key_d.active) {
            player_rotate_clockwise(dt);
        }
    }
}

void game_render(f64 dt)
{
    game_process_mouse();
    game_handle_pressed_keys(dt);

    switch (game.view) {
        default: {} break;

        case VIEW_GAME: {
            platform_clear_backbuffer(green);
            draw_3d_view(player);
            draw_minimap(player);
        } break;

        case VIEW_MAP: {
            platform_clear_backbuffer(green);
            draw_map(player);
        } break;

        case VIEW_MENU: {
            platform_clear_backbuffer(game.menu.bg);
            draw_menu();
        } break;
    }

    switch (overlay.state) {
       default: {} break;

       case OVERLAY_STATE_HIDDEN: {} break;

       case OVERLAY_STATE_SHOWN: {
           u32 fps = (u32)(1.0f / dt);
           overlay_update_message(&overlay, fps, player.pos);
           draw_overlay(overlay);
           // NOTE: can change to wrapped font rendering if msg gets too long
       } break;
    }
}

