#ifndef UIL_H
#define UIL_H

extern int ui_lua_keyup(const int key_code);
extern int ui_lua_keydown(const int key_code);
extern int ui_lua_load_preload(const char *filename);
extern int ui_lua_load(const char *filename);
extern void ui_lua_close();

#endif
