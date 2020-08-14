#ifndef DSL_LUA_H
#define DSL_LUA_H

#include <stdlib.h>
#include <stdint.h>

void dsl_lua_load_scripts();
char* dsl_lua_print(unsigned char *dsl, const size_t len);

#endif
