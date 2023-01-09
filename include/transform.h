#ifndef TRANSFORM_H_INCLUDE
#define TRANSFORM_H_INCLUDE

#include <gbdk/platform.h>
#include <stdint.h>

enum rotate_dir { ROT_CCW, ROT_CW };

void rotate_scene(enum rotate_dir dir) BANKED;
void rotate_scene_coords(enum rotate_dir dir, uint8_t * x, uint8_t * y) BANKED;

#endif