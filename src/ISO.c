#include <gb/gb.h>

#if defined(DEBUGGING) || defined(PROFILING)
#include <gb/bgb_emu.h>
#endif
#ifdef DEBUGGING
#include <stdio.h>
char buf[0x20];
#endif

#include "globals.h"
#include "shadow.h"
#include "mapping.h"
#include "scenes.h"

#include "rooms.h"

#include "scene_resources.h"
#include "misc_resources.h"

// player position
UBYTE px = 8, py = 0, pz = 0;
UBYTE opx, opy, opz;

// pointer to tile resources
const unsigned char * tiles;

static UBYTE __put_map_x, __put_map_y, __put_map_id;

void put_map() { 
    static UBYTE i, oy, dy;
    static unsigned char * data1, * data2, * spr, * mask, * limit;
        
    if (__put_map_y < ((viewport_height - 1) * 8)) {       
        oy = __put_map_y >> 3u;
        
        if (__put_map_id != 0xffu) {
            spr = (unsigned char *)&tiles[(int)__put_map_id << 7u];
            mask = spr + 0x40u;
        }  else {
            spr = mask = (unsigned char *)empty_tiles;
        }
               
        dy = (__put_map_y & 7u);
           
        data1 = (unsigned char *)shadow_rows[oy];
        data1 += ((WORD)(__put_map_x - 1) << 4u);
        data1 += (dy << 1u);
        data2 = data1 + 16u;
        
        if (data1 > shadow_buffer_limit) return;
        
        for (i = 0u; i < 0x10u; i++) {
            *data1++ = *data1 & *mask++ | *spr++;
            *data1++ = *data1 & *mask++ | *spr++;
            dy++;
            if (dy == 8u) {
                dy = 0; 
                data1 += ((viewport_width - 1) * 16);
                if (data1 > shadow_buffer_limit) break; 
            }
        }
        
        dy = (__put_map_y & 7u);  

        for (i = 0u; i < 0x10u; i++) {
            *data2++ = *data2 & *mask++ | *spr++;
            *data2++ = *data2 & *mask++ | *spr++;
            dy++;
            if (dy == 8u) {
                dy = 0; 
                data2 += ((viewport_width - 1) * 16);
                if (data2 > shadow_buffer_limit) break; 
            }
        }
    }
}

void redraw_scene(scene_item_t * scene) {
    static UWORD plc;
    static UBYTE plx, ply, not_drawn;
    static scene_item_t * item;

    item = scene;

    __put_map_x = to_x(opx, opy, opz), __put_map_y = to_y(opx, opy, opz), __put_map_id = 0xffu;
    put_map();

    plx = to_x(px, py, pz); ply = to_y(px, py, pz);
    plc = to_coords(px, py, pz);
    not_drawn = 1;
    while (item) {
        #ifdef DEBUGGING
            BGB_MESSAGE_FMT(buf, "%d ? %d n: %x", plc, item->coords, item->next);
        #endif
        if ((not_drawn) && (plc < item->coords)) { 
            __put_map_x = plx, __put_map_y = ply, __put_map_id = 0x09u;
            put_map();  
            not_drawn = 0; 
        }
        __put_map_x = item->x, __put_map_y = item->y, __put_map_id = item->id;
        put_map();
        item = item->next;
    }
    if (not_drawn) { 
        __put_map_x = plx, __put_map_y = ply, __put_map_id = 0x09u;
        put_map();
    }
    #ifdef DEBUGGING
        BGB_MESSAGE("done");
    #endif    
}

UBYTE joy, redraw, falling;
static UBYTE tmp;
static UWORD sz;
static unsigned char * shadow_ptr;

void main() {
    SWITCH_RAM_MBC1(0);

    SWITCH_ROM_MBC1(scene_resources.bank);
    tiles = scene_resources.data->data;
    
    // clear the shadow buffer
    sz = (viewport_height + 1) * viewport_width * 16, shadow_ptr = shadow_buffer;
    while(sz) *shadow_ptr++ = 0u, sz--;

    opx = px, opy = py, opz = pz;
    
    // clear the screen
    set_bkg_tiles(0, 0, 20, 18, shadow_buffer);

    // copy scene to RAM
    scene_count = copy_scene(room0, scene_items);
    // decompress scene to 3D map
    scene_to_map(scene_items, &collision_buf);

    // redraw the scene into the shadow buffer
    if (scene_count) redraw_scene(scene_items);
    // copy the tiles into vram
    copy_tiles();

    // output the viewport
    set_view_port(1, 2);

    SHOW_BKG;

    while (1) {
        // fall
        falling = 0;
        if (pz > 0) {
            tmp = collision_buf[px][pz - 1][py];
            if ((tmp == 0u) || (tmp == 1u) || (tmp == 9u)) {
                pz--, falling = 1u;
            }
        }
        // walk
        if (!falling) {
            joy = joypad();
            if (joy & J_LEFT) {
                if (px) px--;
            } else if (joy & J_RIGHT) {
                if (px < (max_scene_x - 1)) px++;
            } else if (joy & J_UP) {
                if (py < (max_scene_y - 1)) py++;
            } else if (joy & J_DOWN) {
                if (py) py--;
            }
        }
        // check collisions
        redraw = 0u;
        if (!((opx == px) && (opy == py) && (opz == pz))) {
            tmp = collision_buf[px][pz][py];
            if ((tmp == 0u) || (tmp == 1u) || (tmp == 9u)) {
                redraw = 1u;
            } else if (tmp == 5u) {
                pz++, redraw = 1u;
            } else {
                px = opx, py = opy, pz = opz;
            }
        }
        // redraw
        if (redraw) {
            if (scene_count) redraw_scene(scene_items);
            wait_vbl_done();
            copy_tiles();
            opx = px, opy = py, opz = pz;
        } else wait_vbl_done();
    }
}