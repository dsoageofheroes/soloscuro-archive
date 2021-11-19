#ifndef REGION_MANAGER_H
#define REGION_MANAGER_H

#include "region.h"

void          region_manager_init();
void          region_manager_cleanup();
sol_region_t* region_manager_get_region(const int region_id);
extern void   region_manager_set_current(sol_region_t *region);
extern int    region_manager_add_region(sol_region_t *region);
sol_region_t* region_manager_get_current();
extern void   region_manager_remove_players();

void         ds_region_load_region_from_save(const int id, const int region_id);

#endif
