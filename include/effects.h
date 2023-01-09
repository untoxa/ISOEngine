#ifndef EFFECTS_H_INCLUDE
#define EFFECTS_H_INCLUDE

#include <gbdk/platform.h>
#include <stdint.h>

enum scroll_dir {SC_NONE, SC_NORTH, SC_EAST, SC_SOUTH, SC_WEST};

void scroll_out(enum scroll_dir dir, uint8_t x, uint8_t y) BANKED;

#endif