// helper to include lua for the various compilers...
#ifdef _WIN32
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#else
#include <lua5.3/lua.h>
#include <lua5.3/lualib.h>
#include <lua5.3/lauxlib.h>
#endif
