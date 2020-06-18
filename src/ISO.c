#include <gb/gb.h>

#if defined(DEBUGGING) || defined(PROFILING)
#include <gb/bgb_emu.h>
#endif
#ifdef DEBUGGING
#include <stdio.h>
static char buf[0x20];
#endif

#include "globals.h"
#include "shadow.h"
#include "mapping.h"
#include "clipping.h"
#include "scenes.h"
#include "multiple.h"
#include "effects.h"

#include "rooms.h"

#include "scene_resources.h"
#include "misc_resources.h"

#define try_change_room(dir, action) ((dir) ? (position = (dir), (action), 1u) : 0u)

// current bank tracking variable defined in crt0
extern volatile __sfr _current_bank;

// set initial position in the global world
const world_item_t * position = &world[0];

// enemies
#define enemies_count 2
item_bitmap_t enemies_bkg[enemies_count];
scene_item_t enemies_itm[enemies_count] = {{.id=0x0bu, .n=0, .next=0}, {.id=0x0bu, .n=0, .next=0}};
clip_item_t enemies[enemies_count] = {
    {.x=0, .y=0, .z=0, .flags=0, .item_bkg=&enemies_bkg[0], .scene_item=&enemies_itm[0]}, 
    {.x=0, .y=3, .z=0, .flags=0, .item_bkg=&enemies_bkg[1], .scene_item=&enemies_itm[1]}
};
const scene_item_t * enemies_room = r2; // enemies only exist in room2
#define enemies_exist (position->room == enemies_room)
#define iter_enemies(key, what) for (UBYTE key = 0; key < enemies_count; key++) (what)
#define rev_iter_enemies(key, what) for (INT8 key = enemies_count; key != 0; key--) (what)

// player item
item_bitmap_t player_bkg;
scene_item_t player_item = {.id=0x09u, .n=0, .next=0};
clip_item_t player = {.x=8, .y=0, .z=0, .flags=0, .item_bkg=&player_bkg, .scene_item=&player_item};
// player old position
UBYTE opx, opy, opz;

UBYTE joy, redraw, falling, room_changed;
static UBYTE tmp;
static UBYTE scene_count;
static enum scroll_dir sc_dir;

// sys_time is defined by crt0
extern UWORD sys_time;

void move_enemies() {
    static UBYTE change_dir;
    static clip_item_t * enemy;
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

void update_scene_item_coords(clip_item_t * item) {
    item->scene_item->x = to_x(item->x, item->y, item->z), 
    item->scene_item->y = to_y(item->x, item->y, item->z), 
    item->scene_item->coords = to_coords(item->x, item->y, item->z);
}

void main() {
    SWITCH_RAM_MBC1(0);

    _current_bank = scene_resources.bank;
    SWITCH_ROM_MBC1(_current_bank);
    initialize_tiles(scene_resources.data->data, empty_tiles);
    
    // clear the shadow buffer
    clear_shadow_buffer();
    
    // clear the screen
    set_bkg_tiles(0, 0, 20, 18, shadow_buffer);

    // initialize the player
    player.scene_item->x = to_x(player.x, player.y, player.z), 
    player.scene_item->y = to_y(player.x, player.y, player.z), 
    player.scene_item->coords = to_coords(player.x, player.y, player.z);

    opx = player.x, opy = player.y, opz = player.z;

    // copy scene to RAM
    scene_count = copy_scene(position->room, scene_items);

    // if scene not empty
    if (scene_count) {
        // decompress scene to 3D map
        scene_to_map(scene_items, &collision_buf);

        // redraw the scene into the shadow buffer
        redraw_scene(scene_items);
        
    } else clear_map(&collision_buf);
    
    // save background under the player
    save_item_bkg(player);
    // save background under the enemies
    if enemies_exist iter_enemies(i, save_item_bkg(enemies[i]));

    // copy the tiles into vram
    copy_tiles();

    // output the viewport
    set_view_port(1, 2);

    SHOW_BKG;

    room_changed = 0, redraw = 1u;

    while (1) {
        // fall
        falling = 0;
        if (player.z > 0) {
            tmp = collision_buf[player.x][player.z - 1][player.y];
            if ((tmp == 0u) || (tmp == 1u) || (tmp == 9u)) {
                player.z--, falling = 1u;
            }
        }
        // walk
        if (!falling) {
            joy = joypad();
            if (joy & J_LEFT) {
                if (!player.x) {
                    room_changed = try_change_room(position->W, (player.x = max_scene_x - 1u, sc_dir = SC_WEST)); // go west
                } else player.x--;
            } else if (joy & J_RIGHT) {
                if (player.x == (max_scene_x - 1u)) {
                    room_changed = try_change_room(position->E, (player.x = 0u, sc_dir = SC_EAST)); // go east
                } else player.x++;
            } else if (joy & J_UP) {
                if (player.y == (max_scene_y - 1u)) {
                    room_changed = try_change_room(position->N, (player.y = 0u, sc_dir = SC_NORTH)); // go north
                } else player.y++;
            } else if (joy & J_DOWN) {
                if (!player.y) { 
                    room_changed = try_change_room(position->S, (player.y = max_scene_y - 1u, sc_dir = SC_SOUTH)); // go south
                } else player.y--;
            } else if (joy & J_A) {
                waitpadup();
                test_clipping(player.item_bkg);
            } else if (joy & J_B) {
                waitpadup();
                restore_item_bkg(player);
                copy_tiles();
            }
        }
        if (!room_changed) {
            // check collisions
            if (!((opx == player.x) && (opy == player.y) && (opz == player.z))) {
                tmp = collision_buf[player.x][player.z][player.y];
                if ((tmp == 0u) || (tmp == 1u) || (tmp == 9u)) {
                    redraw = 1u;
                } else if (tmp == 5u) {
                    player.z++, redraw = 1u;
                } else {
                    player.x = opx, player.y = opy, player.z = opz;
                }
            }
            // move enemies
            if (enemies_exist && (redraw || (!((UBYTE)sys_time & 7)))) move_enemies();
        } else {
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
                if (!room_changed) {
                    // restore background under the enemies
                    if enemies_exist rev_iter_enemies(i, restore_item_bkg(enemies[i-1]));
                    // restore background under the player
                    restore_item_bkg(player);
                }
                
                // set the new position of the player item
                update_scene_item_coords(&player);
                // set the new position for each enemy item
                if enemies_exist iter_enemies(i, update_scene_item_coords(&enemies[i]));
                                    
                if (room_changed) redraw_scene(scene_items);

                // save background under the player
                save_item_bkg(player);
                // save background under the enemies
                if enemies_exist iter_enemies(i, save_item_bkg(enemies[i]));

                // output enemies
                if enemies_exist iter_enemies(i, draw_item(scene_items, &enemies[i]));
                // output player
                draw_item(scene_items, &player);
            }
            if (!room_changed) {
                wait_vbl_done();
                copy_tiles();                
            } else scroll_out(sc_dir, 1, 2); 
            opx = player.x, opy = player.y, opz = player.z;
        } else wait_vbl_done();
        room_changed = redraw = 0u;
    }
}