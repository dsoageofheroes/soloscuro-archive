#ifndef REGION_MANAGER_H
#define REGION_MANAGER_H

#include "region.h"

void region_manager_init();
void region_manager_cleanup();
region_t* region_manager_get_region(const int region_id);
void ds_region_load_region_from_save(const int id, const int region_id);
ds_region_t* ds_region_load_region(const int region_id);
ds_region_t* ds_region_get_region(const int region_id);

#endif
