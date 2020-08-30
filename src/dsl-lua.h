#ifndef DSL_LUA_H
#define DSL_LUA_H

#include <stdlib.h>
#include <stdint.h>

#define LUA_MAX_SIZE (1<<18)
#define BUF_SIZE (1<<12)
#define MAX_NUM_FUNCTIONS (128)
#define function_map_t uint16_t[MAX_NUM_FUNCTIONS];

//char* dsl_lua_print(unsigned char *dsl, const size_t len, const int master_mas, size_t *script_len);
//void dsl_lua_load_script(const uint32_t script_id, const uint8_t is_mas) {
char* dsl_lua_print(const size_t script_id, const int is_mas, size_t *script_len);

#endif
