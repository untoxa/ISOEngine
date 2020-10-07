#ifndef __NONINTRINSIC_H_INCLUDE
#define __NONINTRINSIC_H_INCLUDE

#include <gb/gb.h>

#define SET_ROM_BANK(n) (SWITCH_ROM_MBC1((n)))
#define GET_ROM_BANK _current_bank
#define SET_RAM_BANK(n) (SWITCH_RAM_MBC1((n)))

void set_ROM_bank1(void) __nonbanked __preserves_regs(b, c, d, e);
void set_ROM_bank2(void) __nonbanked __preserves_regs(b, c, d, e);
void set_ROM_bank3(void) __nonbanked __preserves_regs(b, c, d, e);

__addressmod set_ROM_bank1 const CODE_1;
__addressmod set_ROM_bank2 const CODE_2;
__addressmod set_ROM_bank3 const CODE_3;

void set_RAM_bank0(void) __nonbanked __preserves_regs(b, c, d, e);

__addressmod set_RAM_bank0 DATA_0;

#endif