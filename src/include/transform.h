#ifndef TRANSFORM_H_INCLUDE
#define TRANSFORM_H_INCLUDE

#include <gb/gb.h>

enum rotate_dir { ROT_CLOCKWISE, ROT_COUNTERCLOCKWISE};

void rotate_scene(enum rotate_dir dir) __banked;

#endif