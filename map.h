#include "constants.h"
#include "meta.h"

#include <stdbool.h>

#ifndef MAP_H
#define MAP_H

typedef enum {
    EMPTY,
    WALL,
    TEXTURE
} Map_Square_Kind;

i32 next_map_index(i32 map_index);

Map_Square_Kind get_map_square(i32 map_index, f32 x, f32 y);
bool is_wall(i32 map_index, f32 x, f32 y);
bool is_texture(i32 map_index, f32 x, f32 y);
bool is_perim(f32 x, f32 y);

Map_Square_Kind get_map_square_by_tile(i32 map_index, i32 x, i32 y);
bool is_wall_tile(i32 map_index, i32 x, i32 y);
bool is_texture_tile(i32 map_index, i32 x, i32 y);
bool is_perim_tile(i32 x, i32 y);

#endif // MAP_H
