#ifndef DSL_MANAGER_H
#define DSL_MANAGER_H

#include <stdlib.h>
#include <stdint.h>

void dsl_manager_init();
void gpl_manager_cleanup();
uint8_t dsl_lua_execute_script(size_t file, size_t addr, uint8_t is_mas);
void gpl_execute_string(const char *str);
void dsl_lua_load_all_scripts();

#endif
