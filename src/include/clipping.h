#ifndef CLIPPING_H_INCLUDE
#define CLIPPING_H_INCLUDE

#include <gb/gb.h>
#include "globals.h"

typedef unsigned char item_bitmap_t[item_tilewidth*item_tileheight*16];

void copy_from_shadow(UBYTE x, UBYTE y, item_bitmap_t * dest);

void test_clipping();

#endif