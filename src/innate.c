#include "innate.h"
#include "gpl.h"
#include "popup.h"
#include "player.h"
#include "region-manager.h"
#include <stdio.h>

#define ITEM_COINS0 (1388)
#define ITEM_COINS1 (1389)
#define ITEM_COINS2 (1384)

static int is_door(dude_t *dude) {
    return
        (abs(dude->ds_id) >= 1131 && abs(dude->ds_id) <= 1181)
        || (abs(dude->ds_id) >= 2051 && abs(dude->ds_id) <= 2200)
        || (abs(dude->ds_id) >= 2400 && abs(dude->ds_id) <= 2949);
}

static sol_status_t custom_action(dude_t *dude) {
    ds1_item_t ds1_item;
    char msg[1024];

    if (is_door(dude)) {
        entity_t *door = dude;
        //printf("DOOR: %d, %d, %d\n", dude->ds_id, dude->mapx, dude->mapy);
        entity_t *twin_door = sol_region_find_entity_by_id(sol_region_manager_get_current(), dude->ds_id + 1);
        if (!(twin_door->mapx == door->mapx || twin_door->mapy == door->mapy)) {
            twin_door = sol_region_find_entity_by_id(sol_region_manager_get_current(), dude->ds_id - 1);
            if (!(twin_door->mapx == door->mapx || twin_door->mapy == door->mapy)) {
                twin_door = NULL;
            }
        }
        sol_sprite_get_frame(door->anim.spr);
        door->anim.scmd = entity_animation_get_scmd(door, 0, 0,
                sol_sprite_get_frame(door->anim.spr) == 0
                ? EA_DOOR_OPEN
                : EA_DOOR_CLOSE);
        entity_animation_list_add(&(door->actions), EA_SCMD, door, door, NULL, 30);
        if (twin_door) {
            twin_door->anim.scmd = door->anim.scmd;
            entity_animation_list_add(&(twin_door->actions), EA_SCMD, door, door, NULL, 30);
        }
        return SOL_SUCCESS;
    }

    switch(abs(dude->ds_id)) {
        case ITEM_COINS0:
        case ITEM_COINS1:
        case ITEM_COINS2:
            ssi_item_load(&ds1_item, dude->ds_id);
            sprintf(msg, "YOU FIND %dcp\n", ds1_item.value); 
            sol_window_push(&popup_window, 100, 75);
            sol_popup_quick_message(msg);
            sol_player_get_active()->cp + ds1_item.value;
            //printf("->%s\n", ds1_item->name);
            sol_region_remove_entity(sol_region_manager_get_current(), dude);
            entity_free(dude);
            return SOL_SUCCESS;
    }

    return SOL_FAILURE;
}

extern sol_status_t sol_innate_action(dude_t *dude) {
    if (!dude) { return SOL_FAILURE; }
    if (custom_action(dude) == SOL_SUCCESS) { return SOL_SUCCESS; }

    gpl_request_impl(5, dude->ds_id, -1, -1);
    return SOL_SUCCESS;
}
