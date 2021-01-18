/*
 * These are the functions needed to be implmeneted for porting to different systems.
 * This include both OS specific call and callbacks.
 */
#ifndef PORT_H
#define PORT_H

#include "dsl-object.h"

// Narrate functions
extern int8_t port_narrate_open(int16_t action, const char *text, int16_t index);
extern void port_narrate_clear();
extern void port_narrate_close();

// Object manipulation
extern void port_swap_objs(int obj_id, region_object_t *obj);
extern void port_add_obj(region_object_t *obj);

#endif
