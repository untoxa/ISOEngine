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

typedef struct world_item_t {
    struct world_item_t * N, * E, * S, * W;
    struct scene_item_t * room;
} world_item_t;

// global world
const world_item_t world[4] = { 
    {.N = &world[1], .E = &world[2], .S = 0,          .W = 0,          .room = room0 },
    {.N = 0,         .E = &world[3], .S = &world[0],  .W = 0,          .room = room1 },
    {.N = &world[3], .E = 0,         .S = 0,          .W = &world[0],  .room = room2 },
    {.N = 0,         .E = 0,         .S = &world[2],  .W = &world[1],  .room = room3 } 
};

#define try_change_room(dir, action) ((dir) ? (position = (dir), (action), 1u) : 0u)

// set initial position in the global world
const world_item_t * position = &world[0];

// player position
UBYTE px = 8, py = 0, pz = 0;
UBYTE opx, opy, opz;

UBYTE joy, redraw, falling, room_changed;
static UBYTE tmp;
static scene_item_t player;
static UBYTE scene_count;

void main() {
    SWITCH_RAM_MBC1(0);

    SWITCH_ROM_MBC1(scene_resources.bank);
    initialize_tiles(scene_resources.data->data, empty_tiles);
    
    // clear the shadow buffer
    clear_shadow_buffer();
    
    // clear the screen
    set_bkg_tiles(0, 0, 20, 18, shadow_buffer);

    // initialize the player
    opx = px, opy = py, opz = pz;
    player.id = 0x09u, player.x = to_x(px, py, pz), player.y = to_y(px, py, pz), player.n = 0u, player.coords = to_coords(px, py, pz), player.next = 0;

    // copy scene to RAM
    scene_count = copy_scene(position->room, scene_items);

    // if scene not empty
    if (scene_count) {
        // decompress scene to 3D map
        scene_to_map(scene_items, &collision_buf);

        // place the player into the scene
        replace_scene_item(scene_items, &player);

        // redraw the scene into the shadow buffer
        redraw_scene(scene_items);
    } else clear_map(&collision_buf);
    
    // copy the tiles into vram
    copy_tiles();

    // output the viewport
    set_view_port(1, 2);

    SHOW_BKG;

    room_changed = 0;

    while (1) {
        room_changed = redraw = 0u;
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
                if (!px) {
                    room_changed = try_change_room(position->W, px = max_scene_x - 1u); // go west
                } else px--;
            } else if (joy & J_RIGHT) {
                if (px == (max_scene_x - 1u)) {
                    room_changed = try_change_room(position->E, px = 0u); // go east
                } else px++;
            } else if (joy & J_UP) {
                if (py == (max_scene_y - 1u)) {
                    room_changed = try_change_room(position->N, py = 0u); // go north
                } else py++;
            } else if (joy & J_DOWN) {
                if (!py) { 
                    room_changed = try_change_room(position->S, py = max_scene_y - 1u); // go south
                } else py--;
            }
        }
        if (!room_changed) {
            // check collisions
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
        } else {
            // clear the shadow buffer
            clear_shadow_buffer();
            // decompress scene to 3D map     
            scene_count = copy_scene(position->room, scene_items);
            if (scene_count) scene_to_map(scene_items, &collision_buf); else clear_map(&collision_buf);                
            // player is absent in the new scene
            player.n = 0u, player.next = 0, redraw = 1u;
        }
        // redraw
        if (redraw) {
            if (scene_count) {
                // remove player from the screen
                erase_item(&player);
                // set the new position of the player item
                player.x = to_x(px, py, pz), player.y = to_y(px, py, pz), player.coords = to_coords(px, py, pz);
                // move the player in the sorted scene list to the new position
                replace_scene_item(scene_items, &player);
                // redraw the scene
                redraw_scene(scene_items);
            }
            wait_vbl_done();
            copy_tiles();
            opx = px, opy = py, opz = pz;
        } else wait_vbl_done();
    }
}