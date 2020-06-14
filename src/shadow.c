#include "globals.h"
#include "shadow.h"

void set_RAM_bank1(void) __nonbanked __preserves_regs(b, c, d, e) { SWITCH_RAM_MBC1(1); }

__addressmod set_RAM_bank1 DATA_0;

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
UBYTE scene_items_count;

// collision buffer in RAM1
DATA_0 scene_t collision_buf;

DATA_0 UBYTE __end_marker;

void clear_shadow_buffer() {
__asm
        ld      HL, #_shadow_buffer
        ld      DE, #(viewport_height * viewport_width * (16 / 8))
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
