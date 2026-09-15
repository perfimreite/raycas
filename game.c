#include "game.h"
#include "utils.h"

#if defined(_WIN32)
    #define _USE_MATH_DEFINES
#endif
#include <math.h>
#ifndef M_PI
    #define M_PI_2 1.57079632679489661923
#endif

#define STB_IMAGE_IMPLEMENTATION
#include "thirdparty/stb_image.h"

#include <stdio.h>
#include <float.h>

global const Color black             = { .r = 0  , .g = 0  , .b = 0  , .a = 255 };
global const Color white             = { .r = 255, .g = 255, .b = 255, .a = 255 };
global const Color light_gray        = { .r = 127, .g = 127, .b = 127, .a = 255 };
global const Color gray              = { .r = 63 , .g = 63 , .b = 63 , .a = 255 };
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
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 1},
        {1, 3, 0, 0, 0, 0, 2, 2, 0, 0, 0, 1, 1, 1, 0, 1},
        {1, 3, 0, 0, 0, 0, 2, 2, 0, 0, 0, 0, 0, 0, 0, 1},
        {1, 3, 0, 0, 0, 0, 5, 2, 0, 0, 0, 0, 0, 0, 0, 1},
        {1, 3, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 1},
        {1, 3, 0, 0, 0, 0, 0, 2, 2, 0, 0, 0, 0, 0, 0, 1},
        {1, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
        {1, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
        {1, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1},
        {1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 1, 0, 0, 0, 1},
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    },
    {
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 1},
        {1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
        {1, 1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1},
        {1, 1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1},
        {1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1},
        {1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1},
        {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
        {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
        {1, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1},
        {1, 0, 0, 1, 1, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 1},
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    }
};

internal void next_map_index(void)
{
    game.map_index = (game.map_index + 1) % MAP_COUNT;
}

internal u32 get_map_tile(f32 x, f32 y)
{
    i32 cy = y / CELL_SIZE;
    i32 cx = x / CELL_SIZE;
    ASSERT(cy >= 0);
    ASSERT(cx >= 0);
    return map[game.map_index][cy][cx];
}

internal b32 is_wall(f32 x, f32 y)
{
    return get_map_tile(x, y) >= 1;
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

internal Texture get_fallback_texture(void)
{
    Texture texture = {0};
    texture.width = DEFAULT_TEXTURE_WIDTH;
    texture.height = DEFAULT_TEXTURE_HEIGHT;
    texture.data = malloc(sizeof(u32) * DEFAULT_TEXTURE_WIDTH * DEFAULT_TEXTURE_HEIGHT);
    texture.name = "fallback";

    for (u32 y = 0; y < texture.height; y++) {
        for (u32 x = 0; x < texture.width; x++) {
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

internal char *get_file_name_from_path(const char *path)
{
    u32 len = strlen(path);
    char *file_name = malloc(len + 1);

    u32 j = 0;
    for (u32 i = 0; i < len; i++) {
        if (path[i] == '/') {
            file_name = &file_name[j+1];
            j = 0;
            continue;
        } else if (path[i] == '.') {
            for (u32 k = i + 1; k < len; k++) {
                if (path[k] == '/') {
                    continue;
                }
            }
            break;
        }
        file_name[j++] = path[i];
    }

    file_name[j] = '\0';
    return file_name;
}

internal u32 get_average_from_pixels(u32 pixels[], u32 size)
{
    u32 b = 0;
    u32 g = 0;
    u32 r = 0;
    u32 a = 0;

    for (u32 i = 0; i < size; i++) {
        b += pixels[i] >> (8*0) & 0xFF;
        g += pixels[i] >> (8*1) & 0xFF;
        r += pixels[i] >> (8*2) & 0xFF;
        a += pixels[i] >> (8*3) & 0xFF;
    }

    b /= size;
    g /= size;
    r /= size;
    a /= size;

    u32 color = b << (8*0) | g << (8*1) | r << (8*2) | a << (8*3);

    return color;
}

internal Texture load_texture_from_file(const char *file_path)
{
    local_persist int id = 2;
    Texture texture = {0};

    FILE *file = fopen(file_path, "rb");

    if (file != NULL) {
        i32 width, height;
        u32 *data = (u32 *)stbi_load_from_file(file, &width, &height, NULL, STBI_rgb_alpha);

        if (data != NULL) {
            texture.data = malloc(sizeof(u32) * width * height);
            texture.width = width;
            texture.height = height;
            texture.name = get_file_name_from_path(file_path);

            for (u32 y = 0; y < texture.height; y++) {
                for (u32 x = 0; x < texture.width; x++) {
                    // Swap blue and red bytes to match color interpretation of `Color`
                    texture.data[texture.height * x + y] =
                        ( data[texture.width * y + x] & 0xFF00FF00) |
                        ((data[texture.width * y + x] & 0x00FF0000) >> 16) |
                        ((data[texture.width * y + x] & 0x000000FF) << 16);
                }
            }
        } else {
            fprintf(stderr, "ERROR: Failed to read texture file: `%s`\n", file_path);
            texture = get_fallback_texture();
        }

        fclose(file);
    } else {
        fprintf(stderr, "ERROR: Failed to open texture file: `%s`\n", file_path);
        texture = get_fallback_texture();
    }
    texture.id = id++;
    texture.tile_color = color_from_u32(get_average_from_pixels(texture.data, texture.width * texture.height));

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

internal Rect rect_shrink(Rect rect, i32 k)
{
    return make_rect(rect.x + k, rect.y + k, rect.w - 2 * k, rect.h - 2 * k);
}

internal Box make_box(const char *text, i32 ptsize, Rect rect,
                      Box_Style_Flag box_style_flag, Color fg, Color bg,
                      Color border_color, i32 border_radius, i32 border_thickness)
{
    return (Box) {
        .text = text,
        .ptsize = ptsize,
        .rect = rect,
        .style_flag = box_style_flag,
        .fg = fg,
        .bg = bg,
        .border_color = border_color,
        .border_radius = border_radius,
        .border_thickness = border_thickness
    };
}

internal Button make_button(const char *text, i32 ptsize, Rect rect,
                            Box_Style_Flag box_style_flag, Color fg, Color bg,
                            Color border_color, i32 border_radius, i32 border_thickness)
{
    return (Button) {
        .pressed = false,
        .box = make_box(text, ptsize, rect, box_style_flag,
                        fg, bg, border_color, border_radius, border_thickness)
    };
}

internal Menu create_menu(void)
{
    Menu menu = {0};
    menu.bg = light_gray;

    Button start = make_button("START", 32,
                               make_rect(WINDOW_CENTER_X - 100, WINDOW_CENTER_Y - 120, 200, 80),
                               BOX_STYLE_FLAG_ROUNDED | BOX_STYLE_FLAG_BORDER,
                               white, blue, black, 16, 4);
    Button quit = make_button("QUIT", 32,
                              make_rect(WINDOW_CENTER_X - 100, WINDOW_CENTER_Y + 40, 200, 80),
                              BOX_STYLE_FLAG_ROUNDED | BOX_STYLE_FLAG_BORDER,
                              white, red, black, 16, 4);
    menu.buttons.v[0] = start;
    menu.buttons.v[1] = quit;

    // Check that right amount of buttons are in union
    ASSERT(&menu.buttons.button_end - &menu.buttons.v[0] == ARRAY_COUNT(menu.buttons.v));

    return menu;
}

void overlay_init(void)
{
    overlay.state = OVERLAY_STATE_HIDDEN;
    overlay.text_format = "RESOLUTION:%dx%d FPS:%d AVERAGE_FPS:%.1lf POS:(%.1f, %.1f)";
    overlay.ptsize = 16;
}

internal inline void overlay_update_message(Overlay *overlay, u32 fps, f64 frame_average, V2f player_pos)
{
    i32 bytes_written = sprintf(overlay->text, overlay->text_format,
                                game.width, game.height,
                                fps, frame_average, player_pos.x, player_pos.y);

    ASSERT(bytes_written < OVERLAY_TEXT_SIZE);
}

internal void overlay_next_state(void)
{
    overlay.state = (overlay.state + 1) % _overlay_state_count;
}

void player_init(void)
{
    player.pos = v2f(200.0f, 200.0f);
    player.dir = v2f(1.0f, 0.0f);
    player.vel = 10000 / CELL_SIZE;
    player.rotation_vel = 200;
    player.fov = M_PI_2;
    player.angle_step = player.fov / (game.width - 1); 
    player.fov_color = black_transparent;
    player.radius = 6;
    player.color = blue;
}

void player_move_forward(f64 dt)
{
    V2f old_pos = player.pos;

    player.pos = v2f_add(player.pos, v2f_scale(player.dir, dt * player.vel));
    if (is_wall(player.pos.x, player.pos.y)) {
        player.pos = old_pos;
    }
}

void player_move_backward(f64 dt)
{
    V2f old_pos = player.pos;

    player.pos = v2f_sub(player.pos, v2f_scale(player.dir, dt * player.vel));
    if (is_wall(player.pos.x, player.pos.y)) {
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

internal inline Texture *get_texture_from_mtv(u32 mtv)
{
    u32 index = mtv - FIRST_TEXTURE_ID;
    return &LIST_GET(game.textures, index);
}

internal Intersect get_intersect(V2f pos, V2f dir)
{
    V2f delta_dist = v2f(0, 0);
    V2f step       = v2f(0, 0);
    V2f side_dist  = v2f(0, 0);

    delta_dist.x = dir.x == 0 ? FLT_MAX : fabs(CELL_SIZE / dir.x);
    delta_dist.y = dir.y == 0 ? FLT_MAX : fabs(CELL_SIZE / dir.y);

    V2f map_tile = v2f_scale(v2f_floor(v2f_scale(pos, 1.0f / CELL_SIZE)), CELL_SIZE);

    if (dir.x < 0) {
        step.x = -1;
        side_dist.x = (pos.x - map_tile.x);
    } else {
        step.x = 1;
        side_dist.x = (map_tile.x + CELL_SIZE - pos.x);
    }
    if (dir.y < 0) {
        step.y = -1;
        side_dist.y = (pos.y - map_tile.y);
    } else {
        step.y = 1;
        side_dist.y = (map_tile.y + CELL_SIZE - pos.y);
    }

    side_dist = v2f_mul(side_dist, v2f_scale(delta_dist, 1.0f / CELL_SIZE));

    Intersect intersect = {0};

    for (;;) {
        if (side_dist.x < side_dist.y) {
            side_dist.x += delta_dist.x;
            map_tile.x += step.x * CELL_SIZE;
            intersect.horizontal = false;
        } else {
            side_dist.y += delta_dist.y;
            map_tile.y += step.y * CELL_SIZE;
            intersect.horizontal = true;
        }

        if (is_wall(map_tile.x, map_tile.y)) {
            intersect.map_tile_value = get_map_tile(map_tile.x, map_tile.y);
            break;
        }
    }

    intersect.perp_wall_dist = (intersect.horizontal ?
                                side_dist.y - delta_dist.y :
                                side_dist.x - delta_dist.x);
    intersect.pos = v2f_add(player.pos, v2f_scale(dir, intersect.perp_wall_dist));
    return intersect;
}

internal void draw_crosshair(Color color)
{
    i32 l = 5;
    platform_draw_line(color,
                       v2f(WINDOW_CENTER_X, WINDOW_CENTER_Y - l),
                       v2f(WINDOW_CENTER_X, WINDOW_CENTER_Y + l));
    platform_draw_line(color,
                       v2f(WINDOW_CENTER_X - l, WINDOW_CENTER_Y),
                       v2f(WINDOW_CENTER_X + l, WINDOW_CENTER_Y));
}

internal void draw_3d_view(Player player)
{
    f32 angle_curr  = -player.fov / 2.0f;
    for (u32 x = 0; x < game.width; x++) {
        V2f ray = v2f_rotate(player.dir, angle_curr);
        Intersect intersect = get_intersect(player.pos, ray);
        f32 rel_perp_wall_dist = intersect.perp_wall_dist / CELL_SIZE;

        f32 wall_height = game.height / rel_perp_wall_dist;
        f32 wall_top = CLAMP((-wall_height / 2.0f) + (game.height / 2.0f), 0.0f, game.height - 1.0f);
        V2f window_start = v2f(x, 0);
        V2f wall_start = v2f(x, wall_top);
        platform_draw_line(light_blue, window_start, wall_start);

        if (intersect.map_tile_value == 1) {
            f32 wall_bottom = CLAMP((wall_height / 2.0f) + (game.height / 2.0f), 0.0f, game.height - 1.0f);
            V2f wall_end = v2f(x, wall_bottom);
            Color wall_color = intersect.horizontal ? light_gray : gray;
            platform_draw_line(wall_color, wall_start, wall_end);
        } else {
            Texture *texture = get_texture_from_mtv(intersect.map_tile_value);

            f32 wall_x = intersect.horizontal ?
                player.pos.x / CELL_SIZE + rel_perp_wall_dist * ray.x :
                player.pos.y / CELL_SIZE + rel_perp_wall_dist * ray.y;
            wall_x -= floor(wall_x);
            V2f texture_index = v2f(0, 0);

            texture_index.x = (i32)(wall_x * texture->width);
            if ((!intersect.horizontal && ray.x > 0) || (intersect.horizontal && ray.y < 0)) {
                texture_index.x = texture->width - texture_index.x - 1;
            }

            f32 step = texture->height / wall_height;
            f32 texture_pos = (wall_top - game.height / 2.0 + wall_height / 2.0) * step;

            for (u32 y = wall_top; y < wall_height + wall_top; y++, texture_pos += step) {
                texture_index.y = (i32)texture_pos & (texture->height - 1);

                u32 pixel = texture->data[(i32)(texture->height * texture_index.x + texture_index.y)];
                if (intersect.horizontal) {
                    // NOTE: `&` with binary number to set last
                    // bit of every byte to zero after the `>>`
                    pixel = (pixel >> 1) & 8355711; // 0b011111110111111101111111
                }

                platform_draw_point(color_from_u32(pixel), v2f(x, y));
            }
        }

        angle_curr += player.angle_step;
    }

    if (game.show_crosshair) {
        draw_crosshair(game.crosshair_color);
    }
}

internal void draw_map_walls()
{
    for (u32 y = 0; y < game.height; y += CELL_SIZE) {
        for (u32 x = 0; x < game.width; x += CELL_SIZE) {
            Rect rect = make_rect(x, y, CELL_SIZE, CELL_SIZE);
            if (is_wall(x, y)) {
                u32 map_tile_value = get_map_tile(x, y);
                if (map_tile_value == 1) {
                    platform_draw_rect(light_gray, rect);
                } else {
                    Texture *texture = get_texture_from_mtv(map_tile_value);
                    platform_draw_rect(texture->tile_color, rect);
                }
            }
        }
    }
}

internal void draw_map_grid()
{
    for (u32 y = CELL_SIZE; y < game.height; y += CELL_SIZE) {
        platform_draw_line(black, v2f(0, y), v2f(game.width, y));
    }

    for (u32 x = CELL_SIZE; x < game.width; x += CELL_SIZE) {
        platform_draw_line(black, v2f(x, 0), v2f(x, game.height));
    }
}

internal void draw_map_player(u32 beam_spread)
{
    platform_draw_circle_filled(player.color, player.pos, player.radius);

#if 1 // Look at player fov
    f32 angle_curr  = -player.fov / 2.0f;
    f32 angle_end   =  player.fov / 2.0f;
    f32 angle_step  =  player.angle_step * beam_spread;
    for (; angle_curr <= angle_end; angle_curr += angle_step) {
        V2f curr_dir = v2f_rotate(player.dir, angle_curr);
        Intersect intersect = get_intersect(player.pos, curr_dir);
        platform_draw_line(player.fov_color, player.pos, intersect.pos);
    }
#else
    (void)beam_spread;
#endif
}

internal void draw_map()
{
    draw_map_walls();
    draw_map_grid();
    draw_map_player(8);
}

internal void draw_minimap_walls(f32 scale)
{
    f32 scaled_cell_size = CELL_SIZE * scale;

    for (u32 y = game.minimap_dims.y; y < game.minimap_dims.y + game.minimap_dims.h; y += scaled_cell_size) {
        for (u32 x = game.minimap_dims.x; x < game.minimap_dims.x + game.minimap_dims.w; x += scaled_cell_size) {
            if (is_wall((x - game.minimap_dims.x) / scale, (y - game.minimap_dims.y) / scale)) {
                Rect rect = make_rect(x, y, scaled_cell_size, scaled_cell_size);
                u32 map_tile_value =
                    get_map_tile((x - game.minimap_dims.x) * CELL_SIZE / scaled_cell_size,
                                 (y - game.minimap_dims.y) * CELL_SIZE / scaled_cell_size);
                if (map_tile_value == 1) {
                    platform_draw_rect(light_gray, rect);
                } else {
                    Texture *texture = get_texture_from_mtv(map_tile_value);
                    platform_draw_rect(texture->tile_color, rect);
                }
            }
        }
    }
}

internal void draw_minimap_grid(f32 scale)
{
    f32 scaled_cell_size = CELL_SIZE * scale;

    for (u32 y = game.minimap_dims.y + scaled_cell_size; y < game.minimap_dims.y + game.minimap_dims.h; y += scaled_cell_size) {
        platform_draw_line(black, v2f(game.minimap_dims.x, y), v2f(game.minimap_dims.x + game.minimap_dims.w, y));
    }

    for (u32 x = game.minimap_dims.x + scaled_cell_size ; x < game.minimap_dims.x + game.minimap_dims.w; x += scaled_cell_size) {
        platform_draw_line(black, v2f(x, game.minimap_dims.y), v2f(x, game.minimap_dims.y + game.minimap_dims.h));
    }
}

internal void draw_minimap_player(f32 scale)
{
    V2f player_minimap_pos = v2f_add(v2f_scale(player.pos, scale), v2f(game.minimap_dims.x, game.minimap_dims.y));
    platform_draw_circle_filled(blue, player_minimap_pos, 2);
}

internal void draw_minimap()
{
    platform_draw_rect(green, game.minimap_dims);

    f32 scale = (f32)game.minimap_dims.w / game.width;
    draw_minimap_walls(scale);
    draw_minimap_grid(scale);
    draw_minimap_player(scale);
}

internal void draw_box(Box box)
{
    if (box.style_flag & BOX_STYLE_FLAG_BORDER) {
        if (box.style_flag & BOX_STYLE_FLAG_ROUNDED) {
            platform_draw_rect_rounded(box.border_color, box.rect, box.border_radius);
            platform_draw_rect_rounded(box.bg, rect_shrink(box.rect, 4), box.border_radius);
        } else {
            platform_draw_rect(box.border_color, box.rect);
            platform_draw_rect(box.bg, rect_shrink(box.rect, 4));
        }
    } else {
        if (box.style_flag & BOX_STYLE_FLAG_ROUNDED) {
            platform_draw_rect_rounded(box.bg, box.rect, box.border_radius);
        } else {
            platform_draw_rect(box.bg, box.rect);
        }
    }

    if (box.text != NULL && strcmp(box.text, "") != 0) {
        Rect text_rect = platform_center_text_in_rect(box.rect, box.ptsize, box.text);
        platform_draw_text(box.fg, box.bg, text_rect, box.text, box.ptsize);
    }
}

internal void draw_menu(void)
{
    for (u32 i = 0; i < ARRAY_COUNT(game.menu.buttons.v); i++) {
        draw_box(game.menu.buttons.v[i].box);
    }
}

internal void draw_overlay(Overlay overlay)
{
    V2f dims = platform_get_text_dims(overlay.ptsize, overlay.text);
    overlay.rect = make_rect(0, 0, dims.x, dims.y);
    platform_draw_text(white, black_transparent, overlay.rect, overlay.text, overlay.ptsize);
}

void game_init(void)
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

    game.textures = (Textures){0};

    // LIST_PUSH(game.textures, load_texture_from_file("assets/wall.png"));
    LIST_PUSH(game.textures, load_texture_from_file(TEXTURES_PATH"redbrick.png"));
    LIST_PUSH(game.textures, load_texture_from_file(TEXTURES_PATH"bluestone.png"));
    LIST_PUSH(game.textures, load_texture_from_file(TEXTURES_PATH"colorstone.png"));
    LIST_PUSH(game.textures, load_texture_from_file(TEXTURES_PATH"eagle.png"));
    LIST_PUSH(game.textures, load_texture_from_file(TEXTURES_PATH"greystone.png"));
    LIST_PUSH(game.textures, load_texture_from_file(TEXTURES_PATH"mossy.png"));
    LIST_PUSH(game.textures, load_texture_from_file(TEXTURES_PATH"purplestone.png"));
    LIST_PUSH(game.textures, load_texture_from_file(TEXTURES_PATH"wood.png"));

    // LIST_PUSH(game.textures, load_texture_from_file(TEXTURES_PATH"greenlight.png"));
    // LIST_PUSH(game.textures, load_texture_from_file(TEXTURES_PATH"barrel.png"));
    // LIST_PUSH(game.textures, load_texture_from_file(TEXTURES_PATH"pillar.png"));

    game.menu = create_menu();
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

internal inline void game_toggle_crosshair(void)
{
    game.show_crosshair = !game.show_crosshair;
}

void game_process_mouse(void)
{
    for (u32 i = 0; i < ARRAY_COUNT(game.menu.buttons.v); i++) {
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

void game_update(f64 dt)
{
    game_process_mouse();
    game_handle_pressed_keys(dt);
}

void game_render(f64 dt)
{
    switch (game.view) {
        default: {} break;

        case VIEW_GAME: {
            platform_clear_backbuffer(green);
            draw_3d_view(player);
            draw_minimap();
        } break;

        case VIEW_MAP: {
            platform_clear_backbuffer(green);
            draw_map();
        } break;

        case VIEW_MENU: {
            platform_clear_backbuffer(game.menu.bg);
            draw_menu();
        } break;
    }

    local_persist f64 elapsed_time = 0.0;
    local_persist i32 secs = 1;
    local_persist f64 frames = 0;
    local_persist f64 frame_average = 0;

    elapsed_time += dt;
    frames += 1;

    switch (overlay.state) {
        default: {} break;

        case OVERLAY_STATE_HIDDEN: {} break;

        case OVERLAY_STATE_SHOWN: {
            if (elapsed_time >= secs) {
                frame_average = frames / elapsed_time;
#if 0 // Get average FPS for one second at a time
                elapsed_time = 0.0;
                frames = 0.0;
#else // Get average FPS for the whole run time of the game
                secs++;
#endif
            }
            u32 fps = (u32)(1.0 / dt);
            overlay_update_message(&overlay, fps, frame_average, player.pos);
            draw_overlay(overlay);
            // NOTE: can change to wrapped font rendering if msg gets too long
        } break;
    }
}

