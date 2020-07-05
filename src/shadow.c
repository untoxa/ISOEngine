#include "globals.h"
#include "nonintrinsic.h"
#include "shadow.h"

// shadow buffer in RAM1
DATA_0 unsigned char shadow_buffer[(viewport_height * viewport_width * 16)];

// row index
const unsigned char * const shadow_rows[] = { 
    &shadow_buffer[                       0], &shadow_buffer[     viewport_width * 16], &shadow_buffer[ 2 * viewport_width * 16], &shadow_buffer[ 3 * viewport_width * 16], 
    &shadow_buffer[ 4 * viewport_width * 16], &shadow_buffer[ 5 * viewport_width * 16], &shadow_buffer[ 6 * viewport_width * 16], &shadow_buffer[ 7 * viewport_width * 16], 
    &shadow_buffer[ 8 * viewport_width * 16], &shadow_buffer[ 9 * viewport_width * 16], &shadow_buffer[10 * viewport_width * 16], &shadow_buffer[11 * viewport_width * 16], 
    &shadow_buffer[12 * viewport_width * 16] };

// scene in RAM1
DATA_0 scene_item_t scene_items[255];

// scene item count in WRAM, not SRAM0
UBYTE scene_items_count;

// collision buffer in RAM1
DATA_0 scene_t collision_buf;

DATA_0 unsigned char dirty_rows[16];

// this is a marker that shows the end of used SRAM0 - just for debugging
DATA_0 UBYTE __end_marker;

void __memset8() __naked {
__asm
        inc     D
        inc     E
        xor     A
        jr      2$
1$:        
        ld      (HL+), A
        ld      (HL+), A
        ld      (HL+), A
        ld      (HL+), A
        ld      (HL+), A
        ld      (HL+), A
        ld      (HL+), A
        ld      (HL+), A
2$:        
        dec     E
        jr      NZ, 1$
        dec     D
        jr      NZ, 1$        
        ret
__endasm;    
}

void clear_shadow_buffer() __naked {
__asm
        ld      HL, #_shadow_buffer
        ld      DE, #(viewport_height * viewport_width * (16 / 8))
        jp      ___memset8
__endasm;    
}

void clear_dirty_rows() __naked {
__asm
        ld      HL, #_dirty_rows
        ld      DE, #(16 / 8)
        jp      ___memset8
__endasm;    
}

void mark_row_dirty(UBYTE y) __naked {
    y;
__asm
        lda     HL, 2(SP)
        ld      A, (HL)
        
        srl     A
        srl     A
        srl     A
        rl      E
        and     #0x0f
        
        ld      HL, #_dirty_rows
        add     L
        ld      L, A
        adc     H
        sub     L
        ld      H, A
        
        ld      A, #1
        ld      (HL+), A
        ld      (HL+), A
        
        srl     E
        ret     NC
        
        ld      (HL), A
        ret
__endasm;    
}

/*
// old pure C functions for reference
void clear_shadow_buffer() {
    static unsigned char * shadow_ptr;
    static UWORD sz;
    sz = sizeof(shadow_buffer), shadow_ptr = (unsigned char *)&shadow_buffer;
    while(sz) *shadow_ptr++ = 0u, sz--;
}
*/    
