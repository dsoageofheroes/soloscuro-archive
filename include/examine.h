#ifndef EXAMINE_H
#define EXAMINE_H

#include "window-manager.h"
#include "entity.h"
#include "status.h"

extern sol_status_t sol_examine_entity(sol_entity_t *dude);
extern sol_status_t sol_examine_is_open();

extern sol_wops_t examine_window;

#endif
