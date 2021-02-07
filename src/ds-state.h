#ifndef DS_STATE_H
#define DS_STATE_H

#include <lua5.3/lua.h>

enum {
    GNAME_UNKNOWN0,
    GNAME_UNKNOWN1,
    GNAME_UNKNOWN2,
    GNAME_UNKNOWN3,
    GNAME_UNKNOWN4,
    GNAME_UNKNOWN5,
    GNAME_UNKNOWN6,
    GNAME_PASSIVE,
    GNAME_UNKNOWN8,
    GNAME_UNKNOWN9,
    GNAME_UNKNOWNA,
    GNAME_UNKNOWNB,
    GNAME_UNKNOWNC
};

void dsl_state_init();
void dsl_state_cleanup();
void dsl_state_register(lua_State *l);

void dsl_set_region(const uint32_t region);
uint32_t dsl_get_region();
uint32_t dsl_get_gff_index();

void dsl_set_gname(const int index, const int32_t obj);
void dsl_local_clear();

#endif
