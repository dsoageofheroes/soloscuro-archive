#ifndef GPL_MANAGER_H
#define GPL_MANAGER_H

#include <stdlib.h>
#include <stdint.h>

#include "status.h"

extern sol_status_t sol_gpl_manager_init();
extern sol_status_t sol_gpl_manager_cleanup();
extern sol_status_t sol_gpl_lua_execute_script(size_t file, size_t addr, uint8_t is_mas);
extern sol_status_t sol_gpl_execute_string(const char *str);
extern sol_status_t sol_gpl_lua_load_all_scripts();
extern sol_status_t sol_gpl_lua_debug();
extern sol_status_t sol_gpl_push_context();
extern sol_status_t sol_gpl_pop_context();
extern sol_status_t sol_gpl_set_exit();
extern sol_status_t sol_gpl_in_exit();

#endif
