#include "lua-entity.h"
#include "powers.h"
#include "wizard.h"
#include "gpl.h"
#include "combat.h"
#include "arbiter.h"
#include "entity-animation.h"
#include "region-manager.h"
#include "rules.h"
#include <string.h>

static int entity_cast(lua_State *l) {
    //sol_lua_dumpstack (l);
    dude_t *dude = (dude_t*) lua_touserdata(l, lua_upvalueindex(1));
    dude_t *target = (dude_t*) sol_lua_get_userdata(l, -1 - lua_gettop(l));
    power_t *pw = pw = wizard_get_spell(luaL_checkinteger(l,2));

    if (!pw) {
        error("Did not find power!\n");
        return 0;
    }

    sol_combat_activate_power(pw, dude, target, target->mapx, target->mapy);

    return 0;
}

static int push_entity_function(lua_State *l, entity_t *entity, int (*func)(lua_State *l)) {
    lua_pushlightuserdata(l, entity);
    lua_pushcclosure(l, func, 1);
    return 1;
}

static int in_combat(lua_State *l) {
    dude_t *dude = (dude_t*) lua_touserdata(l, lua_upvalueindex(1));
    sol_region_t *reg = sol_region_manager_get_region_with_entity(dude);
    lua_pushboolean(l, sol_combat_get_current(sol_arbiter_combat_region(reg)) != NULL);
    return 1;
}

static int move_entity(lua_State *l, const int xdiff, const int ydiff) {
    dude_t *dude = (dude_t*) lua_touserdata(l, lua_upvalueindex(1));
    lua_pushboolean(l, entity_attempt_move(dude, xdiff, ydiff, 1));
    return 1;
}

static int move_left(lua_State *l) { return move_entity(l, -1, 0); }
static int move_right(lua_State *l) { return move_entity(l, 1, 0); }
static int move_up(lua_State *l) { return move_entity(l, 0, -1); }
static int move_down(lua_State *l) { return move_entity(l, 0, 1); }
static int move_up_left(lua_State *l) { return move_entity(l, -1, -1); }
static int move_up_right(lua_State *l) { return move_entity(l, 1, -1); }
static int move_down_left(lua_State *l) { return move_entity(l, -1, 1); }
static int move_down_right(lua_State *l) { return move_entity(l, 1, 1); }

static int set_class(lua_State *l) {
    dude_t   *dude = (dude_t*) lua_touserdata(l, lua_upvalueindex(1));
    uint16_t  which = luaL_checkinteger(l, 1);
    uint16_t  class = luaL_checkinteger(l, 2);
    dude->class[which].class = class;
    dude->class[which].level = 0;
}

static int award_exp(lua_State *l) {
    dude_t   *dude = (dude_t*) lua_touserdata(l, lua_upvalueindex(1));
    uint32_t  exp = luaL_checkinteger(l, 1);

    dnd2e_award_exp(dude, exp);
}

static int give_ds1_item(lua_State *l) {
    dude_t   *dude = (dude_t*) lua_touserdata(l, lua_upvalueindex(1));
    int32_t   slot = luaL_checkinteger(l, 1);
    int32_t   item_index = luaL_checkinteger(l, 2);
    int32_t   item_id = luaL_checkinteger(l, 3);

    sol_give_ds1_item(dude, slot, item_index, item_id);
    //printf("need to give %d to slot %d of %p\n", slot, item_id, dude);
    return 0;
}

static int get_closest_enemy(lua_State *l) {
    dude_t   *dude = (dude_t*) lua_touserdata(l, lua_upvalueindex(1));
    combat_region_t *cr = sol_arbiter_combat_region(sol_region_manager_get_current());
    entity_t *enemy = sol_combat_get_closest_enemy(cr, dude->mapx, dude->mapy);

    return sol_lua_load_entity (l, enemy);
}

extern int sol_lua_entity_function(entity_t *entity, const char *func, lua_State *l) {
    if (!strcmp(func, "cast")) {
        return push_entity_function(l, entity, entity_cast);
    } else if (!strcmp(func, "in_combat")) {
        return push_entity_function(l, entity, in_combat);
    } else if (!strcmp(func, "move_left")) {
        return push_entity_function(l, entity, move_left);
    } else if (!strcmp(func, "move_right")) {
        return push_entity_function(l, entity, move_right);
    } else if (!strcmp(func, "move_up")) {
        return push_entity_function(l, entity, move_up);
    } else if (!strcmp(func, "move_down")) {
        return push_entity_function(l, entity, move_down);
    } else if (!strcmp(func, "move_up_left")) {
        return push_entity_function(l, entity, move_up_left);
    } else if (!strcmp(func, "move_up_right")) {
        return push_entity_function(l, entity, move_up_right);
    } else if (!strcmp(func, "move_down_left")) {
        return push_entity_function(l, entity, move_down_left);
    } else if (!strcmp(func, "move_down_right")) {
        return push_entity_function(l, entity, move_down_right);
    } else if (!strcmp(func, "set_class")) {
        return push_entity_function(l, entity, set_class);
    } else if (!strcmp(func, "award_exp")) {
        return push_entity_function(l, entity, award_exp);
    } else if (!strcmp(func, "give_ds1_item")) {
        return push_entity_function(l, entity, give_ds1_item);
    } else if (!strcmp(func, "get_closest_enemy")) {
        return push_entity_function(l, entity, get_closest_enemy);
    }
    lua_pushinteger(l, 0);
    return 1;
}
