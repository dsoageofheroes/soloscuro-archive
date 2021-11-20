#ifndef GPL_MANAGER_H
#define GPL_MANAGER_H

#include <stdlib.h>
#include <stdint.h>

void    gpl_manager_init();
void    gpl_manager_cleanup();
uint8_t gpl_lua_execute_script(size_t file, size_t addr, uint8_t is_mas);
void    gpl_execute_string(const char *str);
void    gpl_lua_load_all_scripts();

#endif
