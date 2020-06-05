#ifndef SHADOW_H_INCLUDE
#define SHADOW_H_INCLUDE

#include <gb/gb.h>

#include "globals.h"
#include "scenes.h"

#define shadow_buffer_limit (shadow_buffer + (viewport_height * viewport_width * 16))

extern unsigned char shadow_buffer[];
extern const unsigned char * const shadow_rows[];

extern scene_item_t scene_items[255];
extern scene_t collision_buf;

extern UBYTE scene_count;

void clear_shadow_buffer();

#endif