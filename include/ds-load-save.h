// Functions for loading and saving characters and games.
#ifndef DS_LOAD_SAVE_H
#define DS_LOAD_SAVE_H

//char* ls_create_save_file(char *name);
void ls_save_to_file(const char *path, char *save_name);
int ls_load_save_file(const char *path);
int ds_load_character_charsave(const int slot, const int res_id);

#endif
