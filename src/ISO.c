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
#include "enemies.h"
#include "effects.h"

#include "rooms.h"

#include "scene_resources.h"
#include "misc_resources.h"

#define try_change_room(dir, action) ((dir) ? (position = (dir), (action), 1u) : 0u)

// set initial position in the global world
const world_item_t * position = &world[0];

// enemies
#define enemies_count 2
scene_item_t enemies_itm[enemies_count] = {{.id=0x0bu, .n=0, .next=0}, {.id=0x0bu, .n=0, .next=0}};
moving_item_t enemies[enemies_count] = {{.x=0, .y=0, .z=0, .flags=0, .scene_item=&enemies_itm[0]}, {.x=0, .y=3, .z=0, .flags=0, .scene_item=&enemies_itm[1]}};
const scene_item_t * enemies_room = r2; // enemies only exist in room2
#define enemies_exist (position->room == enemies_room)


// player position
UBYTE px = 8, py = 0, pz = 0;
UBYTE opx, opy, opz;

UBYTE joy, redraw, falling, room_changed;
static UBYTE tmp;
static scene_item_t player;
static UBYTE scene_count;
static enum scroll_dir sc_dir;

// sys_time is defined by crt0
extern UWORD sys_time;

void move_enemies() {
    static UBYTE change_dir;
    static moving_item_t * enemy;
    enemy = enemies;
    for (UBYTE i = 0u; i < enemies_count; i++) {
        change_dir = 0u;
        switch (enemy->flags & 3u) {
            case 0u : {
                if (enemy->x < (max_scene_x - 1u)) {
                    tmp = collision_buf[enemy->x+1u][enemy->z][enemy->y];
                    if ((tmp == 0u) || (tmp == 1u) || (tmp == 9u)) enemy->x++, redraw = 1u; else change_dir = 1u;
                } else change_dir = 1u;
                break;
            }
            case 1u :
                if (enemy->y < (max_scene_y - 1u)) {
                    tmp = collision_buf[enemy->x][enemy->z][enemy->y+1u];
                    if ((tmp == 0u) || (tmp == 1u) || (tmp == 9u)) enemy->y++, redraw = 1u; else change_dir = 1u;
                } else change_dir = 1u;
                break;
            case 2u :
                if (enemy->x) {
                    tmp = collision_buf[enemy->x-1u][enemy->z][enemy->y];
                    if ((tmp == 0u) || (tmp == 1u) || (tmp == 9u)) enemy->x--, redraw = 1u; else change_dir = 1u;
                } else change_dir = 1u;
                break;
            case 3u :
                if (enemy->y) {
                    tmp = collision_buf[enemy->x][enemy->z][enemy->y-1u];
                    if ((tmp == 0u) || (tmp == 1u) || (tmp == 9u)) enemy->y--, redraw = 1u; else change_dir = 1u;
                } else change_dir = 1u;
                break;
        }
        if (change_dir) enemy->flags = (DIV_REG & 3u);
        enemy++;
    }
}

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
        place_scene_item(scene_items, &player);

        // update enemies positions and place them into the scene
        if enemies_exist {
            update_multiple_items_pos(enemies, enemies_count); 
            place_multiple_items(scene_items, enemies, enemies_count);
        }

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
                    room_changed = try_change_room(position->W, (px = max_scene_x - 1u, sc_dir = SC_WEST)); // go west
                } else px--;
            } else if (joy & J_RIGHT) {
                if (px == (max_scene_x - 1u)) {
                    room_changed = try_change_room(position->E, (px = 0u, sc_dir = SC_EAST)); // go east
                } else px++;
            } else if (joy & J_UP) {
                if (py == (max_scene_y - 1u)) {
                    room_changed = try_change_room(position->N, (py = 0u, sc_dir = SC_NORTH)); // go north
                } else py++;
            } else if (joy & J_DOWN) {
                if (!py) { 
                    room_changed = try_change_room(position->S, (py = max_scene_y - 1u, sc_dir = SC_SOUTH)); // go south
                } else py--;
            } else if (joy & J_A) {
                wait_vbl_done;
                SCX_REG += 2, SCY_REG++;
            } else if (joy & J_B) {
                scroll_out(SC_WEST, 1, 2);
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
            // move enemies
            if (enemies_exist && (redraw || (!((UBYTE)sys_time & 7)))) move_enemies();
        } else {
            // remove enemies from the old scene
            if enemies_exist remove_multiple_items(scene_items, enemies, enemies_count);
            // remove player from the old scene
            remove_scene_item(scene_items, &player);
            // clear the shadow buffer
            clear_shadow_buffer();
            // decompress scene to 3D map     
            scene_count = copy_scene(position->room, scene_items);
            if (scene_count) scene_to_map(scene_items, &collision_buf); else clear_map(&collision_buf);                
            redraw = 1u;
        }
        // redraw
        if (redraw) {
            if (scene_count) {
                if enemies_exist {                
                    // erase enemies from the screen
                    erase_multiple_items(enemies, enemies_count);
                    // update enemies positions on screen
                    update_multiple_items_pos(enemies, enemies_count);
                }
                // remove player from the screen
                erase_item(&player);
                // set the new position of the player item
                player.x = to_x(px, py, pz), player.y = to_y(px, py, pz), player.coords = to_coords(px, py, pz);
                
                // remove enemies from the scene
                if enemies_exist remove_multiple_items(scene_items, enemies, enemies_count);
                
                // move the player in the sorted scene list to the new position
                remove_scene_item(scene_items, &player);
                place_scene_item(scene_items, &player);
                
                // place enemies to the scene
                if enemies_exist place_multiple_items(scene_items, enemies, enemies_count);
                
                // redraw the scene
                redraw_scene(scene_items);
            }
            if (!room_changed) {
                wait_vbl_done();
                copy_tiles();                
            } else scroll_out(sc_dir, 1, 2); 
            opx = px, opy = py, opz = pz;
        } else wait_vbl_done();
    }
}