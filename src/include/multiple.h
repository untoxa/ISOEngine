#ifndef MULTIPLE_H_INCLUDE
#define MULTIPLE_H_INCLUDE

#include <gb/gb.h>
#include "scenes.h"
#include "clipping.h"

void remove_multiple_items(scene_item_t * scene, clip_item_t * items, UBYTE count);
void place_multiple_items(scene_item_t * scene, clip_item_t * items, UBYTE count);
void erase_multiple_items(clip_item_t * items, UBYTE count);
void update_multiple_items_pos(clip_item_t * items, UBYTE count);

#endif