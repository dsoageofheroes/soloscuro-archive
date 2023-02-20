#ifndef SOL_LOAD_SAVE_H
#define SOL_LOAD_SAVE_H

#include "status.h"

extern sol_status_t sol_save_to_file(const char *filepath, const char *name);
extern sol_status_t sol_load_from_file(const char *filepath);
extern sol_status_t sol_load_get_name(const char *filepath, char *name, const size_t len);

#endif
