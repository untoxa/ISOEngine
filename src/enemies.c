#include "enemies.h"

static moving_item_t * item; 
static scene_item_t * s_item, * m_item;
static UBYTE i;

void remove_multiple_items(scene_item_t * scene, moving_item_t * items, UBYTE count) {
    if (!count) return;
    item = items + (count - 1);    
    for (i = 0; i < count; i++) {
        m_item = item->scene_item;
        if (m_item->n) {
            s_item = scene + (m_item->n - 1);
            s_item->next = m_item->next;
            m_item->n = 0, m_item->next = 0;
        }
        item--;        
    }
}

void place_multiple_items(scene_item_t * scene, moving_item_t * items, UBYTE count) {
    if (!count) return;
    item = items;
    for (i = 0; i < count; i++) {
        place_scene_item(scene, item->scene_item);
        item++;        
    }    
}

void erase_multiple_items(moving_item_t * items, UBYTE count) {
    if (!count) return;
    item = items;
    for (i = 0; i < count; i++) {
        erase_item(item->scene_item);
        item++;        
    }    
}

void update_multiple_items_pos(moving_item_t * items, UBYTE count) {
    if (!count) return;
    item = items;
    for (i = 0; i < count; i++) {
        item->scene_item->x = to_x(item->x, item->y, item->z); 
        item->scene_item->y = to_y(item->x, item->y, item->z); 
        item->scene_item->coords = to_coords(item->x, item->y, item->z);
        item++;        
    }    
}