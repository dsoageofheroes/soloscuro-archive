#include "lua-entity.h"
#include "powers.h"
#include "wizard.h"
#include "gpl.h"
#include "combat.h"
#include "arbiter.h"
#include "entity-animation.h"
#include "region-manager.h"
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
    lua_pushboolean(l, sol_arbiter_is_in_combat(reg));
    return 1;
}

extern int sol_lua_entity_function(entity_t *entity, const char *func, lua_State *l) {
    if (!strcmp(func, "cast")) {
        return push_entity_function(l, entity, entity_cast);
    }
    if (!strcmp(func, "in_combat")) {
        return push_entity_function(l, entity, in_combat);
    }
    lua_pushinteger(l, 0);
    return 1;
}
