#include "scenes.h"
#include "shadow.h"

// pointer to tile resources
static const unsigned char * __tiles, *__empty;
static UBYTE __put_map_x, __put_map_y, __put_map_id;

void initialize_tiles(const unsigned char * tiles, const unsigned char * empty) {
    __tiles = tiles, __empty = empty;
}

void put_map() { 
    static UBYTE i, oy, dy;
    static unsigned char * data1, * data2, * spr, * mask, * limit;
        
    if (__put_map_y < ((viewport_height - 1) * 8)) {       
        oy = __put_map_y >> 3u;
        
        if (__put_map_id != 0xffu) {
            spr = (unsigned char *)&__tiles[(int)__put_map_id << 7u];
            mask = spr + 0x40u;
        }  else {
            spr = mask = (unsigned char *)__empty;
        }
               
        dy = (__put_map_y & 7u);
           
        data1 = (unsigned char *)shadow_rows[oy];
        data1 += ((WORD)(__put_map_x - 1) << 4u);
        data1 += (dy << 1u);
        data2 = data1 + 16u;
        
        if (data1 > shadow_buffer_limit) return;
        
        for (i = 0u; i < 0x10u; i++) {
            *data1++ = *data1 & *mask++ | *spr++;
            *data1++ = *data1 & *mask++ | *spr++;
            dy++;
            if (dy == 8u) {
                dy = 0; 
                data1 += ((viewport_width - 1) * 16);
                if (data1 > shadow_buffer_limit) break; 
            }
        }
        
        dy = (__put_map_y & 7u);  

        for (i = 0u; i < 0x10u; i++) {
            *data2++ = *data2 & *mask++ | *spr++;
            *data2++ = *data2 & *mask++ | *spr++;
            dy++;
            if (dy == 8u) {
                dy = 0; 
                data2 += ((viewport_width - 1) * 16);
                if (data2 > shadow_buffer_limit) break; 
            }
        }
    }
}

void redraw_scene(scene_item_t * scene) {
    static scene_item_t * item;
    item = scene;
    while (item) {
        __put_map_x = item->x, __put_map_y = item->y, __put_map_id = item->id;
        put_map();
        item = item->next;
    }
}

void draw_pattern_XYZ(UBYTE x, UBYTE y, UBYTE z, UBYTE id) {
    __put_map_x = to_x(x, y, z), __put_map_y = to_y(x, y, z), __put_map_id = id;
    put_map();
}

void erase_item(scene_item_t * item) {
    __put_map_x = item->x, __put_map_y = item->y, __put_map_id = 0xffu;
    put_map();
}

void replace_scene_item(scene_item_t * scene, scene_item_t * new_item) {
    static scene_item_t * item, * old, * replace;
    static UBYTE done_ins, done_rep;
    old = scene, item = scene->next;
    done_ins = done_rep = 0;
    replace = new_item->next;
    while (item) {
        if (item->next == new_item) {
            item->next = replace;
            if (done_ins) return;
            done_rep = 1;
        }
        if ((!done_ins) && (new_item->coords < item->coords)) {
            old->next = new_item, new_item->next = item;
            if (done_rep) return;
            done_ins = 1;
        }
        old = item, item = item->next;
    }
    if (!done_ins) {
        old->next = new_item, new_item->next = 0;
    }    
}

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