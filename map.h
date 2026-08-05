#include "base.h"
#include "vector.h"

#ifndef MAP_H
#define MAP_H

typedef enum {
    MAP_TILE_EMPTY,
    MAP_TILE_WALL,
    MAP_TILE_TEXTURE
} Map_Tile_Kind;

i32 next_map_index(i32 map_index);

Map_Tile_Kind get_map_tile_kind(i32 map_index, f32 x, f32 y);
b32 is_wall(i32 map_index, f32 x, f32 y);
b32 is_texture(i32 map_index, f32 x, f32 y);
b32 is_perim(f32 x, f32 y);

// Map_Tile_Kind get_map_kind_by_pos(i32 map_index, i32 x, i32 y);
// b32 is_wall_tile(i32 map_index, i32 x, i32 y);
// b32 is_texture_tile(i32 map_index, i32 x, i32 y);
// b32 is_perim_tile(i32 x, i32 y);

#endif // MAP_H
