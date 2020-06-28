#include <gb/gb.h>
#include "nonintrinsic.h"

void set_ROM_bank1(void) __nonbanked __preserves_regs(b, c, d, e) { SET_ROM_BANK(1); }
void set_ROM_bank2(void) __nonbanked __preserves_regs(b, c, d, e) { SET_ROM_BANK(2); }
void set_ROM_bank3(void) __nonbanked __preserves_regs(b, c, d, e) { SET_ROM_BANK(3); }
