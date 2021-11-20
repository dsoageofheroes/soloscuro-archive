#include <math.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

#include "lua-inc.h"
#include "dsl.h"
#include "ssi-object.h"
#include "dsl-var.h"
#include "gff.h"
#include "gff-image.h"
#include "gff-xmi.h"
#include "gff-map.h"
#include "gfftypes.h"

/* Helper functions */

static void create_table_entry_si(lua_State *L, const char *key, const int val) {
    lua_pushstring(L, key);
    lua_pushinteger(L, val);
    lua_settable(L, -3);
}

static void create_table_entry_ss(lua_State *L, const char *key, const char *val) {
    lua_pushstring(L, key);
    lua_pushstring(L, val);
    lua_settable(L, -3);
}

static void push_ds1_combat(lua_State *L, ds1_combat_t *dc);
//static void push_ds1_monster(lua_State *L, gff_monster_entry_t *me);
static void push_ds1_item(lua_State *L, ds1_item_t *dc);
static void push_scmd(lua_State *L, scmd_t *script);
/* End Helper Functions */

static int lua_gff_init(lua_State *L) {
    lua_state = L;
    gff_init();
    
    return 0;
}

int lua_gff_load_directory(lua_State *L) {
    size_t len;
    const char *path = luaL_checklstring(L, 1, &len);

    gff_load_directory(path);
    dsl_init();

    return 0;
}

#define BUF_SIZE (1024)
int lua_gff_find_index(lua_State *L) {
    size_t len;
    const char *name = luaL_checklstring(L, 1, &len);
    char buf[1024];

    if (len >= (BUF_SIZE-1)) { return -1; }

    for (int i = 0; i < len; i++) {
        buf[i] = tolower(name[i]);
    }
    buf[len] = '\0';

    lua_pushinteger(L, gff_find_index(buf));

    return 1;
}

int lua_gff_open(lua_State *L) {
    size_t len;
    const char *path = luaL_checklstring(L, 1, &len);

    int id = gff_open(path);
    lua_pushinteger(L, id);

    return 1;
}

int lua_gff_list(lua_State *L) {
    size_t len;
    const char** files = gff_list(&len);

    lua_createtable(L, len, 0);
    for (size_t i = 0; i < len; i++) {
        lua_pushstring(L, files[i]);
        lua_rawseti(L, -2, i + 1);
    }

    free(files);
    printf("returning...\n");

    return 1;
}

int lua_gff_get_number_of_types(lua_State *L) {
    int idx = luaL_checkinteger (L, 1);

    lua_pushinteger(L, gff_get_number_of_types(idx));

    return 1;
}

int lua_gff_get_number_of_palettes(lua_State *L) {
    lua_pushinteger(L, gff_get_number_of_palettes());
    return 1;
}

int lua_gff_get_type_id(lua_State *L) {
    int idx = luaL_checkinteger(L, 1);
    int type_index = luaL_checkinteger(L, 2);

    lua_pushinteger(L, gff_get_type_id(idx, type_index));

    return 1;
}

int lua_get_gff_type_name(lua_State *L) {
    char buf[1024];
    int gff_type = luaL_checkinteger(L, 1);

    gff_get_gff_type_name(gff_type, buf);

    lua_pushstring(L, buf);

    return 1;
}

int lua_gff_get_id_list(lua_State *L) {
    int idx = luaL_checkinteger(L, 1);
    int type_id = luaL_checkinteger(L, 2) & GFFMAXCHUNKMASK;

    lua_createtable(L, 0, 0);

    unsigned int num_ids = gff_get_resource_length(idx, type_id);
    unsigned int *ids = gff_get_id_list(idx, type_id);

    for (unsigned int i = 0; i < num_ids; i++) {
        lua_pushinteger(L, i + 1);
        lua_pushinteger(L, ids[i]);
        lua_settable(L, -3);
    }

    free(ids);

    return 1;
}

#define BUF_MAX (1<<12)

int lua_gff_get_data_as_text(lua_State *L) {
    int idx = luaL_checkinteger(L, 1);
    int type_id = luaL_checkinteger(L, 2) & GFFMAXCHUNKMASK;
    int res_id = luaL_checkinteger(L, 3);
    char buf[BUF_MAX];
    gff_chunk_header_t chunk = gff_find_chunk_header(idx, type_id, res_id);
    gff_read_chunk(idx, &chunk, buf, BUF_MAX);

    buf[chunk.length - 1] = '\0';
    lua_pushstring(L, buf);

    return 1;
}

int lua_gff_write_raw_bytes(lua_State *L) {
    int idx = luaL_checkinteger(L, 1);
    int type_id = luaL_checkinteger(L, 2) & GFFMAXCHUNKMASK;
    int res_id = luaL_checkinteger(L, 3);
    const char *path = luaL_checkstring(L, 4);

    lua_pushinteger(L, gff_write_raw_bytes(idx, type_id, res_id, path));

    return 1;
}

int lua_gff_close (lua_State *L) {
    lua_Integer id = luaL_checkinteger (L, 1);

    gff_close(id);

    return 0;
}

int lua_gff_cleanup (lua_State *L) {
    gff_cleanup();

    return 0;
}

/* Image functions */

int lua_get_frame_count(lua_State *L) {
    lua_Integer gff_index = luaL_checkinteger (L, 1);
    lua_Integer type_id = luaL_checkinteger (L, 2);
    lua_Integer res_id = luaL_checkinteger (L, 3);

    lua_pushinteger(L, gff_get_frame_count(gff_index, type_id, res_id));

    return 1;
}

int lua_get_frame_width(lua_State *L) {
    lua_Integer gff_index = luaL_checkinteger (L, 1);
    lua_Integer type_id = luaL_checkinteger (L, 2);
    lua_Integer res_id = luaL_checkinteger (L, 3);
    lua_Integer frame_id = luaL_checkinteger (L, 4);

    lua_pushinteger(L, gff_get_frame_width(gff_index, type_id, res_id, frame_id));

    return 1;
}

int lua_get_frame_height(lua_State *L) {
    lua_Integer gff_index = luaL_checkinteger (L, 1);
    lua_Integer type_id = luaL_checkinteger (L, 2);
    lua_Integer res_id = luaL_checkinteger (L, 3);
    lua_Integer frame_id = luaL_checkinteger (L, 4);

    lua_pushinteger(L, gff_get_frame_height(gff_index, type_id, res_id, frame_id));

    return 1;
}

int lua_get_frame_rgba(lua_State *L) {
    lua_Integer gff_index = luaL_checkinteger (L, 1);
    lua_Integer type_id = luaL_checkinteger (L, 2);
    lua_Integer res_id = luaL_checkinteger (L, 3);
    lua_Integer frame_id = luaL_checkinteger (L, 4);

    int w = gff_get_frame_width(gff_index, type_id, res_id, frame_id);
    int h = gff_get_frame_height(gff_index, type_id, res_id, frame_id);
    char* data = (char*)gff_get_frame_rgba_with_palette(gff_index, type_id, res_id, frame_id, -1);

    if (data == NULL) {
        lua_pushinteger(L, 0);
    } else {
        lua_pushlstring(L, data, 4 * w * h);
        free(data);
    }

    return 1;
}

int lua_get_portrait(lua_State *L) {
    lua_Integer res_id = luaL_checkinteger (L, 1);
    unsigned int w, h;
    unsigned char buf[BUF_MAX];
    gff_chunk_header_t chunk = gff_find_chunk_header(DSLDATA_GFF_INDEX, GFF_PORT, res_id);
    gff_read_chunk(DSLDATA_GFF_INDEX, &chunk, buf, BUF_MAX);

    char *data = (char*)gff_get_portrait(buf, &w, &h);
    if (data == NULL) {
        lua_pushinteger(L, 0);
        lua_pushinteger(L, 0);
        lua_pushinteger(L, 0);
    } else {
        lua_pushlstring(L, data, 4 * w * h);
        lua_pushinteger(L, w);
        lua_pushinteger(L, h);
        free(data);
    }

    return 3;
}

int lua_get_frame_rgba_with_palette(lua_State *L) {
    lua_Integer gff_index = luaL_checkinteger (L, 1);
    lua_Integer type_id = luaL_checkinteger (L, 2);
    lua_Integer res_id = luaL_checkinteger (L, 3);
    lua_Integer frame_id = luaL_checkinteger (L, 4);
    lua_Integer palette_id = luaL_checkinteger (L, 5);

    int w = gff_get_frame_width(gff_index, type_id, res_id, frame_id);
    int h = gff_get_frame_height(gff_index, type_id, res_id, frame_id);
    char* data = (char*)gff_get_frame_rgba_with_palette(gff_index, type_id, res_id, frame_id, palette_id);

    if (data == NULL) {
        lua_pushinteger(L, 0);
    } else {
        lua_pushlstring(L, data, 4 * w * h);
        free(data);
    }

    return 1;
}
/* End Image Functions */

/* Sound Functions */
int lua_get_chunk_as_midi(lua_State *L) {
    lua_Integer gff_idx = luaL_checkinteger (L, 1);
    lua_Integer type_id = luaL_checkinteger (L, 2);
    lua_Integer res_id = luaL_checkinteger (L, 3);
    unsigned long xmi_len = 0;
    unsigned int midi_len = 0;
    char buf[BUF_MAX];
    gff_chunk_header_t chunk = gff_find_chunk_header(gff_idx, type_id, res_id);
    gff_read_chunk(gff_idx, &chunk, buf, BUF_MAX);

    unsigned char* midi_data = gff_xmi_to_midi((unsigned char*)buf, xmi_len, &midi_len);
    FILE *tmp = fopen("t.mid", "wb+");
    fwrite(midi_data, 1, midi_len, tmp);
    fclose(tmp);

    if (midi_data == NULL) {
        lua_pushinteger(L, 0);
    } else {
        lua_pushlstring(L, (char*)midi_data, midi_len);
        free(midi_data);
    }

    return 1;
}
/* End Sound Functions */

/* Begin Map Functions */

/*
int lua_load_tile_ids(lua_State *L) {
    lua_Integer gff_index = luaL_checkinteger (L, 1);
    lua_Integer res_id = luaL_checkinteger (L, 2);

    gff_load_map_tile_ids(gff_index, res_id);

    return 0;
}

int lua_load_map_flags(lua_State *L) {
    lua_Integer gff_index = luaL_checkinteger (L, 1);
    lua_Integer res_id = luaL_checkinteger (L, 2);

    gff_load_map_flags(gff_index, res_id);

    return 0;
}

int lua_load_map(lua_State *L) {
    lua_Integer gff_index = luaL_checkinteger (L, 1);

    gff_load_map(gff_index);

    return 0;
}

int lua_get_tile_id(lua_State *L) {
    lua_Integer gff_index = luaL_checkinteger (L, 1);
    lua_Integer row = luaL_checkinteger (L, 2);
    lua_Integer column = luaL_checkinteger (L, 3);

    lua_pushinteger(L, get_tile_id(gff_index, row, column));

    return 1;
}
*/

/*
int lua_map_is_block(lua_State *L) {
    lua_Integer gff_index = luaL_checkinteger (L, 1);
    lua_Integer row = luaL_checkinteger (L, 2);
    lua_Integer column = luaL_checkinteger (L, 3);

    lua_pushboolean(L, gff_map_is_block(gff_index, row, column) > 0);

    return 1;
}

int lua_map_is_actor(lua_State *L) {
    lua_Integer gff_index = luaL_checkinteger (L, 1);
    lua_Integer row = luaL_checkinteger (L, 2);
    lua_Integer column = luaL_checkinteger (L, 3);

    lua_pushboolean(L, gff_map_is_actor(gff_index, row, column) > 0);

    return 1;
}

int lua_map_is_danger(lua_State *L) {
    lua_Integer gff_index = luaL_checkinteger (L, 1);
    lua_Integer row = luaL_checkinteger (L, 2);
    lua_Integer column = luaL_checkinteger (L, 3);

    lua_pushboolean(L, gff_map_is_danger(gff_index, row, column) > 0);

    return 1;
}
*/

int lua_map_get_number_of_objects(lua_State *L) {
    lua_Integer gff_index = luaL_checkinteger (L, 1);
    lua_Integer res_id = luaL_checkinteger (L, 2);

    lua_pushinteger(L, gff_map_get_num_objects(gff_index, res_id));

    return 1;
}

int lua_map_get_object_frame_count(lua_State *L) {
    lua_Integer gff_index = luaL_checkinteger (L, 1);
    lua_Integer res_id = luaL_checkinteger (L, 2);
    lua_Integer obj_id = luaL_checkinteger (L, 3);

    lua_pushinteger(L, gff_map_get_object_frame_count(gff_index, res_id, obj_id));

    return 1;
}

int lua_map_get_object_scmd(lua_State *L) {
    lua_Integer gff_index = luaL_checkinteger (L, 1);
    lua_Integer res_id = luaL_checkinteger (L, 2);
    lua_Integer obj_id = luaL_checkinteger (L, 3);
    lua_Integer scmd_index = luaL_checkinteger (L, 4);

    scmd_t* script = gff_map_get_object_scmd(gff_index, res_id, obj_id, scmd_index);

    //lua_createtable(L, 0, 0);
    push_scmd(L, script);

    return 1;
}

int lua_map_get_object_bmp(lua_State *L) {
    lua_Integer gff_index = luaL_checkinteger (L, 1);
    lua_Integer res_id = luaL_checkinteger (L, 2);
    lua_Integer obj_id = luaL_checkinteger (L, 3);
    lua_Integer frame_id = luaL_checkinteger (L, 4);

    int w, h;

    char *data = (char*)gff_map_get_object_bmp(gff_index, res_id, obj_id, &w, &h, frame_id);

    if (data == NULL) {
        lua_pushinteger(L, 0);
    } else {
        lua_pushlstring(L, data, 4 * w * h);
        free(data);
    }

    lua_pushinteger(L, w);
    lua_pushinteger(L, h);

    return 3;
}

int lua_map_get_object_location(lua_State *L) {
    lua_Integer gff_index = luaL_checkinteger (L, 1);
    lua_Integer res_id = luaL_checkinteger (L, 2);
    lua_Integer obj_id = luaL_checkinteger (L, 3);

    uint16_t x, y;
    uint8_t z;
    gff_map_get_object_location(gff_index, res_id, obj_id, &x, &y, &z);

    lua_pushinteger(L, x);
    lua_pushinteger(L, y);
    lua_pushinteger(L, z);

    return 3;
}
/* End Map Functions */

static void push_ds1_combat(lua_State *L, ds1_combat_t *dc);

/* Object Functions */
int lua_object_inspect(lua_State *L) {
    lua_Integer gff_index = luaL_checkinteger (L, 1);
    lua_Integer res_id = luaL_checkinteger (L, 2);

    so_object_t* so = gff_object_inspect(gff_index, res_id);
    lua_createtable(L, 0, 0);

    //printf("type = %d, %d\n", so->type, SO_DS1_COMBAT);
    switch (so->type) {
        case SO_DS1_COMBAT:
            push_ds1_combat(L, &(so->data.ds1_combat));
            break;
        case SO_DS1_ITEM:
            push_ds1_item(L, &(so->data.ds1_item));
            break;
        case SO_EMPTY:
        default:
            create_table_entry_si(L, "type", SO_EMPTY);
            break;
    }

    free(so);
    return 1;
}

/*int lua_load_monster(lua_State *L) {
    lua_Integer region_id = luaL_checkinteger (L, 1);
    lua_Integer monster_id = luaL_checkinteger (L, 2);

    gff_monster_entry_t* me = gff_load_monster(region_id, monster_id);

    lua_createtable(L, 0, 0);
    push_ds1_monster(L, me);

    return 1;
}
*/

int lua_create_font_img(lua_State *L) {
    lua_Integer gff_idx = luaL_checkinteger (L, 1);
    lua_Integer c = luaL_checkinteger (L, 2);
    lua_Integer fg_color = luaL_checkinteger (L, 3);
    lua_Integer bg_color = luaL_checkinteger (L, 4);
    char buf[BUF_MAX];
    gff_chunk_header_t chunk = gff_find_chunk_header(gff_idx, GFF_FONT, 100);
    gff_read_chunk(gff_idx, &chunk, buf, BUF_MAX);

    ds_font_t *font = (ds_font_t*)buf;
    ds_char_t *ds_char = (ds_char_t*)(((uint8_t*)font) + font->char_offset[c]);
    char *data = (char*)gff_create_font_rgba(gff_idx, c, fg_color, bg_color);

    if (data == NULL) {
        lua_pushinteger(L, 0);
    } else {
        lua_pushlstring(L, data, 4 * font->height * ds_char->width);
        free(data);
    }

    lua_pushinteger(L, ds_char->width);
    lua_pushinteger(L, font->height);

    return 3;
}

int lua_font_count(lua_State *L) {
    lua_Integer gff_idx = luaL_checkinteger (L, 1);
    char buf[BUF_MAX];
    gff_chunk_header_t chunk = gff_find_chunk_header(gff_idx, GFF_FONT, 100);
    gff_read_chunk(gff_idx, &chunk, buf, BUF_MAX);

    ds_font_t *font = (ds_font_t*)buf;

//    create_font_rgba(gff_idx, 65);

    lua_pushinteger(L, font->num);

    return 1;
}

// SCMD Functions
static int lua_scmd_len(lua_State *L) {
    int len = 1;
    scmd_t *cmd = (scmd_t *)lua_touserdata(L, 1);
    if (!cmd) { 
        lua_pushnumber(L, 0);
        return 1;
    }
    while (!(cmd->flags & SCMD_LAST)) {
        cmd++;
        len++;
    }
    lua_pushnumber(L, len);
    return 1;
}

static scmd_t* get_cmd(lua_State *L) {
    scmd_t *cmd = (scmd_t *)lua_touserdata(L, 1);
    lua_Integer index = luaL_checkinteger(L, 2);
    luaL_argcheck(L, cmd != NULL, 1, "`scmd' expected");
    cmd += index;
    return cmd;
}

static int lua_scmd_index_is_default(lua_State *L) {
    scmd_t *scmd = (scmd_t *)lua_touserdata(L, 1);
    lua_Integer scmd_index = luaL_checkinteger(L, 2);
    lua_pushboolean(L, ssi_scmd_is_default(scmd, scmd_index));
    return 1;
}

static int lua_scmd_bmp_idx(lua_State *L) {
    scmd_t *cmd = get_cmd(L);
    lua_pushnumber(L, cmd->bmp_idx);
    return 1;
}

static int lua_scmd_flag(lua_State *L, uint8_t flag) {
    scmd_t *cmd = get_cmd(L);
    lua_pushboolean(L, cmd->flags & flag);
    return 1;
}

static int lua_scmd_jump(lua_State *L) {return lua_scmd_flag(L, SCMD_JUMP);}
static int lua_scmd_last(lua_State *L) {return lua_scmd_flag(L, SCMD_LAST);}
static int lua_scmd_xmirror(lua_State *L) {return lua_scmd_flag(L, SCMD_XMIRROR);}
static int lua_scmd_ymirror(lua_State *L) {return lua_scmd_flag(L, SCMD_YMIRROR);}
static int lua_scmd_moving(lua_State *L) {return lua_scmd_flag(L, SCMD_MOVING);}
static int lua_scmd_combat(lua_State *L) {return lua_scmd_flag(L, SCMD_COMBAT);}
static int lua_scmd_ok_hot(lua_State *L) {return lua_scmd_flag(L, SCMD_OK_HOT);}

static int lua_scmd_delay(lua_State *L) {
    scmd_t *cmd = get_cmd(L);
    lua_pushnumber(L, cmd->delay);
    return 1;
}

static int lua_scmd_xoffset(lua_State *L) {
    scmd_t *cmd = get_cmd(L);
    lua_pushnumber(L, cmd->xoffset);
    return 1;
}

static int lua_scmd_yoffset(lua_State *L) {
    scmd_t *cmd = get_cmd(L);
    lua_pushnumber(L, cmd->yoffset);
    return 1;
}

static int lua_scmd_xoffsethot(lua_State *L) {
    scmd_t *cmd = get_cmd(L);
    lua_pushnumber(L, cmd->xoffsethot);
    return 1;
}

static int lua_scmd_yoffsethot(lua_State *L) {
    scmd_t *cmd = get_cmd(L);
    lua_pushnumber(L, cmd->yoffsethot);
    return 1;
}

static int lua_scmd_sound_idx(lua_State *L) {
    scmd_t *cmd = get_cmd(L);
    lua_pushnumber(L, cmd->soundidx);
    return 1;
}

static int lua_dsl_change_region(lua_State *L) {
    lua_Integer region_id = luaL_checkinteger (L, 1);
    dsl_change_region(region_id);
    return 0;
}

#define CREATE_SET_GET_CHARACTER_LUA_ENTRIES(NAME) \
    { "get_char_"#NAME, lua_get_ ## NAME }, \
    { "set_char_"#NAME, lua_set_ ## NAME }

/* End Object Functions */

//library to be registered
static const struct luaL_Reg lslib [] = {
      {"gff_init", lua_gff_init},
      {"gff_load_directory", lua_gff_load_directory},
      {"gff_find_index", lua_gff_find_index},
      {"gff_open", lua_gff_open},
      {"gff_list", lua_gff_list},
      {"gff_close", lua_gff_close},
      {"gff_cleanup", lua_gff_cleanup},
      {"gff_get_number_of_types", lua_gff_get_number_of_types},
      {"gff_get_number_of_palettes", lua_gff_get_number_of_palettes},
      //{"gff_get_palette_id", lua_gff_get_palette_id},
      {"gff_get_type_id", lua_gff_get_type_id},
      {"get_gff_type_name", lua_get_gff_type_name},
      {"gff_get_id_list", lua_gff_get_id_list},
      {"gff_get_data_as_text", lua_gff_get_data_as_text},
      {"gff_write_raw_bytes", lua_gff_write_raw_bytes},

      // Image Functions
      {"get_frame_count", lua_get_frame_count},
      {"get_frame_width", lua_get_frame_width},
      {"get_frame_height", lua_get_frame_height},
      {"get_frame_rgba", lua_get_frame_rgba},
      {"get_frame_rgba_with_palette", lua_get_frame_rgba_with_palette},
      {"get_portrait", lua_get_portrait},

      // Sound Functions
      {"get_chunk_as_midi", lua_get_chunk_as_midi},

      // Map Functions
      //{"load_tile_ids", lua_load_tile_ids},
      //{"load_map_flags", lua_load_map_flags},
      //{"load_map", lua_load_map},
      //{"get_tile_id", lua_get_tile_id},
      //{"map_is_block", lua_map_is_block},
      //{"map_is_actor", lua_map_is_actor},
      //{"map_is_danger", lua_map_is_danger},
      {"map_get_number_of_objects", lua_map_get_number_of_objects},
      {"map_get_object_frame_count", lua_map_get_object_frame_count},
      {"map_get_object_bmp", lua_map_get_object_bmp},
      {"map_get_object_location", lua_map_get_object_location},
      {"map_get_object_scmd", lua_map_get_object_scmd},

      // Font functions
      {"font_count", lua_font_count},
      {"create_font_img", lua_create_font_img},

      // Object Functions
      {"object_inspect", lua_object_inspect},
      //{"load_monster", lua_load_monster},

      // SCMD functions
      {"scmd_len", lua_scmd_len},
      {"scmd_bmp_idx", lua_scmd_bmp_idx},
      {"scmd_delay", lua_scmd_delay},
      {"scmd_xoffset", lua_scmd_xoffset},
      {"scmd_yoffset", lua_scmd_yoffset},
      {"scmd_xoffsethot", lua_scmd_xoffsethot},
      {"scmd_yoffsethot", lua_scmd_yoffsethot},
      {"scmd_sound_idx", lua_scmd_sound_idx},
      {"scmd_jump", lua_scmd_jump},
      {"scmd_last", lua_scmd_last},
      {"scmd_xmirror", lua_scmd_xmirror},
      {"scmd_ymirror", lua_scmd_ymirror},
      {"scmd_moving", lua_scmd_moving},
      {"scmd_combat", lua_scmd_combat},
      {"scmd_ok_hot", lua_scmd_ok_hot},
      {"scmd_index_is_default", lua_scmd_index_is_default},

      // MAS functions
      {"dsl_change_region", lua_dsl_change_region},

      // The End
      {NULL, NULL}  /* sentinel */
    };

// How Lua registers the functions.
int luaopen_libds (lua_State *L){
    debug("Registering functions...\n");
    fflush(stdout);
    printf("lslib = %p\n", lslib);
    //luaL_register(L, "libds", lslib);
    debug("Functions Registered.\n");
    return 1;
}

static void push_scmd(lua_State *L, scmd_t *script) {
    scmd_t *cmd = script;
    int len = 1;
    if (!script) { return; }
    while (!(cmd->flags & SCMD_LAST)) {
        cmd++;
        len++;
    }
    void *lscmd = lua_newuserdata(L, sizeof(scmd_t) * len);
    memcpy(lscmd, script, sizeof(scmd_t) * len);
}

static void push_ds1_combat(lua_State *L, ds1_combat_t *dc) {
    create_table_entry_si(L, "type", SO_DS1_COMBAT);
    create_table_entry_si(L, "hp", dc->hp);
    create_table_entry_si(L, "psp", dc->psp);
    create_table_entry_si(L, "id", abs(dc->id));
    create_table_entry_si(L, "ac", dc->ac);
    create_table_entry_si(L, "move", dc->move);
    create_table_entry_si(L, "thac0", dc->thac0);
    create_table_entry_si(L, "str", dc->stats.str);
    create_table_entry_si(L, "dex", dc->stats.dex);
    create_table_entry_si(L, "con", dc->stats.con);
    create_table_entry_si(L, "int", dc->stats.intel);
    create_table_entry_si(L, "wis", dc->stats.wis);
    create_table_entry_si(L, "cha", dc->stats.cha);
    create_table_entry_ss(L, "name", dc->name);
}

static void push_ds1_item(lua_State *L, ds1_item_t *di) {
    create_table_entry_si(L, "type", SO_DS1_ITEM);
    create_table_entry_si(L, "id", abs(di->id));
    create_table_entry_si(L, "quantity", di->quantity);
    create_table_entry_si(L, "value", di->value);
    create_table_entry_si(L, "icon", di->icon);
    create_table_entry_si(L, "charges", di->charges);
    create_table_entry_si(L, "special", di->special);
    create_table_entry_si(L, "slot", di->slot);
    create_table_entry_si(L, "name_index", di->name_idx);
    //disk_object_t* diskobject = gff_get_object(di->id);
    disk_object_t diskobject;
    gff_read_object(di->id, &diskobject);
    create_table_entry_si(L, "bmp_id", diskobject.bmp_id);
    create_table_entry_si(L, "script_id", diskobject.script_id);
}
