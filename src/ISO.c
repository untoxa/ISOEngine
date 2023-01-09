#include <gbdk/platform.h>
#include <stdint.h>

#include "globals.h"
#include "nonintrinsic.h"
#include "shadow.h"
#include "mapping.h"
#include "clipping.h"
#include "scenes.h"
#include "multiple.h"
#include "effects.h"
#include "transform.h"

#include "rooms.h"

#include "scene_resources.h"
#include "misc_resources.h"

#define try_change_room(dir, action) ((dir) ? (position = (dir), (action), 1u) : 0u)

// set initial position in the global world
const world_item_t * position = &world[0];

// enemies
#define enemies_count 2
item_bitmap_t enemies_bkg[enemies_count];
scene_item_t enemies_itm[enemies_count] = {
    {.id=0x0bu, .n=0, .next=0},
    {.id=0x0bu, .n=0, .next=0}
};
clip_item_t enemies[enemies_count] = {
    {.x=3, .y=0, .z=0, .flags=0, .item_bkg=&enemies_bkg[0], .scene_item=&enemies_itm[0]},
    {.x=2, .y=3, .z=0, .flags=0, .item_bkg=&enemies_bkg[1], .scene_item=&enemies_itm[1]}
};
const scene_item_t * enemies_room = r2; // enemies only exist in room2
#define enemies_exist (position->room == enemies_room)
#define iter_enemies(key, what) for (uint8_t key = 0; key < enemies_count; key++) (what)
#define rev_iter_enemies(key, what) for (int8_t key = enemies_count; key != 0; key--) (what)

// player item
item_bitmap_t player_bkg;
scene_item_t player_item = {.id=0x0cu, .n=0, .next=0};
clip_item_t player = {.x=8, .y=0, .z=0, .flags=0, .item_bkg=&player_bkg, .scene_item=&player_item};

uint8_t joy, redraw, falling, room_changed;
static uint8_t tmp;
static uint8_t scene_count;
static enum scroll_dir sc_dir;

static enum scroll_dir anim_dir;
static uint8_t anim_ids[5][2] = {
    {0x00u, 0x00u},
    {0x0eu, 0x0cu},
    {0x0du, 0x0cu},
    {0x0eu, 0x0cu},
    {0x0du, 0x0cu}
};
static uint8_t anim_fall[2] = {0x0fu, 0x0cu};
static uint8_t anim_climb[5][2] = {
    {0x00u, 0x00u},
    {0x0eu, 0x0cu},
    {0x0du, 0x0cu},
    {0x0eu, 0x0cu},
    {0x0du, 0x0cu}
};

uint16_t old_time = 0;

void move_enemies() {
    static uint8_t change_dir;
    static clip_item_t * enemy;
    enemy = enemies;
    for (uint8_t i = 0u; i < enemies_count; i++) {
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

void redraw_all(uint8_t room_changed) {
    if (scene_count) {
        clear_dirty_rows();
        if (!room_changed) {
            // restore background under the enemies
            if enemies_exist rev_iter_enemies(i, restore_item_bkg(enemies[i-1]));
            // restore background under the player
            restore_item_bkg(player);
        }

        // set the new position of the player item
        update_multiple_items_pos(&player, 1);
        // set the new position for each enemy item
        update_multiple_items_pos(enemies, enemies_count);

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
        //copy_tiles();
        copy_dirty_tiles();
    } else {
        if (sc_dir == SC_NONE) copy_tiles(); else scroll_out(sc_dir, 1, 2);
    }
}

void main() {
    ENABLE_RAM;
    SET_RAM_BANK(0);
    SET_ROM_BANK(BANK(scene_resources));

    initialize_tiles(scene_resources.data, empty_tiles);

    // clear the shadow buffer
    clear_shadow_buffer();

    // clear the screen
    set_bkg_tiles(0, 0, 20, 18, shadow_buffer);

    // initialize the player
    player.scene_item->x = to_x(player.x, player.y, player.z),
    player.scene_item->y = to_y(player.x, player.y, player.z),
    player.scene_item->coords = to_coords(player.x, player.y, player.z);

    // copy scene to RAM
    scene_count = copy_scene(position->room_bank, position->room, scene_items);

    // if scene not empty
    if (scene_count) {
        // decompress scene to 3D map
        scene_to_map(scene_items, &collision_buf);

        // redraw the scene into the shadow buffer
        redraw_scene(scene_items);

    } else clear_map(&collision_buf);
    // copy the tiles into vram
    copy_tiles();

    // save background under the player
    save_item_bkg(player);
    // save background under the enemies
    if enemies_exist iter_enemies(i, save_item_bkg(enemies[i]));

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
                player.z--;
                player.scene_item->id = anim_fall[0];
                redraw_all(0);
                player.scene_item->id = anim_fall[1];
                redraw_all(0);
                falling = 1u;
            }
        }
        // walk
        if (!falling) {
            joy = joypad();
            if (joy & J_LEFT) {
                if (!player.x) {
                    room_changed = try_change_room(position->W, (player.x = max_scene_x - 1u, sc_dir = SC_WEST)); // go west
                } else {
                    tmp = collision_buf[player.x - 1u][player.z][player.y];
                    if ((tmp == 0u) || (tmp == 1u) || (tmp == 9u)) {
                        player.scene_item->id = anim_ids[SC_WEST][0];
                        redraw_all(0);
                        player.x--;
                        player.scene_item->id = anim_ids[SC_WEST][1];
                        redraw_all(0);
                    } else if (tmp == 5u) {
                        player.scene_item->id = anim_climb[SC_WEST][0];
                        redraw_all(0);
                        player.x--;
                        player.z++;
                        player.scene_item->id = anim_climb[SC_WEST][1];
                        redraw_all(0);
                    }
                }
                anim_dir = SC_WEST;
            } else if (joy & J_RIGHT) {
                if (player.x == (max_scene_x - 1u)) {
                    room_changed = try_change_room(position->E, (player.x = 0u, sc_dir = SC_EAST)); // go east
                } else {
                    tmp = collision_buf[player.x + 1u][player.z][player.y];
                    if ((tmp == 0u) || (tmp == 1u) || (tmp == 9u)) {
                        player.x++;
                        player.scene_item->id = anim_ids[SC_EAST][0];
                        redraw_all(0);
                        player.scene_item->id = anim_ids[SC_EAST][1];
                        redraw_all(0);
                    } else if (tmp == 5u) {
                        player.x++;
                        player.scene_item->id = anim_climb[SC_EAST][0];
                        redraw_all(0);
                        player.z++;
                        player.scene_item->id = anim_climb[SC_EAST][1];
                        redraw_all(0);
                    }
                }
            } else if (joy & J_UP) {
                if (player.y == (max_scene_y - 1u)) {
                    room_changed = try_change_room(position->N, (player.y = 0u, sc_dir = SC_NORTH)); // go north
                } else {
                    tmp = collision_buf[player.x][player.z][player.y + 1u];
                    if ((tmp == 0u) || (tmp == 1u) || (tmp == 9u)) {
                        player.scene_item->id = anim_ids[SC_NORTH][0];
                        redraw_all(0);
                        player.y++;
                        player.scene_item->id = anim_ids[SC_NORTH][1];
                        redraw_all(0);
                    } else if (tmp == 5u) {
                        player.scene_item->id = anim_climb[SC_NORTH][0];
                        redraw_all(0);
                        player.y++;
                        player.z++;
                        player.scene_item->id = anim_climb[SC_NORTH][1];
                        redraw_all(0);
                    }
                }
            } else if (joy & J_DOWN) {
                if (!player.y) {
                    room_changed = try_change_room(position->S, (player.y = max_scene_y - 1u, sc_dir = SC_SOUTH)); // go south
                } else {
                    tmp = collision_buf[player.x][player.z][player.y - 1u];
                    if ((tmp == 0u) || (tmp == 1u) || (tmp == 9u)) {
                        player.y--;
                        player.scene_item->id = anim_ids[SC_SOUTH][0];
                        redraw_all(0);
                        player.scene_item->id = anim_ids[SC_SOUTH][1];
                        redraw_all(0);
                    } else if (tmp == 5u) {
                        player.y--;
                        player.scene_item->id = anim_climb[SC_SOUTH][0];
                        redraw_all(0);
                        player.z++;
                        player.scene_item->id = anim_climb[SC_SOUTH][1];
                        redraw_all(0);
                    }
                }
            } else if (joy & J_SELECT) {
                // rotate scene ccw
                rotate_scene(ROT_CCW);
                rotate_scene_coords(ROT_CCW, &player.x, &player.y);
                // clear the shadow buffer
                clear_shadow_buffer();
                // redraw everything
                sc_dir = SC_NONE;
                redraw_all(1);
            }
        }
        if (!room_changed) {
            // move enemies
            if (enemies_exist && ((sys_time - old_time) > 7)) {
                old_time = sys_time;
                move_enemies();
                redraw = 1u;
            }
        } else {
            // clear the shadow buffer
            clear_shadow_buffer();
            // decompress scene to 3D map
            scene_count = copy_scene(position->room_bank, position->room, scene_items);
            if (scene_count) scene_to_map(scene_items, &collision_buf); else clear_map(&collision_buf);
            redraw = 1u;
        }
        // redraw
        if (redraw) {
            redraw_all(room_changed);
        } else wait_vbl_done();

        room_changed = redraw = 0u;
    }
}