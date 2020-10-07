#ifndef SCENES_H_INCLUDE
#define SCENES_H_INCLUDE

#include <gb/gb.h>

#include "globals.h"

typedef unsigned char scene_t[max_scene_x][max_scene_z][max_scene_y];

// scene item type
// used in internal asm functions, do not edit
typedef struct scene_item_t {
    UBYTE id, x, y, n;
    UWORD coords;
    struct scene_item_t * next;
} scene_item_t;
// sizeof(scene_item_t) must be 8
check_size(scene_item_t, 8);

// global world item
typedef struct world_item_t {
    struct world_item_t * N, * E, * S, * W;
    struct scene_item_t * room;
    UBYTE room_bank;
} world_item_t;

// pointer to tiles
extern const unsigned char * __tiles;

// ititialize tiles
void initialize_tiles(const unsigned char * tiles, const unsigned char * empty);

// redraws the scene
void redraw_scene(scene_item_t * scene) __preserves_regs(b, c);

// draws masked bitmap to x, y
void draw_masked_bitmap_XY(UBYTE x, UBYTE y, const unsigned char * spr, const unsigned char * mask) __preserves_regs(b, c);

// erase item (draws empty bitmap) 
void erase_item(scene_item_t * item) __preserves_regs(b, c);

// removes item from the item into the scene
void remove_scene_item(scene_item_t * scene, scene_item_t * new_item);

// place the item into the scene
void place_scene_item(scene_item_t * scene, scene_item_t * new_item);

// copy pre-built scene
UBYTE copy_scene(UBYTE bank, const scene_item_t * sour, scene_item_t * dest);

// clear 3D map
void clear_map(scene_t * dest) __preserves_regs(b, c);

// decompress scene to 3D map
void scene_to_map(const scene_item_t * sour, scene_t * dest);

#endif