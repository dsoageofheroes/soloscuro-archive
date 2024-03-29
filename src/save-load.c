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

#include <ctype.h>
#include <stdio.h>
#include <string.h>

static int save_inventory(FILE *file, sol_entity_t *entity, const char *name) {
    if (!entity->inv) { return 0; }

    for (int i = 0; i < SLOT_END; i++) {
        if (entity->inv[i].ds_id < 0) {
            fprintf(file, "%s.give_ds1_item(%d, %d, %d)\n", name, i,
                          entity->inv[i].item_index, entity->inv[i].ds_id);
        }
    }

    return 1;
}

static void trim_end(char *str) {
    if (!str) { return; }
    size_t len = strlen(str);
    while (!isalpha(str[len])) {
        str[len] = '\0';
        len--;
    }
}

static int write_entity(FILE *file, sol_entity_t *entity, const char *name) {
    sol_sprite_info_t info;

    if (entity->name) {
        trim_end(entity->name);
        fprintf(file, "%s.name = \"%s\"\n", name, entity->name);
    }
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
    fprintf(file, "%s.ds_id = %d\n", name, entity->ds_id);
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
        if (entity->stats.wizard[i].max) {
            fprintf(file, "%s.stats.wizard%d.amt = %d\n", name, i+1, entity->stats.wizard[i].amt);
            fprintf(file, "%s.stats.wizard%d.max = %d\n", name, i+1, entity->stats.wizard[i].max);
        }
        if (entity->stats.priest[i].max) {
            fprintf(file, "%s.stats.priest%d.amt = %d\n", name, i+1, entity->stats.priest[i].amt);
            fprintf(file, "%s.stats.priest%d.max = %d\n", name, i+1, entity->stats.priest[i].max);
        }
    }
    fprintf(file, "%s.stats.saves.paralysis = %d\n", name, entity->stats.saves.paralysis);
    fprintf(file, "%s.stats.saves.wand = %d\n", name, entity->stats.saves.wand);
    fprintf(file, "%s.stats.saves.petrify = %d\n", name, entity->stats.saves.petrify);
    fprintf(file, "%s.stats.saves.breath = %d\n", name, entity->stats.saves.breath);
    fprintf(file, "%s.stats.saves.spell = %d\n", name, entity->stats.saves.spell);

    for (int i = 0; i < 3; i++) {
        fprintf(file, "%s.stats.attack%d.number = %d\n", name, i, entity->stats.attacks[i].number);
        fprintf(file, "%s.stats.attack%d.num_dice = %d\n", name, i, entity->stats.attacks[i].num_dice);
        fprintf(file, "%s.stats.attack%d.sides = %d\n", name, i, entity->stats.attacks[i].sides);
        fprintf(file, "%s.stats.attack%d.bonus = %d\n", name, i, entity->stats.attacks[i].bonus);
        fprintf(file, "%s.stats.attack%d.special = %d\n", name, i, entity->stats.attacks[i].special);
    }

    for (int i = 0; i < 3; i++) {
        if (entity->class[i].current_xp) {
            fprintf(file, "%s.class%d.current_xp = %d\n", name, i, entity->class[i].current_xp);
            fprintf(file, "%s.class%d.high_xp = %d\n", name, i, entity->class[i].high_xp);
            fprintf(file, "%s.class%d.class = %d\n", name, i, entity->class[i].class);
            fprintf(file, "%s.class%d.level = %d\n", name, i, entity->class[i].level);
            fprintf(file, "%s.class%d.high_level = %d\n", name, i, entity->class[i].high_level);
        }
        // TODO: Need to add psionic here....
    }
    fprintf(file, "%s.ability.hunt = %d\n", name, entity->abilities.hunt);
    fprintf(file, "%s.ability.must_go = %d\n", name, entity->abilities.must_go);
    if (entity->abilities.must_go) {
        fprintf(file, "%s.ability.xpos = %d\n", name, entity->abilities.args.pos.x);
        fprintf(file, "%s.ability.ypos = %d\n", name, entity->abilities.args.pos.y);
    }

    save_inventory(file, entity, name);

    sol_status_check(sol_sprite_get_info(entity->anim.spr, &info), "Unable to get sprite info.");
    fprintf(file, "%s.anim.flags = %d\n", name, entity->anim.flags);
    fprintf(file, "%s.anim.pos = %d\n", name, entity->anim.pos);
    fprintf(file, "%s.anim.x = %d\n", name, entity->anim.x);
    fprintf(file, "%s.anim.y = %d\n", name, entity->anim.y);
    fprintf(file, "%s.anim.xoffset = %d\n", name, entity->anim.xoffset);
    fprintf(file, "%s.anim.yoffset = %d\n", name, entity->anim.yoffset);
    fprintf(file, "%s.anim.bmp_id = %d\n", name, entity->anim.bmp_id);
    fprintf(file, "%s.anim.load_frame = %d\n", name, info.current_frame);

    if (entity->anim.scmd_info.gff_idx) {
        if (entity->anim.scmd_info.gff_idx < 0) {
            sol_combat_update_scmd_info(entity);
        }
        fprintf(file, "%s.load_scmd( %d, %d, %d)\n", name, entity->anim.scmd_info.gff_idx,
                  entity->anim.scmd_info.res_id, entity->anim.scmd_info.index);
    }

    return 1;
}

static int write_players(FILE *file) {
    sol_dude_t *dude;
    for (int i = 0; i < MAX_PCS; i++) {
        sol_player_get(i, &dude);
        if (dude) {
            fprintf(file, "p = sol.create_player(%d)\n", i);
            write_entity(file, dude, "p");
        }
    }

    return 1;
}

static int write_region(FILE *file, sol_region_t *reg) {
    fprintf(file, "reg = sol.load_region(%d, 1)\n", reg->map_id);
    //fprintf(file, "reg.assume_loaded = 1 -- don't load region, we got this.\n");
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
    int slot;
    sol_dude_t *entity = NULL;
    sol_region_t *reg;

    sol_region_manager_get_current(&reg);
    write_region(file, reg);
    sol_gpl_write_local_state(file);
    sol_gpl_write_global_state(file);
    sol_write_triggers(file);

    sol_entity_list_for_each(reg->entities, entity) {
        if (sol_player_get_slot(entity, &slot) == SOL_SUCCESS && slot < 0) {
            fprintf(file, "e = sol.create_entity(%d)\n", entity->inv ? 1 : 0);
            write_entity(file, entity, "e");
            fprintf(file, "reg.add_entity(e)\n");
        }
    }

    return 1;
}

extern sol_status_t sol_save_to_file(const char *filepath, const char *name) {
    printf("Need to save to %s\n", filepath);
    sol_combat_region_t *cr = NULL;
    FILE *file;
    sol_region_t *reg;
    sol_dude_t   *dude;

    sol_region_manager_get_current(&reg);
    sol_arbiter_combat_region(reg, &cr);
    if (sol_combat_get_current(cr, &dude) != SOL_SUCCESS && dude != NULL) { return SOL_IN_COMBAT_ERROR; }
    if (sol_narrate_is_open() == SOL_SUCCESS) { return SOL_IN_NARRATE_ERROR; }

    //file = fopen("quick.sav", "wb");
    file = fopen(filepath, "wb");
    if (!file) { return SOL_FILESYSTEM_ERROR; }

    fprintf(file, "name = \"%s\"\n", name);
    fprintf(file, "function init()\n");
    fprintf(file, "local sol = soloscuro\n");
    write_players(file);
    write_regions(file);
    fprintf(file, "sol.change_region(%d)\n", reg->map_id);
    fprintf(file, "sol.load_window(\"map\")\n");
    fprintf(file, "sol.load_window(\"narrate\")\n");
    fprintf(file, "sol.load_window(\"combat\")\n");
    fprintf(file, "end\n");

    fclose(file);
    return SOL_SUCCESS;
}

extern sol_status_t sol_load_get_name(const char *filepath, char *name, const size_t len) {
    FILE *file = fopen(filepath, "rb");
    char c;
    size_t pos = 0;

    if (!file) { return SOL_FILESYSTEM_ERROR; }

    while ((c = fgetc(file)) != EOF && c != '"') {;}
    while ((c = fgetc(file)) != EOF && c != '"') {
        if (pos >= len - 1) { break; }
        name[pos++] = c;
    }
    name[pos] = '\0';
    fclose(file);

    return SOL_SUCCESS;
}

extern sol_status_t sol_load_from_file(const char *filepath) {
    sol_window_clear();
    sol_region_manager_cleanup(1);
    sol_gpl_cleanup();
    sol_player_close();
    sol_trigger_cleanup();
    sol_gpl_init();
    sol_gameloop_init();
    sol_lua_load(filepath);
    return SOL_SUCCESS;
}
