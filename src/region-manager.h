#ifndef REGION_MANAGER_H
#define REGION_MANAGER_H

#include "region.h"

void region_manager_init();
void region_manager_cleanup();
region_t* region_manager_get_region(const int region_id);
extern void region_manager_set_current(region_t *region);
extern int region_manager_add_region(region_t *region);
region_t* region_manager_get_current();

void ds_region_load_region_from_save(const int id, const int region_id);

#endif
