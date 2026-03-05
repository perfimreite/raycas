#include "map.h"

i32 get_map_square(i32 x, i32 y)
{
    return map[y/CELL_SIZE][x/CELL_SIZE];
}

bool is_wall(i32 x, i32 y)
{
    return get_map_square(x, y) == WALL;
}

