// Functions for loading and saving characters and games.
#ifndef DS_LOAD_SAVE_H
#define DS_LOAD_SAVE_H

extern void ls_save_to_file(const char *path, char *save_name);
extern int  ls_load_save_file(const char *path);
extern int  ds_load_character_charsave(const int slot, const int res_id);
extern void ds_region_load_region_from_save(const int id, const int region_id);

#endif
