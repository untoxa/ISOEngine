#include "nonintrinsic.h"
#include "scenes.h"
#include "shadow.h"

// pointer to tile resources
const unsigned char * __tiles;
static const unsigned char * __empty;
static UBYTE __put_map_x, __put_map_y; 

void initialize_tiles(const unsigned char * tiles, const unsigned char * empty) {
    __tiles = tiles, __empty = empty;
}

static UBYTE __dy, __counter;
void __merge_masked() __naked {
__asm
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
        ld      A, #(item_tileheight * 8)
1$:        
        ld      (#___counter), A

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
        ld      A, #>((_shadow_buffer + (viewport_height * viewport_width * 16))) 
        cp      H
        jr      C, 3$
        jr      NZ, 4$
   
        ld      A, #<((_shadow_buffer + (viewport_height * viewport_width * 16)))
        cp      L
        jr      C, 3$
        jr      Z, 3$
   
4$:        
        pop     DE        
        xor     A
        
2$:     ld      (#___dy), A
                
        ld      A, (#___counter)
        dec     A
        jr      NZ, 1$
        ret
        
3$:     pop     DE
        ret
        
__endasm;
}

void __put_masked_map() __naked { 
__asm
        ;; now DE: mask, BC: item 

        ld      A, (#___put_map_y)
        cp      #((viewport_height - 1) * 8)
        ret     NC
        
        push    BC

        ld      HL, #___put_map_x
        ld      A, (hl)
        swap    A
        ld      C, A
        and     A, #0x0f
        ld      B, A
        ld      A, C
        and     A, #0xf0
        ld      C, A

        ld      HL, #_shadow_rows

        ld      A, (#___put_map_y)        
        srl     A
        srl     A
        and     A, #0xfe
        
        add     L
        ld      L, A
        adc     H
        sub     L
        ld      H, A
        
        ld      A, (HL+)
        ld      H,(HL)
        ld      L, A
        
        add     HL, BC

        ld      BC, #(_shadow_buffer + (viewport_height * viewport_width * 16))
        ld      A, B
        cp      H
        jr      C, 3$
        jr      NZ, 4$
        ld      A, C
        cp      L
        jr      C, 3$
4$:
        pop     BC
        
        push    DE
        push    BC
        push    HL

        ld      A, (#___put_map_y)
        and     A, #0x07
        ld      (#___dy), A
        
        call    ___merge_masked
        
        pop     HL
        ld      A, #0x10
        add     L
        ld      L, A
        adc     H
        sub     L
        ld      H, A        
        
        pop     BC
        ld      A, #(item_tileheight * 16)
        add     C
        ld      C, A
        adc     B
        sub     C
        ld      B, A
        
        pop     DE
        ld      A, #(item_tileheight * 16)
        add     E
        ld      E, A
        adc     D
        sub     E
        ld      D, A
        
        ld      A, (#___put_map_y)
        and     A, #0x07
        ld      (#___dy), A
        
        call    ___merge_masked        
        ret
        
3$:
        pop     BC
        ret
__endasm;
}

void redraw_scene(scene_item_t * scene) __preserves_regs(b, c) {
    scene;
__asm
        push    BC

        lda     HL, 4(SP)
        
        ;; HL = scene->next;
        ld      A, (HL+)
        ld      H, (HL)
        ld      L, A
        
        ld      A, #6
        add     L
        ld      L, A
        adc     H
        sub     L
        ld      H, A
        
        ld      A, (HL+)
        ld      H, (HL)
        ld      L, A

1$:
        or      H
        jr      Z, 2$
        
        ld      A, (HL+)
        cp      #0xc0           ; skip all items with ID > 0xc0 
        jr      NC, 3$

        push    HL

        ld      D, A
        xor     A
        ld      E, A
        srl     D
        rr      E
       
        ld      HL, #___tiles
        ld      A, (HL+)
        ld      H, (HL)
        ld      L, A       
       
        add     HL, DE
        ld      C, L
        ld      B, H            ; BC: spr = (unsigned char *)&__tiles[(int)__put_map_id << 7u]
        ld      DE, #(item_tileheight * item_tilewidth * 16)
        add     HL, DE
        ld      E, L
        ld      D, H            ; DE: mask = spr + 0x40u;

        pop     HL

        ld      A, (HL+)
        ld      (#___put_map_x), A
        ld      A, (HL+)
        ld      (#___put_map_y), A
        
        push    HL
        call    ___put_masked_map
        pop     HL
        
        inc     HL
        inc     HL
        inc     HL

4$:
        ld      A, (HL+)
        ld      H, (HL)
        ld      L, A            ; HL = HL->next;
        
        jr      1$
        
3$:     ld      A, #5
        add     L
        ld      L, A
        adc     H
        sub     L               ; HL = HL + 5
        jr      4$
2$:      
        pop     BC
__endasm; 
}

void erase_item(scene_item_t * item) __preserves_regs(b, c) {
    item;
__asm
        push    BC
        
        lda     HL, 4(SP)
        ld      A, (HL+)
        ld      H, (HL)
        ld      L, A
        
        inc     HL              ; skip id
        ld      A, (HL+)
        ld      (#___put_map_x), A
        ld      A, (HL)
        ld      (#___put_map_y), A
        
        ld      HL, #___empty
        ld      A, (HL+)
        ld      H, (HL)
        ld      L, A
        ld      B, H
        ld      C, L
        ld      D, H
        ld      E, L
        call    ___put_masked_map
        
        pop     BC
__endasm;
}

void draw_masked_bitmap_XY(UBYTE x, UBYTE y, const unsigned char * spr, const unsigned char * mask) __preserves_regs(b, c) {
    x; y; spr; mask;
__asm
        push    BC
        
        lda     HL, 9(SP)
        ld      A, (HL-)
        ld      D, A
        ld      A, (HL-)
        ld      E, A
        ld      A, (HL-)
        ld      B, A
        ld      A, (HL-)
        ld      C, A
        ld      A, (HL-)
        ld      (#___put_map_y), A
        ld      A, (HL)
        ld      (#___put_map_x), A
        call    ___put_masked_map
        
        pop     BC
__endasm;    
}

/*
// old pure C functions for reference
static UBYTE __put_map_id; 
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

void erase_item(scene_item_t * item) {
    __put_map_id = 0xffu, __put_map_x = item->x, __put_map_y = item->y;
    put_map();
}
*/

void remove_scene_item(scene_item_t * scene, scene_item_t * new_item) {
    if (new_item->n) {
        (scene + (new_item->n - 1))->next = new_item->next;
        new_item->n = 0, new_item->next = 0;
    }    
}

void place_scene_item(scene_item_t * scene, scene_item_t * new_item) {
    static scene_item_t * item;
    static int l, h, i, c;
    // bsearch 
    item = scene + 1;
    l = 0u, h = scene_items_count - 1u;
    while (l <= h) {
        i = (l + h) >> 1u;
        c = (int)((item + i)->coords) - (int)new_item->coords;
        if (c < 0) l = i + 1u; else h = i - 1u;
    }
    
    if (l < scene_items_count) {
        c = (int)((item + l)->coords) - (int)new_item->coords;
        if (c > 0) {
            if (l) item += l - 1; else item--;
        } else item += l; 
    } else item = scene + scene_items_count;

    new_item->next = item->next, new_item->n = item->n;
    item->next = new_item;
}

UBYTE copy_scene(UBYTE bank, const scene_item_t * sour, scene_item_t * dest) {
    static scene_item_t * src, * dst;
    static UBYTE count, savebank;

    savebank = GET_ROM_BANK, SET_ROM_BANK(bank);

    src = (scene_item_t *)sour, dst = dest;
    count = 1u;

    // zero item must always exist to simplify insertion of objects; it is not drawn
    dst->id = 0xffu, dst->x = 0u, dst->y = 0u, dst->n = 1u, dst->coords = 0u, dst->next = dst + 1;
    dst++;

    while (src) {
        dst->id = src->id;
        dst->x = src->x;
        dst->y = src->y;
        dst->coords = src->coords;
        src = src->next;
        dst->n = ++count;
        if (count == 254u) src = 0;
        if (src) dst->next = dst + 1; else dst->next = 0;
        dst++;
    }

    SET_ROM_BANK(savebank);
    
    scene_items_count = count - 1;
    return scene_items_count;
}

void clear_map(scene_t * dest) __preserves_regs(b, c) {
    dest;
__asm
        lda     HL, 2(SP)
        ld      A, (HL+)
        ld      H, (HL)
        ld      L, A
        ld      DE, #(max_scene_x * max_scene_y * max_scene_z / 4)
        inc     D
        inc     E
        xor     A
        jr      2$
1$:        
        ld      (HL+), A
        ld      (HL+), A
        ld      (HL+), A
        ld      (HL+), A
2$:        
        dec     E
        jr      NZ, 1$
        dec     D
        jr      NZ, 1$        
__endasm;    
}

/*
// old pure C functions for reference
void clear_map(scene_t * dest) {
    static unsigned char * tmp;
    static UWORD sz;
    sz = sizeof(*dest), tmp = (unsigned char *)dest;
    while (sz) *tmp++ = 0u, sz--;    
}
*/

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