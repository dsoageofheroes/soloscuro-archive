/*
 * Contains special functions for object (items/npcs/maps) whose
 * functional was buil inside the engine and not in a GFF.
 */
#ifndef SOL_INNATE_H
#define SOL_INNATE_H

#include "status.h"
#include "entity.h"

extern sol_status_t sol_innate_action(dude_t *dude);
extern int sol_innate_is_door(dude_t *dude);
extern int sol_innate_is_door2(dude_t *dude);

#endif
