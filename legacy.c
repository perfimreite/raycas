/*
 * NOTE: this file contains code that is no longer in use.
 * This file will not compile successfully
 * It is stored to look back upon and may be brought back for
 * learning or debug purpouses
 */

// NOTE: old approach checks every pixle in a line following player.dir
// this is very slow...

// internal Intersect find_intersect(V2f a, V2f b)
// {
//     V2f u = v2f_sub(b, a);
//     f32 l = v2f_len(u);
//
//     for (i32 i = 0;; i++) {
//         f32 t = i / l;
//         f32 x = a.x + t * u.x;
//         f32 y = a.y + t * u.y;
//
//         V2f pos = make_v2f(x, y);
//         Intersect intersect = { .pos = pos, .vertical = false, .wall = false };
//
//         // check for oob
//         if (0 > x || x >= WINDOW_WIDTH) {
//             intersect.vertical = true;
//             intersect.wall = true;
//             return intersect;
//         }
//         if (0 > y || y >= WINDOW_HEIGHT) {
//             intersect.wall = true;
//             return intersect;
//         }
//
//         if ((i32)x % (i32)CELL_SIZE == 0 && (is_wall(x, y) || (u.x < 0 && is_wall(x - 1, y)))) {
//             intersect.vertical = true;
//             return intersect;
//         }
//         if ((i32)y % (i32)CELL_SIZE == 0 && (is_wall(x, y) || (u.y < 0 && is_wall(x, y - 1)))) {
//             return intersect;
//         }
//     }
// }

// typedef struct {
//     V2f center;
//     V2f a;
//     V2f b;
//
//     u32 radius;
// } Camera;

// internal void draw_camera(Player *player, Camera *camera)
// {
//     draw_circle(black, camera->center, camera->radius);
//     draw_line(magenta, player->pos, camera->center, 1);
//     draw_line(blue, camera->a, camera->b, 1);
//     draw_line(red, player->pos, camera->a, 1);
//     draw_line(red, player->pos, camera->b, 1);
// }
//
// {
//     V2f relative_pos_to_player = v2f_scale(player.dir, CELL_SIZE);
//
//     camera.center = v2f_add(player.pos, relative_pos_to_player);
//     camera.a      = v2f_add(camera.center, v2f_scale(v2f_normal(relative_pos_to_player), -1));
//     camera.b      = v2f_add(camera.center, v2f_normal(relative_pos_to_player));
// }

// internal void draw_circle(SDL_Color color, V2f center, u32 radius)
// {
//     f32 acc = radius * 2.0f;
//     f32 radius_sq = radius * radius;
//
//     for (i32 y = center.y - radius; y <= center.y + radius; y++) {
//         for (i32 x = center.x - radius; x <= center.x + radius; x++) {
//             V2f u = make_v2f(x, y);
//             i32 l =  v2f_square_len(v2f_sub(u, center));
//
//             if (radius_sq - acc <= l && l <= radius_sq + acc) {
//                 draw_line(color, center, u, 0);
//             }
//         }
//     }
// }

// Map_Tile_Kind get_map_square_by_tile(i32 map_index, i32 x, i32 y)
// {
//     assert(y >= 0);
//     assert(x >= 0);
//     return map[map_index][y][x];
// }
//
// b32 is_wall_tile(i32 map_index, i32 x, i32 y)
// {
//     return get_map_square_by_tile(map_index, x, y) == MAP_TILE_WALL;
// }
//
// b32 is_texture_tile(i32 map_index, i32 x, i32 y)
// {
//     return get_map_square_by_tile(map_index, x, y) == MAP_TILE_TEXTURE;
// }
//
// b32 is_perim_tile(i32 x, i32 y)
// {
//     return (0 > y || y >= ROWS) || (0 > x || x >= COLS);
// }

// internal void draw_3d_view(Player player)
// {
//     f32 angle_curr  = -player.fov / 2.0f;
//     f32 angle_step  =  player.fov / (game.width - 1);
//     for (u32 x = 0; x < game.width; x++) {
//         V2f curr_dir = v2f_rotate(player.dir, angle_curr);
//         Intersect intersect = get_intersect(player.pos, curr_dir);
//         f32 wall_height = game.height / intersect.perp_wall_dist;
//
//         f32 wall_top = CLAMP((-wall_height / 2.0f) + (game.height / 2.0f), 0.0f, game.height - 1.0f);
//         V2f window_start = v2f(x, 0);
//         V2f wall_start = v2f(x, wall_top);
//         platform_draw_line(light_blue, window_start, wall_start);
//
//         if (intersect.map_tile_value == 1) {
//             f32 wall_bottom = CLAMP((wall_height / 2.0f) + (game.height / 2.0f), 0.0f, game.height - 1.0f);
//             V2f wall_end = v2f(x, wall_bottom);
//             Color wall_color = intersect.horizontal ? light_gray : gray;
//             platform_draw_line(wall_color, wall_start, wall_end);
//         } else {
//             Texture texture = get_texture_from_mtv(intersect.map_tile_value);
//
//             f32 wall_x = intersect.horizontal ?
//                 player.pos.x + intersect.perp_wall_dist * curr_dir.x :
//                 player.pos.y + intersect.perp_wall_dist * curr_dir.y;
//             wall_x -= floor(wall_x);
//             V2f texture_index = v2f(0, 0);
//
//             texture_index.x = texture.width - 1 - (i32)(wall_x * texture.width);
//             if ((!intersect.horizontal && curr_dir.x > 0) || (intersect.horizontal && curr_dir.y < 0)) {
//                 texture_index.x = texture.width - texture_index.x - 1;
//             }
//
//             f32 step = texture.height / wall_height;
//             f32 texture_pos = (wall_top - game.height / 2.0 + wall_height / 2.0) * step;
//
//             if (intersect.horizontal) {
//                 for (u32 y = wall_top; y < wall_height + wall_top; y++, texture_pos += step) {
//                     texture_index.y = (i32)texture_pos & (texture.height - 1);
//
//                     u32 pixel = texture.data[(i32)(texture.height * texture_index.x + texture_index.y)];
//                     pixel = (pixel >> 1) & 8355711;
//                     Color color = color_from_u32(pixel);
//
//                     platform_draw_point(color, v2f(x, y));
//                 }
//             } else {
//                 for (u32 y = wall_top; y < wall_height + wall_top; y++, texture_pos += step) {
//                     texture_index.y = (i32)texture_pos & (texture.height - 1);
//
//                     u32 pixel = texture.data[(i32)(texture.height * texture_index.x + texture_index.y)];
//                     Color color = color_from_u32(pixel);
//
//                     platform_draw_point(color, v2f(x, y));
//                 }
//             }
//         }
//
//         angle_curr += angle_step;
//     }
//
//     if (game.show_crosshair) {
//         draw_crosshair(game.crosshair_color);
//     }
// }
