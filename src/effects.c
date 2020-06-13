#pragma bank 1

#include "effects.h"
#include "mapping.h"
#include "scenes.h"

static const unsigned char blank[18] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

void scroll_out(enum scroll_dir dir, UBYTE x, UBYTE y) __banked {
    for (UBYTE i = 0; i < 20; i++) {
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
    }
    copy_tiles();
    wait_vbl_done();
    SCX_REG = SCY_REG = 0;
    set_view_port(x, y);
}