#ifndef GPL_LUA_H
#define GPL_LUA_H

#include <stdlib.h>
#include <stdint.h>
#include "lua-inc.h"

#define LUA_MAX_SIZE (1<<18)
#define MAX_NUM_FUNCTIONS (128)
#define function_map_t uint16_t[MAX_NUM_FUNCTIONS];

//TODO: consolidate the different lua_States!
extern lua_State     *lua_state;
extern char*      gpl_lua_print(const size_t script_id, const int is_mas, size_t *script_len);

#endif
