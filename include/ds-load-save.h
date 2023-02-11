// Functions for loading and saving characters and games.
#ifndef DS_LOAD_SAVE_H
#define DS_LOAD_SAVE_H

#include "status.h"

extern sol_status_t sol_ls_save_to_file(const char *path, char *save_name);
extern sol_status_t sol_load_character_charsave(const int slot, const int res_id);

#endif
