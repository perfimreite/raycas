#include "map.h"

internal i32 map[MAP_COUNT][ROWS][COLS] = {
    {
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

i32 next_map_index(i32 map_index)
{
    return (map_index + 1) % MAP_COUNT;
}

Map_Square_Kind get_map_square(i32 map_index, f32 x, f32 y)
{
    i32 cy = y / CELL_SIZE;
    i32 cx = x / CELL_SIZE;
    assert(cy >= 0);
    assert(cx >= 0);
    return map[map_index][cy][cx];
}

bool is_wall(i32 map_index, f32 x, f32 y)
{
    return get_map_square(map_index, x, y) == WALL;
}

bool is_perim(f32 x, f32 y)
{
    return (0.0f > y || y >= WINDOW_HEIGHT) || (0.0f > x || x >= WINDOW_WIDTH);
}

Map_Square_Kind get_map_square_by_tile(i32 map_index, i32 x, i32 y)
{
    assert(y >= 0);
    assert(x >= 0);
    return map[map_index][y][x];
}

bool is_wall_tile(i32 map_index, i32 x, i32 y)
{
    return get_map_square_by_tile(map_index, x, y) == WALL;
}

bool is_perim_tile(i32 x, i32 y)
{
    return (0 > y || y >= ROWS) || (0 > x || x >= COLS);
}

