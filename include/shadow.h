#ifndef SHADOW_H_INCLUDE
#define SHADOW_H_INCLUDE

#include <gbdk/platform.h>
#include <stdint.h>

#include "globals.h"
#include "scenes.h"

#define shadow_buffer_limit (shadow_buffer + (viewport_height * viewport_width * 16))

extern uint8_t shadow_buffer[];
extern const uint8_t * const shadow_rows[];
extern uint8_t dirty_rows[];

extern scene_item_t scene_items[255];
extern uint8_t scene_items_count;
extern scene_t collision_buf;

void clear_shadow_buffer() PRESERVES_REGS(b, c);
void clear_dirty_rows() PRESERVES_REGS(b, c);

void mark_row_dirty(uint8_t y) OLDCALL PRESERVES_REGS(b, c);

#endif