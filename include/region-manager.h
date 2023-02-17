#ifndef REGION_MANAGER_H
#define REGION_MANAGER_H

#include "region.h"

#define MAX_REGIONS (0x100)

extern sol_status_t sol_region_manager_load_etab(sol_region_t *reg);
extern sol_status_t sol_region_manager_init();
extern sol_status_t sol_region_manager_cleanup(int free_entities);
extern sol_status_t sol_region_manager_set_current(sol_region_t *region);
extern sol_status_t sol_region_manager_remove_players();
extern sol_status_t sol_region_manager_get_region(const int region_id, const int assume_loaded, sol_region_t **reg);
extern sol_status_t sol_region_manager_get_region_with_entity(const sol_entity_t *entity, sol_region_t **r);
extern sol_status_t sol_region_manager_add_region(sol_region_t *region);
extern sol_status_t sol_region_manager_get_current(sol_region_t **r);

#endif
