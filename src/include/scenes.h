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

// copy pre-built scene
UBYTE copy_scene(const scene_item_t * sour, scene_item_t * dest);

// decompress scene to 3D map
void scene_to_map(const scene_item_t * sour, scene_t * dest);

#endif