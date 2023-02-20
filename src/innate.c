#include "innate.h"
#include "gpl.h"
#include "popup.h"
#include "player.h"
#include "region-manager.h"
#include <stdio.h>

#define ITEM_COINS0 (1388)
#define ITEM_COINS1 (1389)
#define ITEM_COINS2 (1384)

extern sol_status_t sol_innate_is_door(sol_dude_t *dude) {
    return
        (abs(dude->ds_id) >= 1131 && abs(dude->ds_id) <= 1181)
        || (abs(dude->ds_id) >= 2051 && abs(dude->ds_id) <= 2200)
        || (abs(dude->ds_id) >= 2400 && abs(dude->ds_id) <= 2949)
        ? SOL_SUCCESS
        : SOL_NOT_FOUND;
}

static sol_entity_t* find_twin(sol_entity_t *door) {
    sol_region_t *reg;
    sol_entity_t *twin_door;
    if (!door) { return NULL; }


    sol_region_manager_get_current(&reg);
    sol_region_find_entity_by_id(reg, door->ds_id + 1, &twin_door);
    if (twin_door && (twin_door->mapx == door->mapx || twin_door->mapy == door->mapy)) {
        return twin_door;
    }

    sol_region_find_entity_by_id(reg, door->ds_id - 1, &twin_door);
    if (twin_door && (twin_door->mapx == door->mapx || twin_door->mapy == door->mapy)) {
        return twin_door;
    }

    return NULL;
}

static void set_door_blocks(sol_dude_t *door, int door_is_open) {
    sol_region_t *reg;
    sol_entity_t *twin_door = find_twin(door);
    sol_sprite_info_t info;

    sol_region_manager_get_current(&reg);
    if (!twin_door) {
        error("Unable to find twin for door!\n");
        return;
    }

    // Something is backwards...
    sol_status_check(sol_sprite_get_info(door->anim.spr, &info), "Unable to get menu options sprite info");
    if (door->mapx == twin_door->mapx) {
        for (int diff = 0; diff < (info.w + 1) / (16 * settings_zoom()); diff++) {
        for (int i = door->mapy + 1; i < twin_door->mapy; i++) {
            if (door_is_open) {
                sol_region_clear_block(reg, i, door->mapx + diff, MAP_BLOCK);
            } else {
                sol_region_set_block(reg, i, door->mapx + diff, MAP_BLOCK);
            }
        }
        }
    } else {
        for (int diff = 0; diff < (info.h + 0) / (16 * settings_zoom()); diff++) {
        for (int i = door->mapx + 1; i < twin_door->mapx; i++) {
            if (door_is_open) {
                sol_region_clear_block(reg, door->mapy - diff, i, MAP_BLOCK);
            } else {
                sol_region_set_block(reg, door->mapy - diff, i, MAP_BLOCK);
            }
        }
        }
    }
}

extern sol_status_t sol_innate_activate_door(sol_dude_t *door) {
    sol_sprite_info_t info;
    sol_status_check(sol_sprite_get_info(door->anim.spr, &info), "Unable to get door's sprite info");
    enum sol_entity_action_e action = info.current_frame == 0 ? EA_DOOR_OPEN : EA_DOOR_CLOSE;
    sol_entity_animation_get_scmd(door, 0, 0, action, &door->anim.scmd);
    set_door_blocks(door, action == EA_DOOR_OPEN);
    sol_entity_animation_list_add(&(door->actions), EA_SCMD, door, door, NULL, 30);

    return SOL_SUCCESS;
}

static sol_status_t custom_action(sol_dude_t *dude) {
    ds1_item_t ds1_item;
    sol_entity_t *active;
    sol_region_t *reg;
    char msg[1024];

    if (sol_innate_is_door(dude) == SOL_SUCCESS) {
        sol_entity_t *door = dude;
        //printf("DOOR: %d, %d, %d\n", dude->ds_id, dude->mapx, dude->mapy);
        // The GPL appears to only operate on one of the doors
        // So we have to find the other...
        sol_entity_t *twin_door = find_twin(door);

        sol_innate_activate_door(door);
        if (twin_door) {
            sol_innate_activate_door(twin_door);
        }
        return SOL_SUCCESS;
    }

    switch(abs(dude->ds_id)) {
        case ITEM_COINS0:
        case ITEM_COINS1:
        case ITEM_COINS2:
            sol_map_clear_highlight();
            ssi_item_load(&ds1_item, dude->ds_id);
            sprintf(msg, "YOU FIND %dcp\n", ds1_item.value); 
            sol_window_push(&popup_window, 100, 75);
            sol_popup_quick_message(msg);
            //sol_player_get_active(&active);
            //active->cp + ds1_item.value;
            //printf("->%s\n", ds1_item->name);
            sol_region_manager_get_current(&reg);
            sol_region_remove_entity(reg, dude);
            sol_entity_free(dude);
            return SOL_SUCCESS;
    }

    return SOL_NO_CUSTOM_ACTION;
}

extern sol_status_t sol_innate_action(sol_dude_t *dude) {
    if (!dude) { return SOL_UNKNOWN_ERROR; }
    if (custom_action(dude) == SOL_SUCCESS) { return SOL_SUCCESS; }

    sol_gpl_request_impl(5, dude->ds_id, -1, -1);
    return SOL_SUCCESS;
}
