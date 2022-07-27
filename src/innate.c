#include "innate.h"
#include "gpl.h"
#include "popup.h"
#include "player.h"
#include "region-manager.h"
#include <stdio.h>

#define ITEM_COINS0 (1388)
#define ITEM_COINS1 (1389)
#define ITEM_COINS2 (1384)

static sol_status_t custom_action(dude_t *dude) {
    ds1_item_t ds1_item;
    char msg[1024];

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
