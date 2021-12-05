#include <string.h>
#include <stdlib.h>
#include "lua-inc.h"
#include "gpl.h"
#include "port.h"
#include "gpl-manager.h"
#include "gpl-lua.h"
#include "sol-lua-settings.h"
#include "sol-lua-manager.h"
#include "gpl-state.h"
#include "gff.h"
#include "gfftypes.h"

#define BUF_SIZE (1<<12)
#define MAX_STACK (128)

static char** mas_scripts = NULL;
static char** gpl_scripts = NULL;
static size_t mas_max = 0, gpl_max = 0;
static lua_State *clua = NULL;
static lua_State *lua_states[MAX_STACK];
static size_t lua_stack_pos = 0;

static void write_lua(const char *path, const char *lua, const size_t len);
static void gpl_lua_load_scripts();

void gpl_manager_init() {
    gpl_state_init();

    mas_scripts = gpl_scripts = NULL;

    gpl_lua_load_scripts();
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
void gpl_lua_load_script(const uint32_t script_id, const uint8_t is_mas) {
    size_t script_len;
    unsigned char gpl[DSL_MAX];
    char buf[1024];
    char **script = is_mas ? mas_scripts : gpl_scripts;
    char *script_ptr;

    gff_chunk_header_t chunk = gff_find_chunk_header(DSLDATA_GFF_INDEX,
        is_mas ? GFF_MAS : GFF_GPL, script_id);
    if (chunk.length > DSL_MAX) {
        error("DSL chunk size %d is larger than max (%d)\n", chunk.length, DSL_MAX);
    }
    if (!gff_read_chunk(DSLDATA_GFF_INDEX, &chunk, gpl, chunk.length)) { return; }

    debug("Converting %s %d to lua, length = %d\n",
        is_mas ? "MAS" : "GPL",
        script_id, chunk.length);
    //script_ptr = gpl_lua_print(gpl, len, is_master_mas, &script_len);
    script_ptr = gpl_lua_print(script_id, is_mas, &script_len);
    script[script_id] = malloc(sizeof(char) * (script_len + 1)); // (A)
    strncpy(script[script_id], script_ptr, script_len);
    script[script_id][script_len] = '\0'; // Okay because of (A)
    sprintf(buf, "gpl/%d-%s.lua", script_id,
        is_mas ? "mas" : "gpl");
    write_lua(buf, script[script_id], script_len);
}

void gpl_lua_load_all_scripts() {
    for (int i = 0; i < 100; i++) {
        gpl_lua_load_script(i, 0);
        gpl_lua_load_script(i, 1);
    }
}

void gpl_lua_load_scripts() {
    unsigned int *ids;
    size_t i, amt;

    ids = gff_get_id_list(DSLDATA_GFF_INDEX, GFF_MAS);
    amt = gff_get_resource_length(DSLDATA_GFF_INDEX, GFF_MAS);
    printf("Detected " PRI_SIZET " master GPL files.\n", amt);
    for (i = 0; i < amt; i++) { mas_max = mas_max > ids[i] ? mas_max : ids[i]; }
    mas_max++;
    mas_scripts = calloc(1, sizeof(char*) * mas_max);
    free(ids);

    ids = gff_get_id_list(DSLDATA_GFF_INDEX, GFF_GPL);
    amt = gff_get_resource_length(DSLDATA_GFF_INDEX, GFF_GPL);
    printf("Detected " PRI_SIZET " standard GPL files.\n", amt);
    for (i = 0; i < amt; i++) { gpl_max = gpl_max > ids[i] ? gpl_max : ids[i]; }
    gpl_max++;
    gpl_scripts = calloc(1, sizeof(char*) * gpl_max);
    free(ids);
}

static void open_gpl_lua() {
    clua = lua_states[lua_stack_pos++] = luaL_newstate();
    luaL_openlibs(clua);
    gpl_state_register(clua);
}

static void close_gpl_lua() {
    if (lua_stack_pos <= 0) {
        error("Trying to close a gpl lua that DNE!");
        exit(1);
    }

    lua_close(lua_states[--lua_stack_pos]);
    lua_states[lua_stack_pos] = NULL;

    clua = lua_stack_pos > 0 ? lua_states[lua_stack_pos] : NULL;
}

uint8_t gpl_lua_execute_script(size_t file, size_t addr, uint8_t is_mas) {
    char **scripts = is_mas ? mas_scripts : gpl_scripts;
    size_t size = is_mas ? mas_max : gpl_max;
    char func[BUF_SIZE];
    int ret = 1;

    debug("execute_script(" PRI_SIZET ", " PRI_SIZET ", %d)\n", file, addr, is_mas);
    snprintf(func, BUF_SIZE, "func" PRI_SIZET "", addr);

    if (file < 0 || file >= size) { return 0; }

    if (scripts[file] == NULL) {
        gpl_lua_load_script(file, is_mas);
    }

    if (scripts[file] == NULL) { return 0; }

    open_gpl_lua();
    if (luaL_dostring(clua, scripts[file])) {
        error("Error: unable to load %s script " PRI_SIZET ":" PRI_SIZET "\n",
            is_mas ? "MAS" : "GPL",
            file, addr);
        error("error: %s\n", lua_tostring(clua, -1));
        clua = NULL;
        lua_close(clua);
        return 0;
    }
    lua_getglobal(clua, func);
    if (lua_pcall(clua, 0, 0, 0)) {
        error("error running function: '%s' in %s file " PRI_SIZET ", addr " PRI_SIZET "\n", func,
            is_mas ? "MAS" : "GPL",
            file, addr);
        error("error: %s\n", lua_tostring(clua, -1));
        ret = 0;
    }

    debug("exiting execute_script(" PRI_SIZET ", " PRI_SIZET ", %d)\n", file, addr, is_mas);
    close_gpl_lua();

    return ret;
}

extern void gpl_execute_string(const char *str) {
    if (!clua) {
        error("trying to call internal gpl execute string, but no context!\n");
        exit(1);
    }

    if (luaL_dostring(clua, str)) {
        error("Unable to execute '%s'!\n", str);
        error("%s\n", lua_tostring(clua, -1));
    }
}

void gpl_manager_cleanup() {
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
    mas_scripts = NULL;

    if (gpl_scripts) {
        for (i = 0; i < gpl_max; i++) {
            if (gpl_scripts[i]) {
                free(gpl_scripts[i]);
                gpl_scripts[i] = NULL;
            }
        }
        free(gpl_scripts);
    }
    gpl_scripts = NULL;

    gpl_state_cleanup();
}
