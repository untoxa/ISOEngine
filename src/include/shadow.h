#ifndef SHADOW_H_INCLUDE
#define SHADOW_H_INCLUDE

#include <gb/gb.h>

#include "globals.h"
#include "scenes.h"

#define shadow_buffer_limit (shadow_buffer + (viewport_height * viewport_width * 16))

extern unsigned char shadow_buffer[];
extern const unsigned char * const shadow_rows[];
extern unsigned char dirty_rows[];

extern scene_item_t scene_items[255];
extern UBYTE scene_items_count;
extern scene_t collision_buf;

void clear_shadow_buffer() __preserves_regs(b, c);
void clear_dirty_rows() __preserves_regs(b, c);

void mark_row_dirty(UBYTE y) __preserves_regs(b, c);

#endif