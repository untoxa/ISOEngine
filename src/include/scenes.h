#ifndef SCENES_H_INCLUDE
#define SCENES_H_INCLUDE

#include <gb/gb.h>

#include "globals.h"

typedef unsigned char scene_t[max_scene_x][max_scene_z][max_scene_y];

typedef struct scene_item_t {
    UBYTE id, x, y;
    UWORD coords;
    struct scene_item_t * next;
} scene_item_t;

// ititialize tiles
void initialize_tiles(const unsigned char * tiles, const unsigned char * empty);

// redraws the scene
void redraw_scene(scene_item_t * scene);

// draws an item at XYZ
void draw_item_XYZ(UBYTE x, UBYTE y, UBYTE z, UBYTE id);

// (re)places the item into scene
void replace_item(scene_item_t * scene, scene_item_t * new_item);

// copy pre-built scene
UBYTE copy_scene(const scene_item_t * sour, scene_item_t * dest);

// decompress scene to 3D map
void scene_to_map(const scene_item_t * sour, scene_t * dest);

#endif