#include "player.h"
#include "region.h"
#include "region-manager.h"
#include "gff.h"
#include "port.h"
#include "lua-region.h"
#include "lua-entity.h"
#include "gameloop.h"
#include "settings.h"
#include "arbiter.h"
#include <string.h>

extern char *strdup(const char *s); // Not in standard.

#define GET_INTEGER_TABLE(a, field) if (!strcmp(str, #field)) { lua_pushinteger(l, a->field); return 1; }
#define GET_STRING_TABLE(a, field) if (!strcmp(str, #field)) { lua_pushstring(l, a->field); return 1; }
#define SET_INTEGER_TABLE(a, field, num) if (!strcmp(str, #field)) { a->field = num; return 0; }
#define SET_STRING_TABLE(a, field, num) if (!strcmp(str, #field)) { if (a->field) { free(a->field); } a->field = strdup(num); return 0; }

extern void* sol_lua_get_userdata(lua_State *l, const int loc) {
    void *ret = NULL;
    luaL_checktype(l, 1, LUA_TTABLE);

    lua_pushstring(l, "ptr__");
    lua_rawget(l, loc);
    ret = lua_touserdata(l, -1);
    lua_pop(l, 1);

    return ret;
}

static void push_table_start(lua_State *l, void *data, const char *name) {
    lua_pushstring(l, name);
    lua_newtable(l);
    lua_pushlightuserdata(l, data);
    lua_setfield(l, -2, "ptr__");
}

static void push_table_end(lua_State *l, const char *metaname) {
    if (metaname) {
        luaL_setmetatable(l, metaname);
    }
    lua_settable(l, -3);
}

static void push_table(lua_State *l, void *data, const char *name, const char *metaname) {
    push_table_start(l, data, name);
    push_table_end(l, metaname);
}

extern void sol_lua_dumpstack (lua_State *L) {
  int top=lua_gettop(L);
  for (int i=-5; i <= top; i++) {
    printf("%d\t%s\t", i, luaL_typename(L,i));
    switch (lua_type(L, i)) {
      case LUA_TNUMBER:
        printf("%g\n",lua_tonumber(L,i));
        break;
      case LUA_TSTRING:
        printf("%s\n",lua_tostring(L,i));
        break;
      case LUA_TBOOLEAN:
        printf("%s\n", (lua_toboolean(L, i) ? "true" : "false"));
        break;
      case LUA_TNIL:
        printf("%s\n", "nil");
        break;
      default:
        printf("%p\n",lua_topointer(L,i));
        break;
    }
  }
}

extern int sol_lua_load_entity (lua_State *l, dude_t *dude) {
    lua_newtable(l); // entity table
    lua_pushlightuserdata(l, dude);
    lua_setfield(l, -2, "ptr__");
    char buf[128];

    push_table_start(l, &(dude->stats), "stats");

    push_table(l, &(dude->stats.attacks[0]), "attack0", "soloscuro.attack");
    push_table(l, &(dude->stats.attacks[1]), "attack1", "soloscuro.attack");
    push_table(l, &(dude->stats.attacks[2]), "attack2", "soloscuro.attack");
    push_table(l, &(dude->stats.saves), "saves", "soloscuro.saves");
    push_table(l, &(dude->stats.wizard[0]), "wizard1", "soloscuro.slots");
    push_table(l, &(dude->stats.wizard[1]), "wizard2", "soloscuro.slots");
    push_table(l, &(dude->stats.wizard[2]), "wizard3", "soloscuro.slots");
    push_table(l, &(dude->stats.wizard[3]), "wizard4", "soloscuro.slots");
    push_table(l, &(dude->stats.wizard[4]), "wizard5", "soloscuro.slots");
    push_table(l, &(dude->stats.wizard[5]), "wizard6", "soloscuro.slots");
    push_table(l, &(dude->stats.wizard[6]), "wizard7", "soloscuro.slots");
    push_table(l, &(dude->stats.wizard[7]), "wizard8", "soloscuro.slots");
    push_table(l, &(dude->stats.wizard[8]), "wizard9", "soloscuro.slots");
    push_table(l, &(dude->stats.wizard[9]), "wizard10", "soloscuro.slots");
    push_table(l, &(dude->stats.priest[0]), "priest1", "soloscuro.slots");
    push_table(l, &(dude->stats.priest[1]), "priest2", "soloscuro.slots");
    push_table(l, &(dude->stats.priest[2]), "priest3", "soloscuro.slots");
    push_table(l, &(dude->stats.priest[3]), "priest4", "soloscuro.slots");
    push_table(l, &(dude->stats.priest[4]), "priest5", "soloscuro.slots");
    push_table(l, &(dude->stats.priest[5]), "priest6", "soloscuro.slots");
    push_table(l, &(dude->stats.priest[6]), "priest7", "soloscuro.slots");
    push_table(l, &(dude->stats.priest[7]), "priest8", "soloscuro.slots");
    push_table(l, &(dude->stats.priest[8]), "priest9", "soloscuro.slots");
    push_table(l, &(dude->stats.priest[9]), "priest10", "soloscuro.slots");

    push_table_end(l, "soloscuro.stats");

    push_table(l, &(dude->class[0]), "class0", "soloscuro.class");
    push_table(l, &(dude->class[1]), "class1", "soloscuro.class");
    push_table(l, &(dude->class[2]), "class2", "soloscuro.class");

    push_table(l, &dude->abilities, "ability", "soloscuro.ability");
    push_table(l, &dude->anim, "anim", "soloscuro.animate");

    if (dude->inv) {
        push_table_start(l, &(dude->stats), "inventory");
        push_table(l, &(dude->inv[0]), "arm", "soloscuro.item");
        push_table(l, &(dude->inv[1]), "ammo", "soloscuro.item");
        push_table(l, &(dude->inv[2]), "missile", "soloscuro.item");
        push_table(l, &(dude->inv[3]), "hand0", "soloscuro.item");
        push_table(l, &(dude->inv[4]), "finger0", "soloscuro.item");
        push_table(l, &(dude->inv[5]), "waist", "soloscuro.item");
        push_table(l, &(dude->inv[6]), "legs", "soloscuro.item");
        push_table(l, &(dude->inv[7]), "head", "soloscuro.item");
        push_table(l, &(dude->inv[8]), "neck", "soloscuro.item");
        push_table(l, &(dude->inv[9]), "chest", "soloscuro.item");
        push_table(l, &(dude->inv[10]), "hand1", "soloscuro.item");
        push_table(l, &(dude->inv[11]), "finger1", "soloscuro.item");
        push_table(l, &(dude->inv[12]), "cloak", "soloscuro.item");
        push_table(l, &(dude->inv[13]), "foot", "soloscuro.item");
        for (int i = 0; i < 12; i++) {
            sprintf(buf, "bp%d", i);
            push_table(l, &(dude->inv[14 + i]), buf, "soloscuro.item");
        }
        push_table_end(l, "soloscuro.inventory");
    }

    luaL_setmetatable(l, "soloscuro.entity"); // entity meta

    return 1;
}

static int load_player (lua_State *l) {
    int n = luaL_checkinteger(l, 1);
    return sol_lua_load_entity(l, sol_player_get(n));
}

static int create_player (lua_State *l) {
    int n = luaL_checkinteger(l, 1);
    sol_player_set(n, sol_entity_create_default_human());
    sol_player_load(n);
    return sol_lua_load_entity(l, sol_player_get(n));
}

static int create_entity (lua_State *l) {
    int has_inventory = luaL_checkinteger(l, 1);
    return sol_lua_load_entity(l, sol_entity_create(has_inventory));
}

static void push_region_lua(lua_State *l, sol_region_t *reg) {
    lua_newtable(l); // entity table
    lua_pushlightuserdata(l, reg);
    lua_setfield(l, -2, "ptr__");

    /*push_table_start(l, &(player_get_entity(n)->stats), "stats");

    push_table(l, &(player_get_entity(n)->stats.attacks[0]), "attack1", "soloscuro.attack");
    push_table(l, &(player_get_entity(n)->stats.attacks[1]), "attack2", "soloscuro.attack");
    push_table(l, &(player_get_entity(n)->stats.attacks[2]), "attack3", "soloscuro.attack");
    push_table(l, &(player_get_entity(n)->stats.saves), "saves", "soloscuro.saves");

    push_table_end(l, "soloscuro.stats");

    push_table(l, &(player_get_entity(n)->class[0]), "class1", "soloscuro.class");
    push_table(l, &(player_get_entity(n)->class[1]), "class2", "soloscuro.class");
    push_table(l, &(player_get_entity(n)->class[2]), "class3", "soloscuro.class");
    */

    luaL_setmetatable(l, "soloscuro.region"); // entity meta
}

static int create_region (lua_State *l) {
    push_region_lua(l, sol_region_create_empty());
    return 1;
}

static int set_region (lua_State *l) {
    luaL_checktype(l, 1, LUA_TTABLE);

    sol_region_t *region = sol_lua_get_userdata(l, 1);
    sol_region_manager_add_region(region);
    sol_region_manager_set_current(region);

    return 0;
}

static int get_region (lua_State *l) {
    push_region_lua(l, sol_region_manager_get_current());

    return 1;
}

static int open_gff (lua_State *l) {
    const char *filename = luaL_checkstring(l, 1);
    int gff_index = gff_open(filename);
    lua_newtable(l); // entity table

    if (gff_index <= 0) {
        return 1;
    }

    lua_pushlightuserdata(l, open_files + gff_index);
    lua_setfield(l, -2, "ptr__");

    luaL_setmetatable(l, "soloscuro.gff"); // entity meta

    return 1;
}

static int start_game (lua_State *l) {
    port_start();
    return 0;
}

// This is a testing hook!
static int test_fail (lua_State *l) {
    sol_game_loop_signal(WAIT_FINAL, 0);
    const char *str = luaL_checkstring(l, 1);
    sol_set_lua_test(1, strdup(str));
    return 0;
}

// This is a testing hook!
static int test_pass (lua_State *l) {
    sol_game_loop_signal(WAIT_FINAL, 0);
    sol_set_lua_test(0, NULL);
    return 0;
}

static int in_combat (lua_State *l) {
    lua_pushboolean(l,
        sol_combat_get_current(sol_arbiter_combat_region(sol_region_manager_get_current())) != NULL);
    return 1;
}

static int load_region(lua_State *l) {
    const int num = lua_gettop(l) >= 2 ? luaL_checkinteger(l, 2) : 0;
    push_region_lua(l, sol_region_manager_get_region(lua_tointeger(l, 1), num));
    return 1;
}

static const struct luaL_Reg sol_lib [] = {
    //{"new", entity_new},
    {"create_region", create_region},
    {"load_player", load_player},
    {"load_region", load_region},
    {"get_player", load_player},
    {"create_player", create_player},
    {"create_entity", create_entity},
    {"set_region", set_region},
    {"get_region", get_region},
    {"open_gff", open_gff},
    {"start_game", start_game},
    {"fail", test_fail},
    {"exit", test_pass},
    {"in_combat", in_combat},
    {NULL, NULL}
};

extern const struct luaL_Reg* lua_struct_get_funcs() {
    return sol_lib;
}

static int entity_get(lua_State *l) {
    const char *str = luaL_checkstring(l, 2);
    entity_t *dude = (entity_t*)sol_lua_get_userdata(l, -3);

    if (!dude) {
        lua_pushnil(l);
        return 1;
    }
    GET_STRING_TABLE(dude, name);
    GET_INTEGER_TABLE(dude, size);
    GET_INTEGER_TABLE(dude, race);
    GET_INTEGER_TABLE(dude, gender);
    GET_INTEGER_TABLE(dude, alignment);
    GET_INTEGER_TABLE(dude, allegiance);
    GET_INTEGER_TABLE(dude, object_flags);
    GET_INTEGER_TABLE(dude, region);
    GET_INTEGER_TABLE(dude, mapx);
    GET_INTEGER_TABLE(dude, mapy);
    GET_INTEGER_TABLE(dude, mapz);
    /*
    GET_INTEGER_TABLE(dude, stats.hp);
    GET_INTEGER_TABLE(dude, stats.high_hp);
    GET_INTEGER_TABLE(dude, stats.str);
    GET_INTEGER_TABLE(dude, stats.dex);
    GET_INTEGER_TABLE(dude, stats.con);
    GET_INTEGER_TABLE(dude, stats.intel);
    GET_INTEGER_TABLE(dude, stats.wis);
    GET_INTEGER_TABLE(dude, stats.cha);
    GET_INTEGER_TABLE(dude, stats.hp);
    GET_INTEGER_TABLE(dude, stats.high_hp);
    GET_INTEGER_TABLE(dude, stats.psp);
    GET_INTEGER_TABLE(dude, stats.high_psp);
    GET_INTEGER_TABLE(dude, stats.base_ac);
    GET_INTEGER_TABLE(dude, stats.base_move);
    GET_INTEGER_TABLE(dude, stats.base_thac0);
    */
    GET_INTEGER_TABLE(dude, sound_fx);
    GET_INTEGER_TABLE(dude, attack_sound);
    GET_INTEGER_TABLE(dude, combat_status);
    GET_INTEGER_TABLE(dude, ds_id);
    //if (!strcmp(str, "blah")) { lua_pushcfunction(l, blah); return 1; }
    return sol_lua_entity_function(dude, str, l);
}

static int entity_set(lua_State *l) {
    const char *str = luaL_checkstring(l, 2);
    entity_t *dude = sol_lua_get_userdata(l, -4);

    if (lua_isinteger(l, 3)) {
        const int num = luaL_checkinteger(l, 3);
        SET_INTEGER_TABLE(dude, size, num);
        SET_INTEGER_TABLE(dude, race, num);
        SET_INTEGER_TABLE(dude, gender, num);
        SET_INTEGER_TABLE(dude, alignment, num);
        SET_INTEGER_TABLE(dude, allegiance, num);
        SET_INTEGER_TABLE(dude, object_flags, num);
        SET_INTEGER_TABLE(dude, region, num);
        SET_INTEGER_TABLE(dude, mapx, num);
        SET_INTEGER_TABLE(dude, mapy, num);
        SET_INTEGER_TABLE(dude, mapz, num);
        SET_INTEGER_TABLE(dude, stats.hp, num);
        SET_INTEGER_TABLE(dude, stats.high_hp, num);
        SET_INTEGER_TABLE(dude, stats.str, num);
        SET_INTEGER_TABLE(dude, stats.dex, num);
        SET_INTEGER_TABLE(dude, stats.con, num);
        SET_INTEGER_TABLE(dude, stats.intel, num);
        SET_INTEGER_TABLE(dude, stats.wis, num);
        SET_INTEGER_TABLE(dude, stats.cha, num);
        SET_INTEGER_TABLE(dude, sound_fx, num);
        SET_INTEGER_TABLE(dude, attack_sound, num);
        SET_INTEGER_TABLE(dude, combat_status, num);
        SET_INTEGER_TABLE(dude, ds_id, num);
    } else if (lua_isstring(l, 3)) {
        const char *name = luaL_checkstring(l, 3);
        SET_STRING_TABLE(dude, name, name);
    }

    return 0;
}

static const luaL_Reg entity_methods[] = {
    {"__index",      entity_get},
    {"__newindex",   entity_set},
    {0,0}
};

static int stats_get(lua_State *l) {
    const char *str = luaL_checkstring(l, 2);
    stats_t *stats = sol_lua_get_userdata(l, -3);

    //printf("indexing '%s' of stats %p\n", str, stats);
    GET_INTEGER_TABLE(stats, str);
    GET_INTEGER_TABLE(stats, dex);
    GET_INTEGER_TABLE(stats, con);
    GET_INTEGER_TABLE(stats, intel);
    GET_INTEGER_TABLE(stats, wis);
    GET_INTEGER_TABLE(stats, cha);
    GET_INTEGER_TABLE(stats, hp);
    GET_INTEGER_TABLE(stats, high_hp);
    GET_INTEGER_TABLE(stats, psp);
    GET_INTEGER_TABLE(stats, high_psp);
    GET_INTEGER_TABLE(stats, base_ac);
    GET_INTEGER_TABLE(stats, combat.move);
    GET_INTEGER_TABLE(stats, combat.initiative);
    GET_INTEGER_TABLE(stats, combat.attack_num);
    GET_INTEGER_TABLE(stats, combat.has_cast);
    GET_INTEGER_TABLE(stats, combat.has_melee);
    GET_INTEGER_TABLE(stats, combat.has_ranged);
    GET_INTEGER_TABLE(stats, base_move);
    GET_INTEGER_TABLE(stats, base_thac0);
    GET_INTEGER_TABLE(stats, magic_resistance);
    GET_INTEGER_TABLE(stats, special_defense);
    //if (!strcmp(str, "blah")) { lua_pushcfunction(l, blah); return 1; }

    lua_pushinteger(l, 0);
    return 1;
}

static int stats_set(lua_State *l) {
    const char *str = luaL_checkstring(l, 2);
    stats_t *stats = sol_lua_get_userdata(l, -4);

    if (lua_isinteger(l, 3)) {
        const int num = luaL_checkinteger(l, 3);
        SET_INTEGER_TABLE(stats, str, num);
        SET_INTEGER_TABLE(stats, dex, num);
        SET_INTEGER_TABLE(stats, con, num);
        SET_INTEGER_TABLE(stats, intel, num);
        SET_INTEGER_TABLE(stats, wis, num);
        SET_INTEGER_TABLE(stats, cha, num);
        SET_INTEGER_TABLE(stats, hp, num);
        SET_INTEGER_TABLE(stats, high_hp, num);
        SET_INTEGER_TABLE(stats, psp, num);
        SET_INTEGER_TABLE(stats, high_psp, num);
        SET_INTEGER_TABLE(stats, base_ac, num);
        SET_INTEGER_TABLE(stats, combat.move, num);
        SET_INTEGER_TABLE(stats, combat.initiative, num);
        SET_INTEGER_TABLE(stats, combat.attack_num, num);
        SET_INTEGER_TABLE(stats, combat.has_cast, num);
        SET_INTEGER_TABLE(stats, combat.has_melee, num);
        SET_INTEGER_TABLE(stats, combat.has_ranged, num);
        SET_INTEGER_TABLE(stats, base_move, num);
        SET_INTEGER_TABLE(stats, base_thac0, num);
        SET_INTEGER_TABLE(stats, magic_resistance, num);
        SET_INTEGER_TABLE(stats, special_defense, num);
    }

    return 0;
}

static const luaL_Reg stats_methods[] = {
    {"__index",      stats_get},
    {"__newindex",   stats_set},
    {0,0}
};

static int animate_get(lua_State *l) {
    const char *str = luaL_checkstring(l, 2);
    animate_sprite_t *as = sol_lua_get_userdata(l, -3);

    //printf("indexing '%s' of attack %p\n", str, attack);
    GET_INTEGER_TABLE(as, flags);
    GET_INTEGER_TABLE(as, delay);
    GET_INTEGER_TABLE(as, pos);
    GET_INTEGER_TABLE(as, x);
    GET_INTEGER_TABLE(as, y);
    GET_INTEGER_TABLE(as, w);
    GET_INTEGER_TABLE(as, h);
    GET_INTEGER_TABLE(as, destx);
    GET_INTEGER_TABLE(as, desty);
    GET_INTEGER_TABLE(as, xoffset);
    GET_INTEGER_TABLE(as, bmp_id);
    GET_INTEGER_TABLE(as, load_frame);
    GET_INTEGER_TABLE(as, yoffset);
    //if (!strcmp(str, "blah")) { lua_pushcfunction(l, blah); return 1; }

    lua_pushinteger(l, 0);
    return 1;
}

static int animate_set(lua_State *l) {
    const char *str = luaL_checkstring(l, 2);
    animate_sprite_t *as = sol_lua_get_userdata(l, -4);

    if (lua_isinteger(l, 3)) {
        const int num = luaL_checkinteger(l, 3);
        SET_INTEGER_TABLE(as, flags, num);
        SET_INTEGER_TABLE(as, delay, num);
        SET_INTEGER_TABLE(as, pos, num);
        SET_INTEGER_TABLE(as, x, num);
        SET_INTEGER_TABLE(as, y, num);
        SET_INTEGER_TABLE(as, w, num);
        SET_INTEGER_TABLE(as, h, num);
        SET_INTEGER_TABLE(as, destx, num);
        SET_INTEGER_TABLE(as, desty, num);
        SET_INTEGER_TABLE(as, xoffset, num);
        SET_INTEGER_TABLE(as, bmp_id, num);
        SET_INTEGER_TABLE(as, load_frame, num);
        SET_INTEGER_TABLE(as, yoffset, num);
    }

    return 0;
}
static const luaL_Reg animate_methods[] = {
    {"__index",    animate_get},
    {"__newindex", animate_set},
    {0,0}
};

static int attack_get(lua_State *l) {
    const char *str = luaL_checkstring(l, 2);
    innate_attack_t *attack = sol_lua_get_userdata(l, -3);

    //printf("indexing '%s' of attack %p\n", str, attack);
    GET_INTEGER_TABLE(attack, number);
    GET_INTEGER_TABLE(attack, num_dice);
    GET_INTEGER_TABLE(attack, sides);
    GET_INTEGER_TABLE(attack, bonus);
    GET_INTEGER_TABLE(attack, special);
    //if (!strcmp(str, "blah")) { lua_pushcfunction(l, blah); return 1; }

    lua_pushinteger(l, 0);
    return 1;
}

static int attack_set(lua_State *l) {
    const char *str = luaL_checkstring(l, 2);
    innate_attack_t *attack = sol_lua_get_userdata(l, -4);

    if (lua_isinteger(l, 3)) {
        const int num = luaL_checkinteger(l, 3);
        SET_INTEGER_TABLE(attack, number, num);
        SET_INTEGER_TABLE(attack, num_dice, num);
        SET_INTEGER_TABLE(attack, sides, num);
        SET_INTEGER_TABLE(attack, bonus, num);
        SET_INTEGER_TABLE(attack, special, num);
    }

    return 0;
}
static const luaL_Reg attack_methods[] = {
    {"__index",      attack_get},
    {"__newindex",   attack_set},
    {0,0}
};

static int gff_get(lua_State *l) {
    const char *str = luaL_checkstring(l, 2);
    gff_file_t *file = sol_lua_get_userdata(l, -3);

    printf("indexing '%s' of file %p\n", str, file);
    //GET_INTEGER_TABLE(file, number);
    //GET_INTEGER_TABLE(file, num_dice);
    //GET_INTEGER_TABLE(file, sides);
    //GET_INTEGER_TABLE(file, bonus);
    //GET_INTEGER_TABLE(file, special);
    //if (!strcmp(str, "blah")) { lua_pushcfunction(l, blah); return 1; }

    lua_pushinteger(l, 0);
    return 1;
}

static int gff_set(lua_State *l) {
    const char *str = luaL_checkstring(l, 2);
    gff_file_t *file = sol_lua_get_userdata(l, -4);

    if (lua_isinteger(l, 3)) {
        const int num = luaL_checkinteger(l, 3);
        printf("set %s=%d for %p\n", str, num, file);
        //SET_INTEGER_TABLE(file, number, num);
        //SET_INTEGER_TABLE(file, num_dice, num);
        //SET_INTEGER_TABLE(file, sides, num);
        //SET_INTEGER_TABLE(file, bonus, num);
        //SET_INTEGER_TABLE(file, special, num);
    }

    return 0;
}
static const luaL_Reg gff_methods[] = {
    {"__index",      gff_get},
    {"__newindex",   gff_set},
    {0,0}
};

static int saves_get(lua_State *l) {
    const char *str = luaL_checkstring(l, 2);
    saving_throws_t *saves = sol_lua_get_userdata(l, -3);

    //printf("indexing '%s' of saves %p\n", str, saves);
    GET_INTEGER_TABLE(saves, paralysis);
    GET_INTEGER_TABLE(saves, wand);
    GET_INTEGER_TABLE(saves, petrify);
    GET_INTEGER_TABLE(saves, breath);
    GET_INTEGER_TABLE(saves, spell);
    //if (!strcmp(str, "blah")) { lua_pushcfunction(l, blah); return 1; }

    lua_pushinteger(l, 0);
    return 1;
}

static int saves_set(lua_State *l) {
    const char *str = luaL_checkstring(l, 2);
    saving_throws_t *saves = sol_lua_get_userdata(l, -4);

    if (lua_isinteger(l, 3)) {
        const int num = luaL_checkinteger(l, 3);
        SET_INTEGER_TABLE(saves, paralysis, num);
        SET_INTEGER_TABLE(saves, wand, num);
        SET_INTEGER_TABLE(saves, petrify, num);
        SET_INTEGER_TABLE(saves, breath, num);
        SET_INTEGER_TABLE(saves, spell, num);
    }

    return 0;
}
static const luaL_Reg saves_methods[] = {
    {"__index",      saves_get},
    {"__newindex",   saves_set},
    {0,0}
};

static int item_get(lua_State *l) {
    const char *str = luaL_checkstring(l, 2);
    item_t *item = sol_lua_get_userdata(l, -3);

    //printf("indexing '%s' of saves %p\n", str, saves);
    GET_INTEGER_TABLE(item, ds_id);
    //if (!strcmp(str, "blah")) { lua_pushcfunction(l, blah); return 1; }

    lua_pushinteger(l, 0);
    return 1;
}

static int item_set(lua_State *l) {
    const char *str = luaL_checkstring(l, 2);
    item_t *item = sol_lua_get_userdata(l, -4);

    if (lua_isinteger(l, 3)) {
        const int num = luaL_checkinteger(l, 3);
        SET_INTEGER_TABLE(item, ds_id, num);
    }

    return 0;
}
static const luaL_Reg item_methods[] = {
    {"__index",      item_get},
    {"__newindex",   item_set},
    {0,0}
};

static int slots_get(lua_State *l) {
    const char *str = luaL_checkstring(l, 2);
    sol_slots_t *slot = sol_lua_get_userdata(l, -3);

    //printf("indexing '%s' of saves %p\n", str, attack);
    GET_INTEGER_TABLE(slot, amt);
    GET_INTEGER_TABLE(slot, max);
    //if (!strcmp(str, "blah")) { lua_pushcfunction(l, blah); return 1; }

    lua_pushinteger(l, 0);
    return 1;
}

static int slots_set(lua_State *l) {
    const char *str = luaL_checkstring(l, 2);
    sol_slots_t *slot = sol_lua_get_userdata(l, -4);

    if (lua_isinteger(l, 3)) {
        const int num = luaL_checkinteger(l, 3);
        SET_INTEGER_TABLE(slot, amt, num);
        SET_INTEGER_TABLE(slot, max, num);
    }

    return 0;
}
static const luaL_Reg slots_methods[] = {
    {"__index",      slots_get},
    {"__newindex",   slots_set},
    {0,0}
};

static int ability_get(lua_State *l) {
    const char *str = luaL_checkstring(l, 2);
    ability_set_t *ability = sol_lua_get_userdata(l, -3);

    //printf("indexing '%s' of saves %p\n", str, attack);
    GET_INTEGER_TABLE(ability, hunt);
    GET_INTEGER_TABLE(ability, must_go);
    if (!strcmp(str, "xpos")) { lua_pushinteger(l, ability->args.pos.x); return 1; }
    if (!strcmp(str, "ypos")) { lua_pushinteger(l, ability->args.pos.y); return 1; }

    lua_pushinteger(l, 0);
    return 1;
}

static int ability_set(lua_State *l) {
    const char *str = luaL_checkstring(l, 2);
    ability_set_t *ability = sol_lua_get_userdata(l, -4);

    if (lua_isinteger(l, 3)) {
        const int num = luaL_checkinteger(l, 3);
        SET_INTEGER_TABLE(ability, hunt, num);
        SET_INTEGER_TABLE(ability, must_go, num);
        if (!strcmp(str, "xpos")) { ability->args.pos.x = num; return 0; }
        if (!strcmp(str, "ypos")) { ability->args.pos.y = num; return 0; }
    }

    return 0;
}
static const luaL_Reg ability_methods[] = {
    {"__index",      ability_get},
    {"__newindex",   ability_set},
    {0,0}
};

static int class_get(lua_State *l) {
    const char *str = luaL_checkstring(l, 2);
    class_t *class = sol_lua_get_userdata(l, -3);

    //printf("indexing '%s' of saves %p\n", str, attack);
    GET_INTEGER_TABLE(class, current_xp);
    GET_INTEGER_TABLE(class, high_xp);
    GET_INTEGER_TABLE(class, class);
    GET_INTEGER_TABLE(class, level);
    GET_INTEGER_TABLE(class, high_level);
    //if (!strcmp(str, "blah")) { lua_pushcfunction(l, blah); return 1; }

    lua_pushinteger(l, 0);
    return 1;
}

static int class_set(lua_State *l) {
    const char *str = luaL_checkstring(l, 2);
    class_t *class = sol_lua_get_userdata(l, -4);

    if (lua_isinteger(l, 3)) {
        const int num = luaL_checkinteger(l, 3);
        SET_INTEGER_TABLE(class, current_xp, num);
        SET_INTEGER_TABLE(class, high_xp, num);
        SET_INTEGER_TABLE(class, class, num);
        SET_INTEGER_TABLE(class, level, num);
        SET_INTEGER_TABLE(class, high_level, num);
    }

    return 0;
}
static const luaL_Reg class_methods[] = {
    {"__index",      class_get},
    {"__newindex",   class_set},
    {0,0}
};

static int region_get(lua_State *l) {
    const char *str = luaL_checkstring(l, 2);
    sol_region_t *region = sol_lua_get_userdata(l, -3);

    GET_INTEGER_TABLE(region, map_id);
    GET_INTEGER_TABLE(region, palette_id);
    GET_INTEGER_TABLE(region, gff_file);
    //GET_INTEGER_TABLE(region, assume_loaded);
    if (strcmp(str, "flags") == 0) {
        //int index = luaL_checkinteger(l, 3);
        //printf("index = %d\n", index);
        sol_lua_dumpstack (l);
        return 0;
    }
    return sol_lua_region_function(region, str, l);
}

static int region_set(lua_State *l) {
    const char *str = luaL_checkstring(l, 2);
    sol_region_t *region = sol_lua_get_userdata(l, -4);

    if (lua_isinteger(l, 3)) {
        const int num = luaL_checkinteger(l, 3);
        SET_INTEGER_TABLE(region, map_id, num);
        SET_INTEGER_TABLE(region, palette_id, num);
        //SET_INTEGER_TABLE(region, assume_loaded, num);
    } else if (lua_istable(l, 3)) {
        gff_file_t *file = sol_lua_get_userdata(l, 3);
        if (!strcmp("gff_file", str)) {
            region->gff_file = file - open_files;
        }
    }

    return 0;
}
static const luaL_Reg region_methods[] = {
    {"__index",      region_get},
    {"__newindex",   region_set},
    {0,0}
};

static void setup_metatable(lua_State *l, const char *name, const luaL_Reg *methods, const char *global) {
    luaL_newmetatable(l, name);
    luaL_setfuncs(l, methods, 0);
    lua_setglobal(l, global);
}

extern void lua_struct_register(lua_State *l) {
    setup_metatable(l, "soloscuro.entity", entity_methods, "entity__");
    setup_metatable(l, "soloscuro.stats", stats_methods, "stats__");
    setup_metatable(l, "soloscuro.slots", slots_methods, "slots__");
    setup_metatable(l, "soloscuro.attack", attack_methods, "attack__");
    setup_metatable(l, "soloscuro.saves", saves_methods, "saves__");
    setup_metatable(l, "soloscuro.class", class_methods, "class__");
    setup_metatable(l, "soloscuro.animate", animate_methods, "animate__");
    setup_metatable(l, "soloscuro.item", item_methods, "item__");
    setup_metatable(l, "soloscuro.region", region_methods, "region__");
    setup_metatable(l, "soloscuro.ability", ability_methods, "ability__");
    setup_metatable(l, "soloscuro.gff", gff_methods, "gff__");
}
