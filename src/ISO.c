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


UBYTE joy, redraw, falling;
static UBYTE tmp;
static scene_item_t player;

void main() {
    SWITCH_RAM_MBC1(0);

    SWITCH_ROM_MBC1(scene_resources.bank);
    initialize_tiles(scene_resources.data->data, empty_tiles);
    
    // clear the shadow buffer
    clear_shadow_buffer();
    
    // clear the screen
    set_bkg_tiles(0, 0, 20, 18, shadow_buffer);

    // copy scene to RAM
    scene_count = copy_scene(room0, scene_items);
    // decompress scene to 3D map
    scene_to_map(scene_items, &collision_buf);

    // initialize player and place it into the scene
    opx = px, opy = py, opz = pz;
    player.id = 0x09u, player.x = to_x(px, py, pz), player.y = to_y(px, py, pz), player.coords = to_coords(px, py, pz), player.next = 0;
    replace_scene_item(scene_items, &player);

    // redraw the scene into the shadow buffer
    redraw_scene(scene_items);
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
            if (scene_count) {
                erase_item(&player);
                player.x = to_x(px, py, pz), player.y = to_y(px, py, pz), player.coords = to_coords(px, py, pz);
                replace_scene_item(scene_items, &player);
                redraw_scene(scene_items);
            }
            wait_vbl_done();
            copy_tiles();
            opx = px, opy = py, opz = pz;
        } else wait_vbl_done();
    }
}