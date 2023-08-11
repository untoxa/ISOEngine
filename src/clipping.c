#include <stdint.h>
#include <string.h>

#include "clipping.h"
#include "shadow.h"

void __get(void) NAKED {
__asm
        ;; now HL: source, DE: dest, C: dy

        ;; HL += (___dy & 7) << 1
        ld      A, C
        and     A, #0x07
        ld      C, A
        add     A
        add     L
        ld      L, A
        adc     H
        sub     L
        ld      H, A

        ;; item is 2 tiles high
        ld      B, #(item_tileheight * 8)
1$:
        ;; AND with item mask
        ld      A, (HL+)
        ld      (DE), A
        inc     DE
        ld      A, (HL+)
        ld      (DE), A
        inc     DE

        ;; check moving to the next tile by Y
        ld      A, #8
        inc     C
        cp      C
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
        ld      C, #0

2$:     dec     B
        jr      NZ, 1$
        ret

3$:     pop     DE
        ret

__endasm;
}

void __get_map(void) NAKED {
__asm
        ;; now B: x, C: y, DE: mask

        ld      A, C
        cp      #((viewport_height - 1) * 8)
        ret     NC

        push    BC

        ld      HL, #_shadow_rows

        ld      A, C
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

        ld      A, B
        swap    A
        ld      C, A
        and     A, #0x0f
        ld      B, A
        ld      A, C
        and     A, #0xf0
        ld      C, A

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

        push    BC
        push    DE
        push    HL

        call    ___get

        pop     HL
        ld      A, #0x10
        add     L
        ld      L, A
        adc     H
        sub     L
        ld      H, A

        pop     DE
        ld      A, #(item_tileheight * 16)
        add     E
        ld      E, A
        adc     D
        sub     E
        ld      D, A

        pop     BC

        call    ___get
        ret

3$:
        pop     BC
        ret
__endasm;
}

void copy_from_shadow_XY(uint8_t x, uint8_t y, item_bitmap_t * dest) OLDCALL PRESERVES_REGS(b, c) {
    x; y; dest;
__asm
        push    BC
        lda     HL, 4(SP)
        ld      A, (HL+)
        ld      B, A
        ld      A, (HL+)
        ld      C, A
        ld      A, (HL+)
        ld      E, A
        ld      A, (HL)
        ld      D, A
        call    ___get_map
        pop     BC
__endasm;
}

uint8_t __absuint8(int8_t v) NAKED {
    v;
__asm
        bit     #7, A
        ret     Z
        cpl
        inc     A
        ret
__endasm;
}

void __merge_inverse_mask(UINT8 dy, const uint8_t * sour, uint8_t * dest) OLDCALL {
    dy; sour; dest;
__asm
        push    BC
        lda     HL, 4(SP)
        ld      A, (HL+)
        ld      C, A
        ld      A, (HL+)
        ld      E, A
        ld      A, (HL+)
        ld      D, A
        ld      A, (HL+)
        ld      H, (HL)
        ld      L, A

        ld      A, C
        bit     #7, A
        jr      Z, 1$
        xor     A, #0xff
        inc     A

        ld      B, A
        add     A
        add     L
        ld      L, A
        adc     H
        sub     L
        ld      H, A
        jr      2$
1$:
        ld      B, A
        add     A
        add     E
        ld      E, A
        adc     D
        sub     E
        ld      D, A
2$:
        ld      A, #16
        sub     B
        ld      B, A
3$:
        ld      A, (DE)
        xor     A, #0xff
        inc     DE
        or      (HL)
        ld      (HL+), A
        ld      A, (DE)
        xor     A, #0xff
        inc     DE
        or      (HL)
        ld      (HL+), A

        dec     B
        jr      NZ, 3$

        pop     BC
__endasm;
}

void merge_inverse_masks(scene_item_t * item, scene_item_t * new_item, uint8_t * dest) {
    static const uint8_t * sour;
    static int8_t ydist;
    static uint8_t x, y;
    if (!item) return;
    x = new_item->x, y = new_item->y;
    // skip items with lower or equal 3D coords
    while ((item) && ((item->coords) <= new_item->coords)) item = item->next;
    // iterate through the rest of the scene
    while (item) {
        if (x == item->x) {
            ydist = y - item->y;
            if (__absuint8(ydist) < 16) {
                sour = &__tiles[((int)(item->id) << 7u) + 0x40];
                __merge_inverse_mask(ydist, sour, dest);
                __merge_inverse_mask(ydist, sour + (item_tileheight * 16), dest + (item_tileheight * 16));
            }
        } else if (x == item->x + 1) {
            ydist = y - item->y;
            if (__absuint8(ydist) < 16) {
                sour = &__tiles[((int)(item->id) << 7u) + 0x40];
                __merge_inverse_mask(ydist, sour + (item_tileheight * 16), dest);
            }
        } else if (x == item->x - 1) {
            ydist = y - item->y;
            if (__absuint8(ydist) < 16) {
                sour = &__tiles[((int)(item->id) << 7u) + 0x40];
              __merge_inverse_mask(ydist, sour, dest + (item_tileheight * 16));
            }
        }
        item = item->next;
    }
}

static uint8_t __dy, __counter;
void __copy(void) NAKED {
__asm
        ;; now HL: data, BC: item

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

        ;; copy data
        ld      A, (BC)
        ld      (HL+), A
        inc     BC

        ld      A, (BC)
        ld      (HL+), A
        inc     BC

        ;; check moving to the next tile by Y
        ld      A, (#___dy)
        inc     A
        cp      #8
        jr      NZ, 2$

        ;; move to next tile by Y
        ld      DE, #((viewport_width - 1) * 16)
        add     HL, DE

        ;; check shadow buffer boundaries: HL < shadow_buffer + sizeof(shadow_buffer)
        ld      A, #>((_shadow_buffer + (viewport_height * viewport_width * 16)))
        cp      H
        ret     C
        jr      NZ, 4$

        ld      A, #<((_shadow_buffer + (viewport_height * viewport_width * 16)))
        cp      L
        ret     C
        ret     Z

4$:
        xor     A

2$:     ld      (#___dy), A

        ld      A, (#___counter)
        dec     A
        jr      NZ, 1$
        ret
__endasm;
}

static uint8_t __put_map_x, __put_map_y;
void __put_map(void) NAKED {
__asm
        ;; now BC: item

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

        push    BC
        push    HL

        ld      A, (#___put_map_y)
        and     A, #0x07
        ld      (#___dy), A

        call    ___copy

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

        ld      A, (#___put_map_y)
        and     A, #0x07
        ld      (#___dy), A

        call    ___copy
        ret

3$:
        pop     BC
        ret
__endasm;
}

void draw_to_shadow_XY(uint8_t x, uint8_t y, const uint8_t * spr) OLDCALL PRESERVES_REGS(b, c) {
    x; y; spr;
__asm
        push    BC

        lda     HL, 7(SP)
        ld      A, (HL-)
        ld      B, A
        ld      A, (HL-)
        ld      C, A
        ld      A, (HL-)
        ld      (#___put_map_y), A
        ld      A, (HL)
        ld      (#___put_map_x), A
        call    ___put_map

        pop     BC
__endasm;
}

void calculate_mask(scene_item_t * scene, scene_item_t * new_item, item_bitmap_t * dest) {
    static scene_item_t * item, * replace;
    static int l, h, i, c;

    // initialize map
    memcpy(dest, &__tiles[((int)(new_item->id) << 7u) + 0x40], (item_tilewidth*item_tileheight*16));

    // bsearch
    item = scene + 1;
    l = 0u, h = scene_items_count - 1u;
    while (l <= h) {
        i = (l + h) >> 1u;
        c = (int)((item + i)->coords) - (int)new_item->coords;
        if (c < 0) l = i + 1u; else h = i - 1u;
    }

    if (l == scene_items_count) return;
    c = (int)((item + l)->coords) - (int)new_item->coords;

    if (c > 0) {
        // if not found then correct the place for item insertion
        if (l) item += l - 1; else item--;
    } else item += l;

    merge_inverse_masks(item, new_item, (uint8_t *)dest);
}

void apply_inverse_mask(item_bitmap_t * sour, item_bitmap_t * mask, item_bitmap_t * dest) OLDCALL PRESERVES_REGS(b, c) {
    sour; mask; dest;
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
        ld      L, (HL)
        ld      H, A

        ld      A, #(item_tilewidth * item_tileheight * 16 / 4)
1$:
        ld      (#___counter), A

        ld      A, (BC)
        xor     A, #0xff
        and     (HL)
        ld      (DE), A
        inc     BC
        inc     DE
        inc     HL

        ld      A, (BC)
        xor     A, #0xff
        and     (HL)
        ld      (DE), A
        inc     BC
        inc     DE
        inc     HL

        ld      A, (BC)
        xor     A, #0xff
        and     (HL)
        ld      (DE), A
        inc     BC
        inc     DE
        inc     HL

        ld      A, (BC)
        xor     A, #0xff
        and     (HL)
        ld      (DE), A
        inc     BC
        inc     DE
        inc     HL

        ld      A, (#___counter)
        dec     A
        jr      NZ, 1$

        pop     BC
__endasm;
}

static item_bitmap_t temp_mask, temp_bitmap;
void draw_item(scene_item_t * scene, clip_item_t * item) {
    calculate_mask(scene, item->scene_item, &temp_mask);
    apply_inverse_mask((item_bitmap_t *)(&__tiles[(int)(item->scene_item->id) << 7u]), &temp_mask, &temp_bitmap);
    draw_masked_bitmap_XY(item->scene_item->x, item->scene_item->y, (uint8_t *)&temp_bitmap, (unsigned char *)&temp_mask);
    mark_row_dirty(item->scene_item->y);
}
