#ifndef GPL_MANAGER_H
#define GPL_MANAGER_H

#include <stdlib.h>
#include <stdint.h>

extern void    gpl_manager_init();
extern void    gpl_manager_cleanup();
extern uint8_t gpl_lua_execute_script(size_t file, size_t addr, uint8_t is_mas);
extern void    gpl_execute_string(const char *str);
extern void    gpl_lua_load_all_scripts();
extern void    gpl_lua_debug();
extern void    gpl_push_context();
extern void    gpl_pop_context();
extern void    gpl_set_exit();
extern int     gpl_in_exit();

#endif
