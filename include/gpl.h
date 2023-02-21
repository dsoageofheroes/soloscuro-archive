/* Game Programming Language (GPL) copyright SSI
 * This is an implementation of the Game Programming Lanugage
 * developed at SSI.
 */

#ifndef GPL_H
#define GPL_H

#ifdef DEBUG
#define  debug(fmt, ...) sol_debug(__FILE__, __LINE__, __PRETTY_FUNCTION__, fmt, ##__VA_ARGS__)
#else
#       define debug(...)
#endif

#define info(fmt, ...) printf((fmt), ##__VA_ARGS__);
#define warn(fmt, ...) printf(("[%s:%d] %s: " fmt), __FILE__, __LINE__, __PRETTY_FUNCTION__, ##__VA_ARGS__);
#define error(fmt, ...) fprintf(stderr, ("[%s:%d] %s: " fmt), __FILE__, __LINE__, __PRETTY_FUNCTION__, ##__VA_ARGS__);
#define fatal(fmt, ...) fprintf(stderr, ("FATAL [%s:%d] %s: " fmt), __FILE__, __LINE__, __PRETTY_FUNCTION__, ##__VA_ARGS__);exit(1);

#include "settings.h"
#include "gff.h"
#include "gfftypes.h"
#include "entity.h"
#include "ssi-scmd.h"

#define gpl_check_index_t uint16_t
#define NULL_CHECK (0xFFFF)

#define MAX_PARAMETERS (8)
typedef struct sol_gpl_param_s {
    int32_t val[MAX_PARAMETERS];
    int32_t *ptr[MAX_PARAMETERS];
} sol_gpl_param_t;

enum sol_gpl_global_e {
    GPL_POV,
    GPL_OTHER
};

extern sol_gpl_param_t param;

extern sol_status_t sol_gpl_init();
extern sol_status_t sol_gpl_change_region(const int region_id);
extern sol_status_t sol_gpl_write_local_state(FILE *file);
extern sol_status_t sol_gpl_write_global_state(FILE *file);

extern sol_status_t sol_gpl_cleanup();
extern sol_status_t sol_gpl_set_global(enum sol_gpl_global_e what, sol_entity_t *entity);

extern sol_status_t sol_gpl_request_impl(int16_t token, int16_t name,
        int32_t num1, int32_t num2);
extern sol_status_t sol_gpl_get_global(enum sol_gpl_global_e what, sol_entity_t **ent);

/* Begin Parsing functions */ 
extern sol_status_t sol_gpl_peek_one_byte(uint8_t *d);
extern sol_status_t sol_gpl_get_byte(uint8_t *d);
/* End Parsing functions */ 

extern sol_status_t gpl_lua_debug();
extern sol_status_t gpl_set_exit();

extern uint8_t command_implemented; // Temporary while I figure out each function.

#endif
