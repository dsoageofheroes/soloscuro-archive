#ifndef DESCRIPTION_H
#define DESCRIPTION_H

#include "window-manager.h"

extern void sol_description_set_message(const char *msg);
extern void sol_description_set_icon(const uint16_t icon);

extern sol_wops_t description_window;

#endif
