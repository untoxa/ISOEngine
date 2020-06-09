#include "scenes.h"
#include "shadow.h"

// pointer to tile resources
static const unsigned char * __tiles, *__empty;
static UBYTE __put_map_x, __put_map_y, __put_map_id;

void initialize_tiles(const unsigned char * tiles, const unsigned char * empty) {
    __tiles = tiles, __empty = empty;
}

UBYTE __dy, __counter;
void __merge(unsigned char * dest, const unsigned char * spr, const unsigned char * mask) __naked {
    dest; spr; mask;
__asm
        push    BC

        lda     HL, 9(SP)
        ld      D, (HL)
        dec     HL
        ld      E, (HL)
        dec     HL
        ld      B, (HL)
        dec     HL
        ld      C, (HL)
        dec     HL
        ld      A, (HL-)
        ld      L, (HL)
        ld      H, A

        ;; now HL: data, DE: mask, BC: item 

        ;; HL += ___dy << 1
        ld      A, (#___dy)
        add     A
        add     L
        ld      L, A
        adc     H
        sub     L
        ld      H, A

        ;; item is 2 tiles high
        ld      A, #0x10
        ld      (#___counter), A
1$:        
        ;; AND with item mask
        ld      A, (DE)
        and     (HL)
        ld      (HL+), A
        inc     DE
        
        ld      A, (DE)
        and     (HL)
        ld      (HL), A
        inc     DE
        
        dec     HL
        
        ;; OR with item
        ld      A, (BC)
        or      (HL)
        ld      (HL+), A
        inc     BC
        
        ld      A, (BC)
        or      (HL)
        ld      (HL+), A
        inc     BC        
        
        ;; check moving to the next tile by Y
        ld      A, (#___dy)
        inc     A
        cp      #8
        jr      NZ, 2$
        
        push    DE

        ;; move to next tile by Y
        ld      DE, #((viewport_width - 1) * 16)
        add     HL, DE
                
        ;; check shadow buffer boundaries: HL < shadow_buffer + sizeof(shadow_buffer)        
        ld      A, #>((_shadow_buffer + 0x0ea0)) 
        cp      H
        jr      C, 3$
        jr      NZ, 4$
   
        ld      A, #<((_shadow_buffer + 0x0ea0))
        cp      L
        jr      C, 3$
        jr      Z, 3$
   
4$:        
        pop     DE        
        xor     A
        
2$:     ld      (#___dy), A
                
        ld      A, (#___counter)
        dec     A
        ld      (#___counter), A
        jr      NZ, 1$
        
        pop     BC
        
        ret
        
3$:     pop     DE
        pop     BC
        ret
        
__endasm;
}

void put_map() { 
    static unsigned char * data, * spr, * mask, * limit;
        
    if (__put_map_y < ((viewport_height - 1) * 8)) {       
        
        if (__put_map_id != 0xffu) {
            spr = (unsigned char *)&__tiles[(int)__put_map_id << 7u], mask = spr + 0x40u;
        }  else {
            spr = mask = (unsigned char *)__empty;
        }
                          
        data = (unsigned char *)shadow_rows[(__put_map_y >> 3u)];
        data += ((WORD)(__put_map_x) << 4u);
        
        if (data > shadow_buffer_limit) return;
        
        __dy = (__put_map_y & 7u);
        __merge(data, spr, mask);
        
        // next column of 2x2 tile item and mask
        spr += 0x20u, mask += 0x20u;
        
        __dy = (__put_map_y & 7u);
        __merge(data + 0x10u, spr, mask);

    }
}

void redraw_scene(scene_item_t * scene) {
    scene;
__asm
        lda     HL, 2(SP)
        
        ;; HL = scene->next;
        ld      A, (HL+)
        ld      H, (HL)
        
        ld      E, #6
        add     E
        ld      L, A
        adc     H
        sub     L
        ld      H, A
        
        ld      A, (HL+)
        ld      H, (HL)
        ld      L, A
        
        dec     E               ; E == 5
1$:     
        or      H
        jr      Z, 2$
        
        ld      A, (HL+)
        cp      #0xc0           ; skip all items with ID > 0xc0 
        jr      NC, 3$
        
        ld      (#___put_map_id), A
        ld      A, (HL+)
        ld      (#___put_map_x), A
        ld      A, (HL+)
        ld      (#___put_map_y), A
        
        push    HL
        call    _put_map
        pop     HL
        
        inc     HL
        inc     HL
        inc     HL

4$:
        ld      A, (HL+)
        ld      H, (HL)
        ld      L, A            ; HL = HL->next;
        
        jr      1$
        
3$:     ld      A, L
        add     E
        ld      L, A
        adc     H
        sub     L               ; HL = HL + 5
        jr      4$
2$:      
__endasm; 
}

/*
// old pure C for reference
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
        data1 += ((WORD)(__put_map_x) << 4u);
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
    item = item->next; // skip zero item of the scene
    while (item) {
        __put_map_x = item->x, __put_map_y = item->y, __put_map_id = item->id;
        put_map();
        item = item->next;
    }
}
*/

void draw_pattern_XYZ(UBYTE x, UBYTE y, UBYTE z, UBYTE id) {
    __put_map_id = id, __put_map_x = to_x(x, y, z), __put_map_y = to_y(x, y, z);
    put_map();
}

void erase_item(scene_item_t * item) {
    __put_map_id = 0xffu, __put_map_x = item->x, __put_map_y = item->y;
    put_map();
}

void replace_scene_item(scene_item_t * scene, scene_item_t * new_item) {
    static scene_item_t * item, * replace;
    static int l, h, i, c;
    if (new_item->n) {
        item = scene;
        item += (new_item->n - 1);
        item->next = new_item->next;
    }
    // bsearch 
    item = scene + 1;
    l = 0u, h = scene_items_count - 1u;
    while (l <= h) {
        i = (l + h) >> 1u;
        c = (int)((item + i)->coords) - (int)new_item->coords;
        if (c < 0) l = i + 1u; else h = i - 1u;
    }
    
    if (c > 0) {
        // if not found then correct the place for item insertion
        if (l) item += l - 1; else item--;
    } else {
        if (l == scene_items_count) l--;
        item += l;
    }

    new_item->next = item->next, new_item->n = item->n;
    item->next = new_item;
}

UBYTE copy_scene(const scene_item_t * sour, scene_item_t * dest) {
    static scene_item_t * src, * dst;
    UBYTE count, id = 1u;

    src = (scene_item_t *)sour, dst = dest;
    count = 0u;

    // zero item must always exist to simplify insertion of objects; it is not drawn
    dst->id = 0xffu, dst->x = 0u, dst->y = 0u, dst->n = id, dst->coords = 0u, dst->next = dst + 1;
    dst++;

    while (src) {
        dst->id = src->id;
        dst->x = src->x;
        dst->y = src->y;
        dst->coords = src->coords;
        src = src->next;
        dst->n = ++id;
        count++;
        if (count == 254u) src = 0;
        if (src) dst->next = dst + 1; else dst->next = 0;
        dst++;
    }

    scene_items_count = count;
    return count;
}

void clear_map(scene_t * dest) {
    static unsigned char * tmp;
    static UWORD sz;
    sz = sizeof(*dest), tmp = (unsigned char *)dest;
    while (sz) *tmp++ = 0u, sz--;    
}

void scene_to_map(const scene_item_t * sour, scene_t * dest) {
    static scene_item_t * src;
    static UBYTE x, y, z;
    
    clear_map(dest);
    
    src = (scene_item_t *)sour;
    while (src) {
        from_coords(src->coords, x, y, z);
        if ((x < max_scene_x) && (y < max_scene_y) && (z < max_scene_z)) {
            (*dest)[x][z][y] = src->id + 1;
        }
        src = src->next;
    }    
}