#ifndef EFFECTS_H_INCLUDE
#define EFFECTS_H_INCLUDE

#include <gb/gb.h>

enum scroll_dir { SC_NORTH, SC_EAST, SC_SOUTH, SC_WEST};

void scroll_out(enum scroll_dir dir, UBYTE x, UBYTE y) __banked;

#endif