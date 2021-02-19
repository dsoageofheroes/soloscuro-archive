// DarkSun Scripting Language (DSL)
#ifndef DSL_H
#define DSL_H

void dsl_debug(const char *file, const int line_num, const char *pretty, const char *str, ...);

#ifdef DEBUG
#define  debug(fmt, ...) dsl_debug(__FILE__, __LINE__, __PRETTY_FUNCTION__, fmt, ##__VA_ARGS__)
#else
#       define debug(...)
#endif

#define info(fmt, ...) printf((fmt), ##__VA_ARGS__);
#define warn(fmt, ...) printf(("[%s:%d] %s: " fmt), __FILE__, __LINE__, __PRETTY_FUNCTION__, ##__VA_ARGS__);
#define error(fmt, ...) fprintf(stderr, ("[%s:%d] %s: " fmt), __FILE__, __LINE__, __PRETTY_FUNCTION__, ##__VA_ARGS__);
#define fatal(fmt, ...) fprintf(stderr, ("FATAL [%s:%d] %s: " fmt), __FILE__, __LINE__, __PRETTY_FUNCTION__, ##__VA_ARGS__);exit(1);

#include "gff.h"
#include "gfftypes.h"
#include "ds-scmd.h"

#define check_index_t uint16_t
#define NULL_CHECK (0xFFFF)

#define MAX_PARAMETERS (8)
typedef struct _param_t {
    int32_t val[MAX_PARAMETERS];
    int32_t *ptr[MAX_PARAMETERS];
} param_t;

extern param_t param;

void dsl_init();
void dsl_execute_function(const int gff_idx, const int res_id, const int file_id);
void dsl_change_region(const int region_id);
int do_dsl_command(uint8_t cmd);
uint32_t dsl_request_impl(int16_t token, int16_t name,
        int32_t num1, int32_t num2);
void dsl_cleanup();
void dsl_set_quiet(const int val);


/* Parsing functions */
uint8_t peek_one_byte();
uint8_t get_byte();
/* End Parsing functions */ 

extern uint8_t command_implemented; // Temporary while I figure out each function.

#endif
