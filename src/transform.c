#pragma bank 1

#include "transform.h"
#include "shadow.h"
#include "scenes.h"

void swap(UBYTE * a, UBYTE * b) {
    UBYTE c = *a; 
    *a = *b, *b = c;
}

void reverse_columns(const scene_t * sour, INT8 z) 
{ 
    for (INT8 i = 0; i < max_scene_x; i++) 
        for (INT8 j = 0, k = max_scene_x - 1; j < k; j++, k--) 
            swap(&((*sour)[j][z][i]), &((*sour)[k][z][i]));
} 
  
void transpose(const scene_t * sour, INT8 z) 
{ 
    for (INT8 i = 0; i < max_scene_y; i++) 
        for (INT8 j = i; j < max_scene_x; j++) 
            swap(&((*sour)[i][z][j]), &((*sour)[j][z][i]));
} 

void map_to_scene(const scene_t * sour, scene_item_t * dest) {
    static scene_item_t * dst;
    static UBYTE id, count;
        
    dst = dest;
    count = 1u;

    // zero item must always exist to simplify insertion of objects; it is not drawn
    dst->id = 0xffu, dst->x = 0u, dst->y = 0u, dst->n = 1u, dst->coords = 0u, dst->next = dst + 1;
    
    for (INT8 x = 0; x < max_scene_x; x++) {
        for (INT8 y = max_scene_y; y != 0; y--) {
            for (INT8 z = 0; z < max_scene_z; z++) {
                id = (*sour)[x][z][y-1];
                if ((id) && (id < 0xc0u)) {
                    dst++;
                    dst->id = id - 1;
                    dst->x = to_x(x, y-1, z), dst->y = to_y(x, y-1, z);
                    dst->n = ++count;
                    dst->coords = to_coords(x, y-1, z);
                    dst->next = dst + 1;
                }
            }
        }
    }
    scene_items_count = count - 1;
    dst->next = 0;
}

void rotate_scene(enum rotate_dir dir) __banked {
    dir;
    for (UINT8 z = 0; z < max_scene_z; z++) {
        transpose(&collision_buf, z); 
        reverse_columns(&collision_buf, z); 
    }
    map_to_scene(&collision_buf, scene_items);
}
