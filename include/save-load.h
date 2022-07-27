#ifndef SOL_LOAD_SAVE_H
#define SOL_LOAD_SAVE_H

extern int sol_save_to_file(const char *filepath, const char *name);
extern int sol_load_from_file(const char *filepath);
extern int sol_load_get_name(const char *filepath, char *name, const size_t len);

#endif
