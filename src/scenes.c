#include "scenes.h"

UBYTE copy_scene(const scene_item_t * sour, scene_item_t * dest) {
    static scene_item_t * src, * dst;
    UBYTE count;

    src = (scene_item_t *)sour, dst = dest;
    count = 0u;

    while (src) {
        dst->id = src->id;
        dst->x = src->x;
        dst->y = src->y;
        dst->coords = src->coords;
        src = src->next;
        count++;
        if (count == 255u) src = 0;
        if (src) dst->next = dst + 1; else dst->next = 0;
        dst++;
    }

    return count;
}

void scene_to_map(const scene_item_t * sour, scene_t * dest) {
    static scene_item_t * src;
    static unsigned char * tmp;
    static UBYTE x, y, z;
    static UWORD sz;
    sz = sizeof(*dest), tmp = (unsigned char *)dest;
    while (sz) *tmp++ = 0u, sz--;
    
    src = (scene_item_t *)sour;
    while (src) {
        from_coords(src->coords, x, y, z);
        (*dest)[x][z][y] = src->id + 1;
        src = src->next;
    }    
}