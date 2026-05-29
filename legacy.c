/*
 * NOTE: this file contains code that is no longer in use.
 * This file will not compile successfully
 * It is stored to look back upon and may be brought back for
 * learning or debug purpouses
 */

// NOTE: old approach checks every pixle in a line following player.dir
// this is very slow...
internal Intersect find_intersect(V2f a, V2f b)
{
    V2f u = v2f_sub(b, a);
    f32 l = v2f_len(u);

    for (i32 i = 0;; i++) {
        f32 t = i / l;
        f32 x = a.x + t * u.x;
        f32 y = a.y + t * u.y;

        V2f pos = make_v2f(x, y);
        Intersect intersect = { .pos = pos, .vertical = false, .wall = false };

        // check for oob
        if (0 > x || x >= WINDOW_WIDTH) {
            intersect.vertical = true;
            intersect.wall = true;
            return intersect;
        }
        if (0 > y || y >= WINDOW_HEIGHT) {
            intersect.wall = true;
            return intersect;
        }

        if ((i32)x % (i32)CELL_SIZE == 0 && (is_wall(x, y) || (u.x < 0 && is_wall(x - 1, y)))) {
            intersect.vertical = true;
            return intersect;
        }
        if ((i32)y % (i32)CELL_SIZE == 0 && (is_wall(x, y) || (u.y < 0 && is_wall(x, y - 1)))) {
            return intersect;
        }
    }
}

typedef struct {
    V2f center;
    V2f a;
    V2f b;

    u32 radius;
} Camera;

internal void draw_camera(Player *player, Camera *camera)
{
    draw_circle(black, camera->center, camera->radius);
    draw_line(magenta, player->pos, camera->center, 1);
    draw_line(blue, camera->a, camera->b, 1);
    draw_line(red, player->pos, camera->a, 1);
    draw_line(red, player->pos, camera->b, 1);
}

{
    V2f relative_pos_to_player = v2f_scale(player.dir, CELL_SIZE);

    camera.center = v2f_add(player.pos, relative_pos_to_player);
    camera.a      = v2f_add(camera.center, v2f_scale(v2f_normal(relative_pos_to_player), -1));
    camera.b      = v2f_add(camera.center, v2f_normal(relative_pos_to_player));
}

internal void draw_circle(SDL_Color color, V2f center, u32 radius)
{
    f32 acc = radius * 2.0f;
    f32 radius_sq = radius * radius;

    for (i32 y = center.y - radius; y <= center.y + radius; y++) {
        for (i32 x = center.x - radius; x <= center.x + radius; x++) {
            V2f u = make_v2f(x, y);
            i32 l =  v2f_square_len(v2f_sub(u, center));

            if (radius_sq - acc <= l && l <= radius_sq + acc) {
                draw_line(color, center, u, 0);
            }
        }
    }
}

