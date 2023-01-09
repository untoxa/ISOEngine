#ifndef MULTIPLE_H_INCLUDE
#define MULTIPLE_H_INCLUDE

#include <gbdk/platform.h>
#include <stdint.h>

#include "scenes.h"
#include "clipping.h"

void remove_multiple_items(scene_item_t * scene, clip_item_t * items, uint8_t count);
void place_multiple_items(scene_item_t * scene, clip_item_t * items, uint8_t count);
void erase_multiple_items(clip_item_t * items, uint8_t count);
void update_multiple_items_pos(clip_item_t * items, uint8_t count);

#endif