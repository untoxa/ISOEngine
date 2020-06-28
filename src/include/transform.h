#ifndef TRANSFORM_H_INCLUDE
#define TRANSFORM_H_INCLUDE

#include <gb/gb.h>

enum rotate_dir { ROT_CCW, ROT_CW };

void rotate_scene(enum rotate_dir dir) __banked;
void rotate_scene_coords(enum rotate_dir dir, UBYTE * x, UBYTE * y) __banked;

#endif