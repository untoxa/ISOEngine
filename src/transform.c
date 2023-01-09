#pragma bank 2

#include <stdint.h>

#include "transform.h"
#include "shadow.h"
#include "scenes.h"

void swap(uint8_t * a, uint8_t * b) {
    uint8_t c = *a;
    *a = *b, *b = c;
}

void rorate_CCW(const scene_t * sour, INT8 z)
{
    for (INT8 i = 0; i < max_scene_y; i++)
        for (INT8 j = i; j < max_scene_x; j++)
            swap(&((*sour)[i][z][j]), &((*sour)[j][z][i]));
    for (INT8 i = 0; i < max_scene_x; i++)
        for (INT8 j = 0, k = max_scene_x - 1; j < k; j++, k--)
            swap(&((*sour)[j][z][i]), &((*sour)[k][z][i]));
}

void map_to_scene(const scene_t * sour, scene_item_t * dest) {
    static scene_item_t * dst;
    static uint8_t id, count;

    dst = dest;
    count = 1u;

    // zero item must always exist to simplify insertion of objects; it is not drawn
    dst->id = 0xffu, dst->x = 0u, dst->y = 0u, dst->n = 1u, dst->coords = 0u, dst->next = dst + 1;

    for (INT8 x = 0; x < max_scene_x; x++) {
        for (INT8 y = max_scene_y - 1; y >= 0; y--) {
            for (INT8 z = 0; z < max_scene_z; z++) {
                id = (*sour)[x][z][y];
                if ((id) && (id < 0xc0u)) {
                    dst++;
                    dst->id = id - 1;
                    dst->x = to_x(x, y, z), dst->y = to_y(x, y, z);
                    dst->n = ++count;
                    dst->coords = to_coords(x, y, z);
                    dst->next = dst + 1;
                }
            }
        }
    }
    scene_items_count = count - 1;
    dst->next = 0;
}

void rotate_scene_coords(enum rotate_dir dir, uint8_t * x, uint8_t * y) __banked {
    uint8_t tmp;
    switch (dir) {
        case ROT_CW:
            tmp = *x, *x = *y, *y = max_scene_y - tmp - 1;
            break;
        case ROT_CCW:
            tmp = *y, *y = *x, *x = max_scene_x - tmp - 1;
            break;
    }
}

void rotate_scene(enum rotate_dir dir) __banked {
    dir;
    for (UINT8 z = 0; z < max_scene_z; z++) rorate_CCW(&collision_buf, z);
    map_to_scene(&collision_buf, scene_items);
}
