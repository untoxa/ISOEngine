#ifndef CLIPPING_H_INCLUDE
#define CLIPPING_H_INCLUDE

#include <gb/gb.h>
#include "globals.h"

#include "scenes.h"

typedef unsigned char item_bitmap_t[item_tilewidth*item_tileheight*16];

typedef struct clip_item_t {
    UBYTE x, y, z;
    UBYTE flags; 
    item_bitmap_t * item_bkg;
    struct scene_item_t * scene_item;
} clip_item_t;
// sizeof(clip_item_t) must be 8
check_size(clip_item_t, 8);

// copy bitmap from the shadow buffer
void copy_from_shadow_XY(UBYTE x, UBYTE y, item_bitmap_t * dest);

// draws bitmap to the shadow buffer
void draw_to_shadow_XY(UBYTE x, UBYTE y, const unsigned char * spr);

// calculates the clipping mask
void calculate_mask(scene_item_t * scene, scene_item_t * new_item, item_bitmap_t * dest);

// apply inverse mask to bitmap
void apply_inverse_mask(item_bitmap_t * sour, item_bitmap_t * mask, item_bitmap_t * dest);

// save the background under the item
#define save_item_bkg(item) copy_from_shadow_XY(item.scene_item->x, item.scene_item->y, item.item_bkg)

// erase item and restore the background
#define restore_item_bkg(item) draw_to_shadow_XY(item.scene_item->x, item.scene_item->y, (const unsigned char *)item.item_bkg)

// calculate clipping and draw item to the shadow buffer
void draw_item(scene_item_t * scene, clip_item_t * item);

void test_clipping(item_bitmap_t * dest);

#endif