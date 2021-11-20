#include "dsl.h"
#include "lua-inc.h"
#include "dsl-manager.h"
#include "region.h"
#include "gpl-state.h"
#include "gameloop.h"
#include "port.h"
#include "ssi-scmd.h"
#include "replay.h"
#include "region-manager.h"
#include "trigger.h"
#include "narrate.h"
#include <string.h>
#include <stdlib.h>

#define BUF_SIZE           (1<<12)
#define MAX_GFLAGS         (800)
#define MAX_LFLAGS         (64)
#define MAX_GNUMS          (400)
#define MAX_GBIGNUMS       (40)
#define MAX_LBIGNUMS       (40)
#define MAX_LNUMS          (32)
#define MAX_GSTRS          (32)
#define STRING_SIZE        (1024)
#define MAX_GNAMES         (13)

static int8_t gpl_global_flags[MAX_GFLAGS];
static int8_t gpl_local_flags[MAX_LFLAGS];
static int16_t gpl_global_nums[MAX_GNUMS];
static int16_t gpl_local_nums[MAX_LNUMS];
static int32_t gpl_global_bnums[MAX_GBIGNUMS];
static int32_t gpl_local_bnums[MAX_LBIGNUMS];
static char gpl_global_strs[MAX_GSTRS][STRING_SIZE];
static int16_t gpl_gnames[MAX_GNAMES];

void gpl_state_init() {
    memset(gpl_global_flags, 0x0, sizeof(int8_t) * MAX_GFLAGS);
    memset(gpl_global_nums, 0x0, sizeof(int16_t) * MAX_GNUMS);
    memset(gpl_global_bnums, 0x0, sizeof(int32_t) * MAX_GBIGNUMS);
    memset(gpl_global_strs, 0x0, sizeof(char) * MAX_GSTRS * STRING_SIZE);
    memset(gpl_gnames, 0x0, sizeof(int16_t) * MAX_GNAMES);
    gpl_local_clear();
}

void gpl_state_cleanup() {
}

static int set_while_callback(lua_State *l) {
    const char *val = luaL_checkstring(l, 2);
    printf("------------------>%s\n", val);
    //lua_Integer state = luaL_checkinteger(l, 1);
    return 0;
}

// Public to C library
void gpl_set_gname(const int index, const int32_t obj) {
    if (index < 0 || index > MAX_GNAMES) { return; }
    //printf("GNAME----------------------------------------------------->[%d] = %d\n", index, obj);
    gpl_gnames[index] = obj;
}

// Public to LUA
static int set_gf(lua_State *l) {
    lua_Integer id = luaL_checkinteger(l, 1);
    lua_Integer val = luaL_checkinteger(l, 2);
    if (id < 0 || id >= MAX_GFLAGS) {
        printf("ERROR: " PRI_LI " is out of range for global flags!\n", id);
        exit(1);
    }
    gpl_global_flags[id] = val;
    debug("gpl_globals_flags[" PRI_LI "] = " PRI_LI "\n", id, val);
    return 0;
}

static int get_gf(lua_State *l) {
    lua_Integer id = luaL_checkinteger(l, 1);
    if (id < 0 || id >= MAX_GFLAGS) {
        printf("ERROR: " PRI_LI " is out of range for global flags!\n", id);
        exit(1);
    }
    lua_pushinteger(l, gpl_global_flags[id]);
    return 1;
}

static int set_lf(lua_State *l) {
    lua_Integer id = luaL_checkinteger(l, 1);
    lua_Integer val = luaL_checkinteger(l, 2);
    if (id < 0 || id >= MAX_LFLAGS) {
        printf("ERROR: " PRI_LI " is out of range for local flags!\n", id);
        exit(1);
    }
    gpl_local_flags[id] = val;
    debug("gpl_local_flags[" PRI_LI "] = " PRI_LI "\n", id, val);
    return 0;
}

static int get_lf(lua_State *l) {
    lua_Integer id = luaL_checkinteger(l, 1);
    if (id < 0 || id >= MAX_LFLAGS) {
        printf("ERROR: " PRI_LI " is out of range for local flags!\n", id);
        exit(1);
    }
    //printf("local_flag[" PRI_LI "] = %d (get)\n", id, gpl_local_flags[id]);
    lua_pushinteger(l, gpl_local_flags[id]);
    return 1;
}

static int set_gn(lua_State *l) {
    lua_Integer id = luaL_checkinteger(l, 1);
    lua_Integer val = luaL_checkinteger(l, 2);
    if (id < 0 || id >= MAX_GNUMS) {
        printf("ERROR: " PRI_LI " is out of range for global nums!\n", id);
        exit(1);
    }
    gpl_global_nums[id] = val;
    debug("gpl_globals_nums[" PRI_LI "] = " PRI_LI "\n", id, val);
    return 0;
}

static int get_gn(lua_State *l) {
    lua_Integer id = luaL_checkinteger(l, 1);
    if (id < 0 || id >= MAX_GNUMS) {
        printf("ERROR: " PRI_LI " is out of range for global nums!\n", id);
        exit(1);
    }
    lua_pushinteger(l, gpl_global_nums[id]);
    return 1;
}

static int set_ln(lua_State *l) {
    lua_Integer id = luaL_checkinteger(l, 1);
    lua_Integer val = luaL_checkinteger(l, 2);
    if (id < 0 || id >= MAX_LNUMS) {
        printf("ERROR: " PRI_LI " is out of range for local nums!\n", id);
        exit(1);
    }
    gpl_local_nums[id] = val;
    return 0;
}

static int get_ln(lua_State *l) {
    lua_Integer id = luaL_checkinteger(l, 1);
    if (id < 0 || id >= MAX_LNUMS) {
        printf("ERROR: " PRI_LI " is out of range for local nums!\n", id);
        exit(1);
    }
    lua_pushinteger(l, gpl_local_nums[id]);
    return 1;
}

static int set_gbn(lua_State *l) {
    lua_Integer id = luaL_checkinteger(l, 1);
    lua_Integer val = luaL_checkinteger(l, 2);
    if (id < 0 || id >= MAX_GBIGNUMS) {
        error("ERROR: " PRI_LI " is out of range for global big nums!\n", id);
        exit(1);
    }
    gpl_global_bnums[id] = val;
    return 0;
}

static int get_gbn(lua_State *l) {
    lua_Integer id = luaL_checkinteger(l, 1);
    if (id < 0 || id >= MAX_GBIGNUMS) {
        error("ERROR: " PRI_LI " is out of range for global big nums!\n", id);
        exit(1);
    }
    lua_pushinteger(l, gpl_global_bnums[id]);
    return 1;
}

static int set_lbn(lua_State *l) {
    lua_Integer id = luaL_checkinteger(l, 1);
    lua_Integer val = luaL_checkinteger(l, 2);
    if (id < 0 || id >= MAX_LBIGNUMS) {
        printf("ERROR: " PRI_LI " is out of range for local big nums!\n", id);
        exit(1);
    }
    gpl_local_bnums[id] = val;
    return 0;
}

static int get_lbn(lua_State *l) {
    lua_Integer id = luaL_checkinteger(l, 1);
    if (id < 0 || id >= MAX_LBIGNUMS) {
        printf("ERROR: " PRI_LI " is out of range for local big nums!\n", id);
        exit(1);
    }
    lua_pushinteger(l, gpl_local_bnums[id]);
    return 1;
}

static int set_gstr(lua_State *l) {
    lua_Integer id = luaL_checkinteger(l, 1);
    const char *val = luaL_checkstring(l, 2);
    if (id < 0 || id >= MAX_GSTRS) {
        printf("ERROR: " PRI_LI " is out of range for local big nums!\n", id);
        exit(1);
    }
    strncpy(gpl_global_strs[id], val, STRING_SIZE);
    return 0;
}

static int get_gstr(lua_State *l) {
    lua_Integer id = luaL_checkinteger(l, 1);
    if (id < 0 || id >= MAX_GSTRS) {
        printf("ERROR: " PRI_LI " is out of range for local big nums!\n", id);
        exit(1);
    }
    lua_pushstring(l, gpl_global_strs[id]);
    return 1;
}

static int get_gname(lua_State *l) {
    lua_Integer id = luaL_checkinteger(l, 1);
    if (id < 0 || id >= MAX_GNAMES) {
        printf("ERROR: " PRI_LI " is out of range for local big nums!\n", id);
        exit(1);
    }
    //printf("GNAME----------------------------------------------------->[" PRI_LI "] = %d\n", id, gpl_gnames[id]);
    lua_pushnumber(l, gpl_gnames[id]);
    return 1;
}

static int set_gname(lua_State *l) {
    lua_Integer id = luaL_checkinteger(l, 1);
    lua_Integer val = luaL_checkinteger(l, 2);
    if (id < 0 || id >= MAX_GNAMES) {
        error("" PRI_LI " is out of range for local big nums!\n", id);
        exit(1);
    }
    if (id != 1 && id != 2 && id != 3) {
        error("illegal set_gname? id = " PRI_LI "\n", id);
    } else {
        gpl_gnames[id] = val;
    }
    //printf("GNAME----------------------------------------------------->[" PRI_LI "] = %d\n", id, gpl_gnames[id]);
    return 0;
}

static int get_type(lua_State *l) {
    //lua_Integer id = luaL_checkinteger(l, 1);
    error("error: dsl.get_type: not implemented returning -1!\n");
    //lua_pushnumber(l, gpl_gnames[id]);
    lua_pushnumber(l, -1);
    return 1;
}

static int get_id(lua_State *l) {
    lua_Integer obj = luaL_checkinteger(l, 1);
    //printf("!!!!!!!!!!dsl.get_name: not implemented returning -1!\n");
    //printf("-------->returning " PRI_LI "\n", obj);
    //error("error: dsl.get_id not implement, just returning obj (" PRI_LI " ).\n", obj);
    lua_pushnumber(l, obj);
    //lua_pushnumber(l, -1);
    return 1;
}

static int get_party(lua_State *l) {
    lua_Integer member = luaL_checkinteger(l, 1);
    //static int idx = -1;
    error("error: dsl.get_party: not implemented returning 9999 for member: " PRI_LI "!\n", member);
    //lua_pushnumber(l, gpl_gnames[id]);
    lua_pushnumber(l, 9999);
    /*
    lua_pushnumber(l, idx);
    if (idx == 9999) { idx = -1; return 1;}
    idx--;
    if (idx < -4) {idx = 9999;};
    */
    return 1;
}

static int is_true(lua_State *l) {
    if (lua_isboolean(l, 1)) {
        //printf("------------>boolean = %d\n", lua_toboolean(l, 1));
        lua_pushboolean(l, lua_toboolean(l, 1) == 1);
    } else if (lua_isinteger(l, 1)) {
        lua_pushboolean(l, lua_tointeger(l, 1) != 0);
    } else {
        error("ERROR: did not received a boolean or int for accum testing!\n");
        lua_pushboolean(l, 0);
    }

    return 1;
}

static int gpl_getX(lua_State *l) {
    lua_Integer id = luaL_checkinteger(l, 1);
    sol_region_t *reg = region_manager_get_current();
    dude_t *dude = NULL;

    entity_list_for_each(reg->entities, dude) {
        if (dude->ds_id == id) {
            lua_pushnumber(l, dude->mapx - (dude->sprite.xoffset + 15) / 16); // the +15 forces a round up.
            return 1;
        }
    }

    lua_pushnumber(l, -1);
    return 1;
}

static int gpl_getY(lua_State *l) {
    lua_Integer id = luaL_checkinteger(l, 1);
    sol_region_t *reg = region_manager_get_current();
    dude_t *dude = NULL;

    entity_list_for_each(reg->entities, dude) {
        if (dude->ds_id == id) {
            lua_pushnumber(l, dude->mapy - (dude->sprite.yoffset + 15) / 16); // the +15 forces a round up.
            return 1;
        }
    }

    lua_pushnumber(l, -1);
    return 1;
}

static int read_complex(lua_State *l) {
    lua_Integer obj = luaL_checkinteger(l, 1);
    lua_Integer header = luaL_checkinteger(l, 1);
    lua_Integer depth = luaL_checkinteger(l, 1);

    warn("Need to implement read_complex(" PRI_LI ", " PRI_LI ", " PRI_LI ")\n", obj, header, depth);

    lua_pushinteger(l, 1);
    return 1;
}

static int gpl_rand(lua_State *l) {
    lua_pushinteger(l, rand());
    return 1;
}

static int range(lua_State *l) {
    const char *x = luaL_checkstring(l, 1);
    const char *y = luaL_checkstring(l, 2);

    debug("Must find range from '%s' to '%s'\n", x, y);
    lua_pushinteger(l, 0);

    return 1;
}

static int attack_trigger(lua_State *l) {
    lua_Integer obj = luaL_checkinteger(l, 1);
    lua_Integer file = luaL_checkinteger(l, 2);
    lua_Integer addr = luaL_checkinteger(l, 3);

    add_attack_trigger(obj, file, addr);
    return 0;
}

static int attack_trigger_global(lua_State *l) {
    lua_Integer obj = luaL_checkinteger(l, 1);
    lua_Integer file = luaL_checkinteger(l, 2);
    lua_Integer addr = luaL_checkinteger(l, 3);

    add_attack_trigger_global(obj, file, addr);
    return 0;
}

static int use_trigger(lua_State *l) {
    lua_Integer obj = luaL_checkinteger(l, 1);
    lua_Integer file = luaL_checkinteger(l, 2);
    lua_Integer addr = luaL_checkinteger(l, 3);

    add_use_trigger(obj, file, addr);
    return 0;
}

static int use_trigger_global(lua_State *l) {
    lua_Integer obj = luaL_checkinteger(l, 1);
    lua_Integer file = luaL_checkinteger(l, 2);
    lua_Integer addr = luaL_checkinteger(l, 3);

    add_use_trigger_global(obj, file, addr);
    return 0;
}

static int look_trigger(lua_State *l) {
    lua_Integer obj = luaL_checkinteger(l, 1);
    lua_Integer file = luaL_checkinteger(l, 2);
    lua_Integer addr = luaL_checkinteger(l, 3);

    add_look_trigger(obj, file, addr);
    return 0;
}

static int look_trigger_global(lua_State *l) {
    lua_Integer obj = luaL_checkinteger(l, 1);
    lua_Integer file = luaL_checkinteger(l, 2);
    lua_Integer addr = luaL_checkinteger(l, 3);

    add_look_trigger_global(obj, file, addr);
    return 0;
}

static int noorders_trigger(lua_State *l) {
    lua_Integer obj = luaL_checkinteger(l, 1);
    lua_Integer file = luaL_checkinteger(l, 2);
    lua_Integer addr = luaL_checkinteger(l, 3);

    add_noorders_trigger(obj, file, addr);
    return 0;
}

static int talk_to_trigger(lua_State *l) {
    lua_Integer obj = luaL_checkinteger(l, 1);
    lua_Integer file = luaL_checkinteger(l, 2);
    lua_Integer addr = luaL_checkinteger(l, 3);

    add_talkto_trigger(obj, file, addr);
    return 0;
}

static int use_with_trigger(lua_State *l) {
    lua_Integer obj1 = luaL_checkinteger(l, 1);
    lua_Integer obj2 = luaL_checkinteger(l, 2);
    lua_Integer file = luaL_checkinteger(l, 3);
    lua_Integer addr = luaL_checkinteger(l, 4);

    add_usewith_trigger(obj1, obj2, file, addr);
    return 0;
}

static int tile_trigger(lua_State *l) {
    lua_Integer x = luaL_checkinteger(l, 1);
    lua_Integer y = luaL_checkinteger(l, 2);
    lua_Integer addr = luaL_checkinteger(l, 3);
    lua_Integer file = luaL_checkinteger(l, 4);
    lua_Integer trip = luaL_checkinteger(l, 5);

    add_tile_trigger(x, y, file, addr, trip);
    return 0;
}

static int box_trigger(lua_State *l) {
    lua_Integer x = luaL_checkinteger(l, 1);
    lua_Integer y = luaL_checkinteger(l, 2);
    lua_Integer w = luaL_checkinteger(l, 3);
    lua_Integer h = luaL_checkinteger(l, 4);
    lua_Integer addr = luaL_checkinteger(l, 5);
    lua_Integer file = luaL_checkinteger(l, 6);
    lua_Integer trip = luaL_checkinteger(l, 7);

    add_box_trigger(x, y, w, h, file, addr, trip);
    return 0;
}

static int request(lua_State *l) {
    lua_Integer cmd = luaL_checkinteger(l, 1);
    const char *obj_type = luaL_checkstring(l, 2);
    lua_Integer num1 = luaL_checkinteger(l, 3);
    lua_Integer num2 = luaL_checkinteger(l, 4);

    //warn("Need to implement: request: cmd: " PRI_LI " obj_type: %s num1: " PRI_LI " num2: " PRI_LI "\n", cmd, obj_type, num1, num2);
    lua_pushinteger(l,
        dsl_request_impl(cmd, atol(obj_type), num1, num2));
    return 1;
}

static int gpl_clone(lua_State *l) {
    lua_Integer obj = luaL_checkinteger(l, 1);
    lua_Integer qty = luaL_checkinteger(l, 2);
    lua_Integer x = luaL_checkinteger(l, 3);
    lua_Integer y = luaL_checkinteger(l, 4);
    lua_Integer priority = luaL_checkinteger(l, 5);
    lua_Integer placement = luaL_checkinteger(l, 6);
    int16_t entry_id = -1;
    gff_palette_t *pal = open_files[RESOURCE_GFF_INDEX].pals->palettes;

    debug("dsl-clone: obj: " PRI_LI ", qty: " PRI_LI ", (" PRI_LI ", "
             PRI_LI ") pri: " PRI_LI ", pla:" PRI_LI "\n", obj, qty, x, y,
        priority, placement);

    for (int i = 0; i < qty; i++) {
        debug("Cloning %d to: %lld, %lld\n", obj, x, y);

        dude_t *dude = entity_create_from_objex(obj);
        dude->mapx = x;
        dude->mapy = y;
        dude->sprite.xoffset = 0;
        dude->sprite.yoffset = 0;
        entry_id = dude->ds_id;

        if (dude) {
            region_move_to_nearest(region_manager_get_current(), dude);
            region_add_entity(region_manager_get_current(), dude);
            port_load_sprite(&(dude->anim), pal, OBJEX_GFF_INDEX, GFF_BMP, dude->sprite.bmp_id,
                (dude->name) ? 2 : 1);
            port_place_entity(dude);
            //TODO: Should reshift the entity?
            dude->anim.scmd = combat_get_scmd(COMBAT_SCMD_STAND_DOWN);
        }
    }

    lua_pushinteger(l, entry_id);
    return 1;
}

static int gpl_hunt(lua_State *l) {
    lua_Integer obj = luaL_checkinteger(l, 1);
    dude_t *dude = NULL;

    entity_list_for_each(region_manager_get_current()->entities, dude) {
        if (dude->ds_id == (int)obj) {
            dude->abilities.hunt = 1;
        } 
    }

    return 0;
}

static int gpl_ask_yes_no(lua_State *l) {
    debug("Must implement yes no!\n");
    lua_pushinteger(l, port_ask_yes_no());
    return 1;
}

static int call_function(lua_State *l) {
    lua_Integer file = luaL_checkinteger(l, 1);
    lua_Integer addr = luaL_checkinteger(l, 2);

    debug("*****************calling file: " PRI_LI " addr: " PRI_LI "\n", file, addr);
    //replay_print("dsl.call_function(" PRI_LI ", " PRI_LI ")\n", file, addr);
    dsl_lua_execute_script(file, addr, 0);

    return 0;
}

static int gpl_exit(lua_State *l) {
    if (luaL_dostring(l, "return")) {
        error("ERORR MUST IMPLEMENT: NEED TO EXIT.\n");
        exit(1);
    }

    return 0;
}

static int lua_narrate_open(lua_State *l) {
    lua_Integer cmd = luaL_checkinteger(l, 1);
    const char *text = luaL_checkstring(l, 2);
    lua_Integer index = luaL_checkinteger(l, 3);

    narrate_open(cmd, text, index);
    debug("I need to do " PRI_LI " with text '%s' at index " PRI_LI "\n", cmd, text, index);

    return 0;
}

static int lua_narrate_show(lua_State *l) {
    lua_pushboolean(l, sol_game_loop_wait_for_signal(WAIT_NARRATE_SELECT));
    return 1;
}

static int lua_play_sound(lua_State *l) {
    lua_Integer bvoc_index = luaL_checkinteger(l, 1);
    debug("I need to play bvoc: " PRI_LI "\n", bvoc_index);
    sol_play_sound_effect(bvoc_index);
    return 0;
}

static int lua_debug(lua_State *l) {
    //printf("%s\n", luaL_checkstring(l, 1));
    return 0;
}


static const struct luaL_Reg gpl_state_lib[] = {
    {"set_while_callback", set_while_callback},
    {"set_gf", set_gf},
    {"get_gf", get_gf},
    {"set_lf", set_lf},
    {"get_lf", get_lf},
    {"set_gn", set_gn},
    {"get_gn", get_gn},
    {"set_ln", set_ln},
    {"get_ln", get_ln},
    {"set_gbn", set_gbn},
    {"get_gbn", get_gbn},
    {"set_lbn", set_lbn},
    {"get_lbn", get_lbn},
    {"set_gstr", set_gstr},
    {"get_gstr", get_gstr},
    {"get_gname", get_gname},
    {"set_gname", set_gname},
    {"get_type", get_type},
    {"get_id", get_id},
    {"get_party", get_party},
    {"is_true", is_true},
    {"getX", gpl_getX},
    {"getY", gpl_getY},
    {"read_complex", read_complex},
    {"rand", gpl_rand},
    {"range", range},
    {"attack_trigger", attack_trigger},
    {"attack_trigger_global", attack_trigger_global},
    {"noorders_trigger", noorders_trigger},
    {"use_trigger", use_trigger},
    {"use_trigger_global", use_trigger_global},
    {"look_trigger", look_trigger},
    {"look_trigger_global", look_trigger_global},
    {"talk_to_trigger", talk_to_trigger},
    {"use_with_trigger", use_with_trigger},
    {"tile_trigger", tile_trigger},
    {"box_trigger", box_trigger},
    {"call_function", call_function},
    {"request", request},
    {"clone", gpl_clone},
    {"hunt", gpl_hunt},
    {"ask_yes_no", gpl_ask_yes_no},
    {"exit", gpl_exit},
    {"narrate_open", lua_narrate_open},
    {"narrate_show", lua_narrate_show},
    {"play_sound", lua_play_sound},
    {"debug", lua_debug},
    {NULL, NULL}
} ;

static void set_globals(lua_State *l) {
    char buf[BUF_SIZE];
    snprintf(buf, BUF_SIZE, "NAR_ADD_MENU = %d\n", NAR_ADD_MENU);
    luaL_dostring(l, buf);
    snprintf(buf, BUF_SIZE, "NAR_PORTRAIT = %d\n", NAR_PORTRAIT);
    luaL_dostring(l, buf);
    snprintf(buf, BUF_SIZE, "NAR_SHOW_TEXT = %d\n", NAR_SHOW_TEXT);
    luaL_dostring(l, buf);
    snprintf(buf, BUF_SIZE, "NAR_SHOW_MENU = %d\n", NAR_SHOW_MENU);
    luaL_dostring(l, buf);
    snprintf(buf, BUF_SIZE, "NAR_EDIT_BOX = %d\n", NAR_EDIT_BOX);
    luaL_dostring(l, buf);
}

void gpl_state_register(lua_State *l) {
    set_globals(l);
    lua_newtable(l);
    luaL_setfuncs(l, gpl_state_lib, 0);
    lua_setglobal(l, "dsl");
}

void gpl_local_clear() {
    memset(gpl_local_flags, 0x0, sizeof(int8_t) * MAX_LFLAGS);
    memset(gpl_local_nums, 0x0, sizeof(int16_t) * MAX_LNUMS);
    memset(gpl_local_bnums, 0x0, sizeof(int32_t) * MAX_LBIGNUMS);
    //memset(gpl_local_bnums, 0x0, sizeof(int32_t) * MAX_LBIGNUMS); string!
}

char* gpl_serialize_globals(uint32_t *len) {
    *len =
        sizeof(gpl_global_flags) +
        sizeof(gpl_global_nums) +
        sizeof(gpl_global_bnums) +
        sizeof(gpl_global_strs) +
        sizeof(gpl_gnames);
    char *ret = malloc(*len);
    char *buf = ret;
    memcpy(buf, gpl_global_flags, sizeof(gpl_global_flags));
    buf += sizeof(gpl_global_flags);
    memcpy(buf, gpl_global_nums, sizeof(gpl_global_nums));
    buf += sizeof(gpl_global_nums);
    memcpy(buf, gpl_global_bnums, sizeof(gpl_global_bnums));
    buf += sizeof(gpl_global_bnums);
    memcpy(buf, gpl_global_strs, sizeof(gpl_global_strs));
    buf += sizeof(gpl_global_strs);
    memcpy(buf, gpl_gnames, sizeof(gpl_gnames));
    buf += sizeof(gpl_gnames);

    return ret;
}

void gpl_deserialize_globals(char *buf) {
    memcpy(gpl_global_flags, buf, sizeof(gpl_global_flags));
    buf += sizeof(gpl_global_flags);
    memcpy(gpl_global_nums, buf, sizeof(gpl_global_nums));
    buf += sizeof(gpl_global_nums);
    memcpy(gpl_global_bnums, buf, sizeof(gpl_global_bnums));
    buf += sizeof(gpl_global_bnums);
    memcpy(gpl_global_strs, buf, sizeof(gpl_global_strs));
    buf += sizeof(gpl_global_strs);
    memcpy(gpl_gnames, buf, sizeof(gpl_gnames));
    buf += sizeof(gpl_gnames);
}

void gpl_deserialize_locals(char *buf) {
    memcpy(gpl_local_flags, buf, sizeof(gpl_local_flags));
    buf += sizeof(gpl_local_flags);
    memcpy(gpl_local_nums, buf, sizeof(gpl_local_nums));
    buf += sizeof(gpl_local_nums);
    memcpy(gpl_local_bnums, buf, sizeof(gpl_local_bnums));
    buf += sizeof(gpl_local_bnums);
}

char* gpl_serialize_locals(uint32_t *len) {
    *len =
        sizeof(gpl_local_flags) +
        sizeof(gpl_local_nums) +
        sizeof(gpl_local_bnums);
    char *ret = malloc(*len);
    char *buf = ret;
    memcpy(buf, gpl_local_flags, sizeof(gpl_local_flags));
    buf += sizeof(gpl_local_flags);
    memcpy(buf, gpl_local_nums, sizeof(gpl_local_nums));
    buf += sizeof(gpl_local_nums);
    memcpy(buf, gpl_local_bnums, sizeof(gpl_local_bnums));
    buf += sizeof(gpl_local_bnums);

    return ret;
}
