#ifndef GLOBALS_H_INCLUDE
#define GLOBALS_H_INCLUDE

#define viewport_height 13
#define viewport_width 18

#define max_scene_x 9u
#define max_scene_y 9u
#define max_scene_z 4u

#define to_x(x,y,z) ((x) + (y))
#define to_y(x,y,z) ((7u * 8u) + ((x) << 2u) - ((y) << 2u) - ((z) << 3u))
#define to_coords(x,y,z) (((x) << 8u) | ((8u - (y)) << 4u) | (z))
#define from_coords(coord, x, y, z) (x = ((coord) >> 8u), y = (8u - ((coord) >> 4u) & 0x0f), z = ((coord) & 0x0f))

#define add_check__(a,b) add_check___(a,b)
#define add_check___(a,b) check_##a##_##b
#define check_size(typ,sz) typedef char add_check__(typ,__LINE__)[ (sizeof(typ) == (sz)) ? 1 : -1]

#endif
