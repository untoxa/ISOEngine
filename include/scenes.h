#ifndef SCENES_H_INCLUDE
#define SCENES_H_INCLUDE

#include <gbdk/platform.h>
#include <stdint.h>

#include "globals.h"

typedef uint8_t scene_t[max_scene_x][max_scene_z][max_scene_y];

// scene item type
// used in internal asm functions, do not edit
typedef struct scene_item_t {
    uint8_t id, x, y, n;
    uint16_t coords;
    struct scene_item_t * next;
} scene_item_t;
// sizeof(scene_item_t) must be 8
check_size(scene_item_t, 8);

// global world item
typedef struct world_item_t {
    struct world_item_t * N, * E, * S, * W;
    struct scene_item_t * room;
    uint8_t room_bank;
} world_item_t;

// pointer to tiles
extern const uint8_t * __tiles;

// ititialize tiles
void initialize_tiles(const uint8_t * tiles, const uint8_t * empty);

// redraws the scene
void redraw_scene(scene_item_t * scene) PRESERVES_REGS(b, c);

// draws masked bitmap to x, y
void draw_masked_bitmap_XY(uint8_t x, uint8_t y, const uint8_t * spr, const uint8_t * mask) OLDCALL PRESERVES_REGS(b, c);

// erase item (draws empty bitmap)
void erase_item(scene_item_t * item) PRESERVES_REGS(b, c);

// removes item from the item into the scene
void remove_scene_item(scene_item_t * scene, scene_item_t * new_item);

// place the item into the scene
void place_scene_item(scene_item_t * scene, scene_item_t * new_item);

// copy pre-built scene
uint8_t copy_scene(uint8_t bank, const scene_item_t * sour, scene_item_t * dest);

// clear 3D map
void clear_map(scene_t * dest) PRESERVES_REGS(b, c);

// decompress scene to 3D map
void scene_to_map(const scene_item_t * sour, scene_t * dest);

#endif