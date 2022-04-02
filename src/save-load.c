#include "arbiter.h"
#include "save-load.h"
#include "player.h"
#include "sol-lua-manager.h"
#include "gpl.h"
#include "map.h"
#include "gameloop.h"
#include "narrate.h"
#include "combat-region.h"
#include "region-manager.h"
#include "trigger.h"
#include <stdio.h>

static int save_item(FILE *file, item_t *item, const char *name) {
    fprintf(file, "%s.ds_id = %d\n", name, item->ds_id);
    fprintf(file, "-- Need to finish saving the item...\n");
    return 1;
}

static int write_entity(FILE *file, entity_t *entity, const char *name) {
    char buf[128];
    fprintf(file, "%s.name = \"%s\"\n", name, entity->name);
    fprintf(file, "%s.size = %d\n", name, entity->size);
    fprintf(file, "%s.race = %d\n", name, entity->race);
    fprintf(file, "%s.gender = %d\n", name, entity->gender);
    fprintf(file, "%s.alignment = %d\n", name, entity->alignment);
    fprintf(file, "%s.allegiance = %d\n", name, entity->allegiance);
    fprintf(file, "%s.object_flags = %d\n", name, entity->object_flags);
    fprintf(file, "%s.direction = %d\n", name, entity->direction);
    fprintf(file, "%s.region = %d\n", name, entity->region);
    fprintf(file, "%s.mapx = %d\n", name, entity->mapx);
    fprintf(file, "%s.mapy = %d\n", name, entity->mapy);
    fprintf(file, "%s.mapz = %d\n", name, entity->mapz);
    fprintf(file, "%s.sound_fx = %d\n", name, entity->sound_fx);
    fprintf(file, "%s.attack_sound = %d\n", name, entity->attack_sound);
    fprintf(file, "%s.combat_status = %d\n", name, entity->combat_status);
    fprintf(file, "%s.stats.str = %d\n", name, entity->stats.str);
    fprintf(file, "%s.stats.dex = %d\n", name, entity->stats.dex);
    fprintf(file, "%s.stats.con = %d\n", name, entity->stats.con);
    fprintf(file, "%s.stats.intel = %d\n", name, entity->stats.intel);
    fprintf(file, "%s.stats.wis = %d\n", name, entity->stats.wis);
    fprintf(file, "%s.stats.cha = %d\n", name, entity->stats.cha);
    fprintf(file, "%s.stats.hp = %d\n", name, entity->stats.hp);
    fprintf(file, "%s.stats.high_hp = %d\n", name, entity->stats.high_hp);
    fprintf(file, "%s.stats.psp = %d\n", name, entity->stats.psp);
    fprintf(file, "%s.stats.high_psp = %d\n", name, entity->stats.high_psp);
    fprintf(file, "%s.stats.base_ac = %d\n", name, entity->stats.base_ac);
    fprintf(file, "%s.stats.base_move = %d\n", name, entity->stats.base_move);
    fprintf(file, "%s.stats.base_thac0 = %d\n", name, entity->stats.base_thac0);
    for (int i = 0; i < 10; i++) {
        fprintf(file, "%s.stats.wizard%d.amt = %d\n", name, i+1, entity->stats.wizard[i].amt);
        fprintf(file, "%s.stats.wizard%d.max = %d\n", name, i+1, entity->stats.wizard[i].max);
        fprintf(file, "%s.stats.priest%d.amt = %d\n", name, i+1, entity->stats.priest[i].amt);
        fprintf(file, "%s.stats.priest%d.max = %d\n", name, i+1, entity->stats.priest[i].max);
    }
    fprintf(file, "%s.stats.saves.paralysis = %d\n", name, entity->stats.saves.paralysis);
    fprintf(file, "%s.stats.saves.wand = %d\n", name, entity->stats.saves.wand);
    fprintf(file, "%s.stats.saves.petrify = %d\n", name, entity->stats.saves.petrify);
    fprintf(file, "%s.stats.saves.breath = %d\n", name, entity->stats.saves.breath);
    fprintf(file, "%s.stats.saves.spell = %d\n", name, entity->stats.saves.spell);

    for (int i = 0; i < 3; i++) {
        fprintf(file, "%s.class%d.current_xp = %d\n", name, i, entity->class[i].current_xp);
        fprintf(file, "%s.class%d.high_xp = %d\n", name, i, entity->class[i].high_xp);
        fprintf(file, "%s.class%d.class = %d\n", name, i, entity->class[i].class);
        fprintf(file, "%s.class%d.level = %d\n", name, i, entity->class[i].level);
        fprintf(file, "%s.class%d.high_level = %d\n", name, i, entity->class[i].high_level);
        // TODO: Need to add psionic here....
    }
    fprintf(file, "%s.ability.hunt = %d\n", name, entity->abilities.hunt);

    if (entity->inv) {
        sprintf(buf, "%s.inventory.arm", name); save_item(file, entity->inv + 0, buf);
        sprintf(buf, "%s.inventory.ammo", name); save_item(file, entity->inv + 1, buf);
        sprintf(buf, "%s.inventory.missile", name); save_item(file, entity->inv + 2, buf);
        sprintf(buf, "%s.inventory.hand0", name); save_item(file, entity->inv + 3, buf);
        sprintf(buf, "%s.inventory.finger0", name); save_item(file, entity->inv + 4, buf);
        sprintf(buf, "%s.inventory.waist", name); save_item(file, entity->inv + 5, buf);
        sprintf(buf, "%s.inventory.legs", name); save_item(file, entity->inv + 6, buf);
        sprintf(buf, "%s.inventory.head", name); save_item(file, entity->inv + 7, buf);
        sprintf(buf, "%s.inventory.neck", name); save_item(file, entity->inv + 8, buf);
        sprintf(buf, "%s.inventory.chest", name); save_item(file, entity->inv + 9, buf);
        sprintf(buf, "%s.inventory.hand1", name); save_item(file, entity->inv + 10, buf);
        sprintf(buf, "%s.inventory.finger1", name); save_item(file, entity->inv + 11, buf);
        sprintf(buf, "%s.inventory.cloak", name); save_item(file, entity->inv + 12, buf);
        sprintf(buf, "%s.inventory.foot", name); save_item(file, entity->inv + 13, buf);
        for (int i = 0; i < 12; i++) {
            sprintf(buf, "%s.inventory.bp%d", name, i); save_item(file, entity->inv + 14 + i, buf);
        }
    }

    return 1;
}

static int write_players(FILE *file) {
    dude_t *dude;
    for (int i = 0; i < MAX_PCS; i++) {
        dude = sol_player_get(i);
        if (dude) {
            fprintf(file, "p = sol.create_player(%d)\n", i);
            write_entity(file, dude, "p");
        }
    }

    return 1;
}

static int write_region(FILE *file, sol_region_t *reg) {
    fprintf(file, "reg = sol.load_region(%d)\n", reg->map_id);
    fprintf(file, "reg.mas_loaded = 1 -- don't reload MAS\n");
    for (int i = 0; i < MAP_ROWS; i++) {
        fprintf(file, "reg.set_flag_ids(%d, \"", i);
        for (int j = 0; j < MAP_COLUMNS; j++) {
            fprintf(file, "%d ", reg->flags[i][j]);
        }
        fprintf(file, "\")\n");
    }
    return 1;
}

static int write_regions(FILE *file) {
    write_region(file, sol_region_manager_get_current());
    gpl_write_local_state(file);
    gpl_write_global_state(file);
    sol_write_triggers(file);
    return 1;
}

extern int sol_save_to_file(const char *filepath) {
    printf("Need to save to %s\n", filepath);
    FILE *file;
    sol_region_t *reg = sol_region_manager_get_current();
    if (sol_combat_get_current(sol_arbiter_combat_region(reg)) != NULL) { return 0; }
    if (narrate_is_open()) { return 0; }

    file = fopen("quick.sav", "wb");
    if (!file) { return 0; }

    fprintf(file, "function init()\n");
    fprintf(file, "local sol = soloscuro\n");
    write_players(file);
    write_regions(file);
    fprintf(file, "sol.change_region(%d)\n", 42);
    fprintf(file, "sol.load_window(\"map\")\n");
    fprintf(file, "sol.load_window(\"narrate\")\n");
    fprintf(file, "sol.load_window(\"combat\")\n");
    fprintf(file, "end\n");

    fclose(file);
    return 1;
}

extern int sol_load_from_file(const char *filepath) {
    sol_window_clear();
    sol_region_manager_cleanup(1);
    gpl_cleanup();
    sol_player_close();
    sol_trigger_cleanup();
    gpl_init();
    sol_gameloop_init();
    sol_lua_load(filepath);
    sol_center_on_player();
    return 0;
}
