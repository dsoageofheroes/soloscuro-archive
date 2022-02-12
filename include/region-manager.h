#ifndef REGION_MANAGER_H
#define REGION_MANAGER_H

#include "region.h"

#define MAX_REGIONS (0xFF)

extern void          sol_region_manager_init();
extern void          sol_region_manager_cleanup();
extern sol_region_t* sol_region_manager_get_region(const int region_id);
extern sol_region_t* sol_region_manager_get_region_with_entity(const entity_t *entity);
extern void          sol_region_manager_set_current(sol_region_t *region);
extern int           sol_region_manager_add_region(sol_region_t *region);
extern sol_region_t* sol_region_manager_get_current();
extern void          sol_region_manager_remove_players();

#endif
