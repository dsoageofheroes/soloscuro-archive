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
#include "ssi-scmd.h"

#define gpl_check_index_t uint16_t
#define NULL_CHECK (0xFFFF)

#define MAX_PARAMETERS (8)
typedef struct gpl_param_s {
    int32_t val[MAX_PARAMETERS];
    int32_t *ptr[MAX_PARAMETERS];
} gpl_param_t;

extern gpl_param_t param;

extern void     gpl_init();
extern void     gpl_execute_function(const int gff_idx, const int res_id, const int file_id);
extern void     gpl_change_region(const int region_id);
//int do_dsl_command(uint8_t cmd);
extern uint32_t gpl_request_impl(int16_t token, int16_t name,
        int32_t num1, int32_t num2);
extern void     gpl_cleanup();
extern void     gpl_set_quiet(const int val);

/* Begin Parsing functions */ 
extern uint8_t  gpl_peek_one_byte();
extern uint8_t  gpl_get_byte();
/* End Parsing functions */ 

extern uint8_t command_implemented; // Temporary while I figure out each function.

#endif
