#ifndef GPL_STATE_H
#define GPL_STATE_H

#include "lua-inc.h"
#include <stdint.h>

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

void     gpl_state_init();
void     gpl_state_cleanup();
void     gpl_state_register(lua_State *l);

uint32_t gpl_get_region();
uint32_t gpl_get_gff_index();

void     gpl_set_gname(const int index, const int32_t obj);
void     gpl_local_clear();

char*    gpl_serialize_globals(uint32_t *len);
void     gpl_deserialize_globals(char *buf);
char*    gpl_serialize_locals(uint32_t *len);
void     gpl_deserialize_locals(char *buf);

#endif
