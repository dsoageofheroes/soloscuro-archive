#ifndef DESCRIPTION_H
#define DESCRIPTION_H

#include "window-manager.h"
#include "status.h"

extern sol_status_t sol_description_set_message(const char *msg);
extern sol_status_t sol_description_set_icon(const uint16_t icon);

extern sol_wops_t description_window;

#endif
