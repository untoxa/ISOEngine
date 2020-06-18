#ifndef MAPPING_H_INCLUDE
#define MAPPING_H_INCLUDE

#include <gb/gb.h>

#define used_tiles_count 170

typedef struct tiledesc_t {
    unsigned char * data;
    UBYTE ofs, count;
} tiledesc_t;

extern const unsigned char viewport_map[];
void set_view_port(UBYTE x, UBYTE y) __banked;

extern const tiledesc_t * used_tile_range;
void copy_tiles() __nonbanked;
void copy_dirty_tiles() __nonbanked;
void copy_tiles_row(UBYTE row) __nonbanked;

#endif