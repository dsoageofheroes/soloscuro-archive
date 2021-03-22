// helper to include lua for the various compilers...
#ifdef _WIN32
#include "win64/lua5.3/lua.h"
#include "win64/lua5.3/lualib.h"
#include "win64/lua5.3/lauxlib.h"
#else
#include <lua5.3/lua.h>
#include <lua5.3/lualib.h>
#include <lua5.3/lauxlib.h>
#endif
