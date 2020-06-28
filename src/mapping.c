#pragma bank 2

#include <gb/gb.h>
#include "shadow.h"
#include "mapping.h"

const unsigned char viewport_map[] = {
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x02,0x03,0x04,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x0d,0x0e,0x0f,0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x00,0x00,0x00,
    0x00,0x19,0x1a,0x1b,0x1c,0x1d,0x1e,0x1f,0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,0x00,
    0x29,0x2a,0x2b,0x2c,0x2d,0x2e,0x2f,0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x3a,
    0x3b,0x3c,0x3d,0x3e,0x3f,0x40,0x41,0x42,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4a,0x4b,0x4c,
    0x4d,0x4e,0x4f,0x50,0x51,0x52,0x53,0x54,0x55,0x56,0x57,0x58,0x59,0x5a,0x5b,0x5c,0x5d,0x5e,
    0x5f,0x60,0x61,0x62,0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x6a,0x6b,0x6c,0x6d,0x6e,0x6f,0x70,
    0x71,0x72,0x73,0x74,0x75,0x76,0x77,0x78,0x79,0x7a,0x7b,0x7c,0x7d,0x7e,0x7f,0x80,0x81,0x82,
    0x00,0x83,0x84,0x85,0x86,0x87,0x88,0x89,0x8a,0x8b,0x8c,0x8d,0x8e,0x8f,0x90,0x91,0x92,0x00,
    0x00,0x00,0x00,0x93,0x94,0x95,0x96,0x97,0x98,0x99,0x9a,0x9b,0x9c,0x9d,0x9e,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x9f,0xa0,0xa1,0xa2,0xa3,0xa4,0xa5,0xa6,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xa7,0xa8,0xa9,0xaa,0x00,0x00,0x00,0x00,0x00,0x00,0x00
};

void set_view_port(UBYTE x, UBYTE y) __banked {
    set_bkg_tiles(x, y, viewport_width, viewport_height, viewport_map);
}

// constant structures declared just before __nonbanked functions are nonbanked too
const tiledesc_t used_tiles[] = {
    { &shadow_buffer[                           7*16],   0,  4 },
    { &shadow_buffer[     viewport_width * 16 + 5*16],   4,  8 },
    { &shadow_buffer[ 2 * viewport_width * 16 + 3*16],  12, 12 },
    { &shadow_buffer[ 3 * viewport_width * 16 + 1*16],  24, 16 },
    { &shadow_buffer[ 4 * viewport_width * 16 + 0*16],  40, 18 },
    { &shadow_buffer[ 5 * viewport_width * 16 + 0*16],  58, 18 },
    { &shadow_buffer[ 6 * viewport_width * 16 + 0*16],  76, 18 },
    { &shadow_buffer[ 7 * viewport_width * 16 + 0*16],  94, 18 },
    { &shadow_buffer[ 8 * viewport_width * 16 + 0*16], 112, 18 },
    { &shadow_buffer[ 9 * viewport_width * 16 + 1*16], 130, 16 },
    { &shadow_buffer[10 * viewport_width * 16 + 3*16], 146, 12 },
    { &shadow_buffer[11 * viewport_width * 16 + 5*16], 158,  8 },
    { &shadow_buffer[12 * viewport_width * 16 + 7*16], 166,  4 }
};

void copy_tiles() __nonbanked {
__asm
        push    BC

        ld      HL, #_used_tiles
        ld      A, #viewport_height
1$:
        push    AF

        ld      A, (HL+)
        ld      E, A
        ld      A, (HL+)
        ld      D, A
        push    DE
        
        ld      A, (HL+)
        inc     A
        ld      E, A
        ld      A, (HL+)
        ld      D, A
        push    DE
        
        ld      B, H
        ld      C, L
        
        call    _set_bkg_data
        add     SP, #4
        
        ld      H, B
        ld      L, C

        pop     AF
        dec     A
        jr      NZ, 1$

        pop     BC
__endasm;
}

void copy_dirty_tiles() __nonbanked {
__asm
        push    BC

        ld      DE, #_dirty_rows
        ld      HL, #_used_tiles
        ld      C, #viewport_height
1$:
        ld      A, (DE)
        inc     DE
        or      A
        jr      Z, 2$
        
        push    BC
        push    DE

        ld      A, (HL+)
        ld      E, A
        ld      A, (HL+)
        ld      D, A
        push    DE
        
        ld      A, (HL+)
        inc     A
        ld      E, A
        ld      A, (HL+)
        ld      D, A
        push    DE
        
        ld      B, H
        ld      C, L
        
        call    _set_bkg_data
        add     SP, #4
        
        ld      H, B
        ld      L, C
        
        pop     DE
        pop     BC        
        
        dec     C
        jr      NZ, 1$

        jr      3$
2$:        
        ld      A, #4
        add     L
        ld      L, A
        adc     H
        sub     L
        ld      H, A

        dec     C
        jr      NZ, 1$
3$:
        pop     BC
__endasm;
}

void copy_tiles_row(UBYTE row) __nonbanked {
  if (row < viewport_height) set_bkg_data(used_tiles[row].ofs + 1, used_tiles[row].count, used_tiles[row].data);
}

/*
// old pure C functions for reference
void copy_tiles() __nonbanked {
    static const tiledesc_t * used_tile_range;
    used_tile_range = used_tiles;
    for (UBYTE i = 0; i < viewport_height; i++) {
        set_bkg_data(used_tile_range->ofs + 1, used_tile_range->count, used_tile_range->data);
        used_tile_range++;
    }
}
*/