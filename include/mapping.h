#ifndef MAPPING_H_INCLUDE
#define MAPPING_H_INCLUDE

#include <gbdk/platform.h>
#include <stdint.h>

#define used_tiles_count 170

typedef struct tiledesc_t {
    uint8_t * data;
    uint8_t ofs, count;
} tiledesc_t;

extern const uint8_t viewport_map[];
void set_view_port(uint8_t x, uint8_t y) BANKED;

extern const tiledesc_t * used_tile_range;
void copy_tiles(void) NONBANKED PRESERVES_REGS(b, c);
void copy_dirty_tiles(void) NONBANKED PRESERVES_REGS(b, c);
void copy_tiles_row(uint8_t row);

#endif