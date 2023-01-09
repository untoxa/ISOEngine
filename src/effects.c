#pragma bank 2

#include <stdint.h>

#include "effects.h"
#include "mapping.h"
#include "scenes.h"

uint8_t FadeStep(uint8_t pal, uint8_t step) NAKED {
    pal; step;
__asm
            ld      D, E
            ld      E, A
            ld      A, D
            or      A
            ret     Z

            ld      D, A
1$:
            ld      H, #4
2$:
            ld      A, E
            and     #3
            jr      Z, 3$
            dec     A
3$:
            srl     A
            rr      L
            srl     A
            rr      L

            srl     E
            srl     E

            dec     H
            jr      NZ, 2$

            ld      E, L

            dec     D
            jr      NZ, 1$

            ld      A, E
            ret
__endasm;
}

static const uint8_t blank[18] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

void scroll_out(enum scroll_dir dir, uint8_t x, uint8_t y) BANKED {
    if (dir == SC_NONE) return;
    uint8_t pal = BGP_REG;
    for (uint8_t i = 0; i < 20; i++) {
        wait_vbl_done();
        switch (dir) {
            case SC_EAST:
                SCX_REG += 8, SCY_REG += 4;
                set_bkg_tiles(i, 0, 1, 18, blank);
                break;
            case SC_WEST:
                SCX_REG -= 8, SCY_REG -= 4;
                set_bkg_tiles(19 - i, 0, 1, 18, blank);
                break;
            case SC_NORTH:
                SCX_REG += 8, SCY_REG -= 4;
                set_bkg_tiles(i, 0, 1, 18, blank);
                break;
            case SC_SOUTH:
                SCX_REG -= 8, SCY_REG += 4;
                set_bkg_tiles(19 - i, 0, 1, 18, blank);
                break;
        }
        if (i == 4) {
            BGP_REG = FadeStep(pal, 1);
        } else if (i == 8) {
            BGP_REG = FadeStep(pal, 2);
        } else if (i == 14) {
            BGP_REG = FadeStep(pal, 3);
        }
    }
    copy_tiles();
    SCX_REG = SCY_REG = 0;
    set_view_port(x, y);
    wait_vbl_done();
    BGP_REG = pal;
}