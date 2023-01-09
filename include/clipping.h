#ifndef CLIPPING_H_INCLUDE
#define CLIPPING_H_INCLUDE

#include <gbdk/platform.h>
#include <stdint.h>

#include "globals.h"

#include "scenes.h"

typedef uint8_t item_bitmap_t[item_tilewidth*item_tileheight*16];

typedef struct clip_item_t {
    uint8_t x, y, z;
    uint8_t flags;
    item_bitmap_t * item_bkg;
    struct scene_item_t * scene_item;
} clip_item_t;
// sizeof(clip_item_t) must be 8
check_size(clip_item_t, 8);

// copy bitmap from the shadow buffer
void copy_from_shadow_XY(uint8_t x, uint8_t y, item_bitmap_t * dest) OLDCALL PRESERVES_REGS(b, c);

// draws bitmap to the shadow buffer
void draw_to_shadow_XY(uint8_t x, uint8_t y, const uint8_t * spr) OLDCALL PRESERVES_REGS(b, c);

// calculates the clipping mask
void calculate_mask(scene_item_t * scene, scene_item_t * new_item, item_bitmap_t * dest);

// apply inverse mask to bitmap
void apply_inverse_mask(item_bitmap_t * sour, item_bitmap_t * mask, item_bitmap_t * dest) OLDCALL PRESERVES_REGS(b, c);

// save the background under the item
#define save_item_bkg(item) copy_from_shadow_XY(item.scene_item->x, item.scene_item->y, item.item_bkg)

// erase item and restore the background
#define restore_item_bkg(item) draw_to_shadow_XY(item.scene_item->x, item.scene_item->y, (const uint8_t *)item.item_bkg), mark_row_dirty(item.scene_item->y)

// calculate clipping and draw item to the shadow buffer
void draw_item(scene_item_t * scene, clip_item_t * item);

#endif