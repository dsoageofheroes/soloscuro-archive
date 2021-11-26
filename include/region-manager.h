#ifndef REGION_MANAGER_H
#define REGION_MANAGER_H

#include "region.h"

void          sol_region_manager_init();
void          sol_region_manager_cleanup();
sol_region_t* sol_region_manager_get_region(const int region_id);
extern void   sol_region_manager_set_current(sol_region_t *region);
extern int    sol_region_manager_add_region(sol_region_t *region);
sol_region_t* sol_region_manager_get_current();
extern void   sol_region_manager_remove_players();

#endif
