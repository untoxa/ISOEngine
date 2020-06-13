#ifndef ENEMIES_H_INCLUDE
#define ENEMIES_H_INCLUDE

#include <gb/gb.h>
#include "scenes.h"

typedef struct moving_item_t {
    UBYTE x, y, z;
    UBYTE flags; 
    UBYTE unused0, unused1;
    struct scene_item_t * scene_item;
} moving_item_t;
// sizeof(moving_item_t) must be 8
check_size(moving_item_t, 8);

void remove_multiple_items(scene_item_t * scene, moving_item_t * items, UBYTE count);
void place_multiple_items(scene_item_t * scene, moving_item_t * items, UBYTE count);
void erase_multiple_items(moving_item_t * items, UBYTE count);
void update_multiple_items_pos(moving_item_t * items, UBYTE count);

#endif