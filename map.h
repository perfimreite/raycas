#include "constants.h"
#include "meta.h"

#include <stdbool.h>

#ifndef MAP_H
#define MAP_H

#define EMPTY 0
#define WALL  1

i32 get_map_square(i32 x, i32 y);
bool is_wall(i32 x, i32 y);
bool is_perim(i32 x, i32 y);

i32 get_map_square_by_tile(i32 x, i32 y);
bool is_wall_tile(i32 x, i32 y);
bool is_perim_tile(i32 x, i32 y);

#endif // MAP_H
