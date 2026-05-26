#include "map.h"

global i32 map[ROWS][COLS] = {
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0},
    {1, 1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 1, 1, 1, 0, 0},
    {0, 1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 1, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0},
    {0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1},
    {0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 1, 0, 0, 0, 1},
    {0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 1},
};

// FIXME: indexing into an array without checking max size.

i32 get_map_square(i32 x, i32 y)
{
    return map[y/(i32)CELL_SIZE][x/(i32)CELL_SIZE];
}

bool is_wall(i32 x, i32 y)
{
    return get_map_square(x, y) == WALL;
}

bool is_perim(i32 x, i32 y)
{
    return (0 > y || y >= WINDOW_HEIGHT) || (0 > x || x >= WINDOW_WIDTH);
}

i32 get_map_square_by_tile(i32 x, i32 y)
{
    return map[y][x];
}

bool is_wall_tile(i32 x, i32 y)
{
    return get_map_square_by_tile(x, y) == WALL;
}

bool is_perim_tile(i32 x, i32 y)
{
    return (0 > y || y >= ROWS) || (0 > x || x >= COLS);
}

