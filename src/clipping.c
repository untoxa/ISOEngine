#include "clipping.h"
#include "shadow.h"

void __get() __naked {
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

void __get_map() __naked { 
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

void copy_from_shadow(UBYTE x, UBYTE y, item_bitmap_t * dest) {
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

/*
void merge_images(UBYTE x, UBYTE y, UBYTE ix, UBYTE iy, item_bitmap_t * sour, item_bitmap_t * dest) {
    UBYTE ydist = ((y > iy) ? y - iy : iy - y);
    if (ydist < 16) {
        if (x == ix) {
            
        } else if (x == ix + 1) { 
        
        } else if (x == ix - 1) {
            
        }
    }
}
*/

static const unsigned char tls[4] = {200,202,201,203};
static item_bitmap_t dest;
static UBYTE i = 5;
void test_clipping() {
    copy_from_shadow(i, 8*8, &dest);
    set_bkg_data(tls[0], 4, dest);
    set_bkg_tiles(0, 0, 2, 2, tls);
    i++; if (i == 18) i = 0;
}