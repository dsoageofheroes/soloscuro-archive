#ifndef ANNOTATIONS_H
#define ANNOTATIONS_H

#include "region.h"

extern void sol_annotations_init();
extern void sol_annotations_cleanup();

extern int sol_annotations_call_func(const uint32_t file, const uint32_t func, int obj);

#endif
