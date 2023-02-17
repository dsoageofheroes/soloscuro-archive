#ifndef SOL_PORTAIT_H
#define SOL_PORTAIT_H

#include <stdint.h>
#include "status.h"

extern sol_status_t sol_portrait_load();
extern sol_status_t sol_portrait_display(const uint32_t id, const uint32_t x, const uint32_t y);

#endif
