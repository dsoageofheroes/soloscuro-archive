#include <lua5.3/lua.h>
#include <lua5.3/lualib.h>
#include <lua5.3/lauxlib.h>
#include <string.h>
#include <stdlib.h>
#include "dsl.h"
#include "dsl-manager.h"
#include "dsl-lua.h"
#include "dsl-state.h"
#include "gff.h"
#include "gfftypes.h"

static char** mas_scripts = NULL;
static char** gpl_scripts = NULL;
static size_t mas_max = 0, gpl_max = 0;
static lua_State *clua = NULL;

static void write_lua(const char *path, const char *lua, const size_t len);
static void dsl_lua_load_scripts();

void dsl_manager_init() {
    dsl_state_init();

    mas_scripts = gpl_scripts = NULL;

    dsl_lua_load_scripts();
}

static void write_lua(const char *path, const char *lua, const size_t len) {
    FILE *file = fopen(path, "w+");
    if (file) {
        fwrite(lua, 1, len, file);
        fclose(file);
    } else {
        fprintf(stderr, "Warning, unable to write to %s\n", path);
    }
}

#define DSL_MAX (1<<14)
void dsl_lua_load_script(const uint32_t script_id, const uint8_t is_mas) {
    size_t script_len;
    unsigned char dsl[DSL_MAX];
    char buf[1024];
    char **script = is_mas ? mas_scripts : gpl_scripts;
    char *script_ptr;

    gff_chunk_header_t chunk = gff_find_chunk_header(DSLDATA_GFF_INDEX,
        is_mas ? GFF_MAS : GFF_GPL, script_id);
    if (chunk.length > DSL_MAX) {
        error("DSL chunk size %d is larger than max (%d)\n", chunk.length, DSL_MAX);
    }
    if (!gff_read_chunk(DSLDATA_GFF_INDEX, &chunk, dsl, chunk.length)) { return; }

    printf("Coverting %s %d to lua, length = %d\n",
        is_mas ? "MAS" : "GPL",
        script_id, chunk.length);
    //script_ptr = dsl_lua_print(dsl, len, is_master_mas, &script_len);
    script_ptr = dsl_lua_print(script_id, is_mas, &script_len);
    script[script_id] = malloc(sizeof(char) * (script_len + 1)); // (A)
    strncpy(script[script_id], script_ptr, script_len);
    script[script_id][script_len] = '\0'; // Okay because of (A)
    sprintf(buf, "lua/%d-%s.lua", script_id,
        is_mas ? "mas" : "gpl");
    write_lua(buf, script[script_id], script_len);
}

void dsl_lua_load_scripts() {
    unsigned int *ids;
    size_t i, amt;

    ids = gff_get_id_list(DSLDATA_GFF_INDEX, GFF_MAS);
    amt = gff_get_resource_length(DSLDATA_GFF_INDEX, GFF_MAS);
    printf("Detected %ld master GPL files.\n", amt);
    for (i = 0; i < amt; i++) { mas_max = mas_max > ids[i] ? mas_max : ids[i]; }
    mas_max++;
    mas_scripts = malloc(sizeof(char*) * mas_max);
    memset(mas_scripts, 0x0, sizeof(char*) * mas_max);
    free(ids);

    ids = gff_get_id_list(DSLDATA_GFF_INDEX, GFF_GPL);
    amt = gff_get_resource_length(DSLDATA_GFF_INDEX, GFF_GPL);
    printf("Detected %ld standard GPL files.\n", amt);
    for (i = 0; i < amt; i++) { gpl_max = gpl_max > ids[i] ? gpl_max : ids[i]; }
    gpl_max++;
    gpl_scripts = malloc(sizeof(char*) * gpl_max);
    memset(gpl_scripts, 0x0, sizeof(char*) * gpl_max);
    free(ids);
}

uint8_t dsl_lua_execute_script(size_t file, size_t addr, uint8_t is_mas) {
    lua_State *l = NULL;
    char **scripts = is_mas ? mas_scripts : gpl_scripts;
    size_t size = is_mas ? mas_max : gpl_max;
    char func[BUF_SIZE];
    int ret = 1;

    debug("execute_script(%ld, %ld, %d)\n", file, addr, is_mas);
    snprintf(func, BUF_SIZE, "func%ld", addr);

    if (file < 0 || file >= size) { return 0; }

    if (scripts[file] == NULL) {
        dsl_lua_load_script(file, is_mas);
    }

    if (scripts[file] == NULL) { return 0; }

    clua = l = luaL_newstate();
    luaL_openlibs(l);
    dsl_state_register(l);
    if (luaL_dostring(l, scripts[file])) {
        printf("Error: unable to load %s script %ld:%ld\n",
            is_mas ? "MAS" : "GPL",
            file, addr);
        printf("error: %s\n", lua_tostring(l, -1));
        return 0;
    }
    lua_getglobal(l, func);
    if (lua_pcall(l, 0, 0, 0)) {
        printf("error running function: '%s' in %s file %ld, addr %ld\n", func,
            is_mas ? "MAS" : "GPL",
            file, addr);
        printf("error: %s\n", lua_tostring(l, -1));
        ret = 0;
    }

    printf("*******************CLOSING LUA****************************\n");
    clua = NULL;
    lua_close(l);
    debug("exiting execute_script(%ld, %ld, %d)\n", file, addr, is_mas);

    return ret;
}

void dsl_execute_string(const char *str) {
    if (clua) {
        if (luaL_dostring(clua, str)) {
            error("Unable to execute '%s'!\n", str);
            error("%s\n", lua_tostring(clua, -1));
        }
    } else {
        error("Unable to execute '%s', no lua context!\n", str);
    }
}

void dsl_manager_cleanup() {
    int i;

    if (mas_scripts) {
        for (i = 0; i < mas_max; i++) {
            if (mas_scripts[i]) {
                free(mas_scripts[i]);
                mas_scripts[i] = NULL;
            }
        }
        free(mas_scripts);
    }

    if (gpl_scripts) {
        for (i = 0; i < gpl_max; i++) {
            if (gpl_scripts[i]) {
                free(gpl_scripts[i]);
                gpl_scripts[i] = NULL;
            }
        }
        free(gpl_scripts);
    }

    dsl_state_cleanup();
}
