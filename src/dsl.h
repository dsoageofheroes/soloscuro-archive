// DarkSun Scripting Language (DSL)
#ifndef DSL_H
#define DSL_H

#ifdef DEBUG
#       define debug(fmt, ...) printf(("[%s:%d] %s: " fmt), __FILE__, __LINE__, __PRETTY_FUNCTION__, ##__VA_ARGS__);
#else
#       define debug(...)
#endif

#define info(fmt, ...) printf((fmt), ##__VA_ARGS__);
#define warn(fmt, ...) printf(("[%s:%d] %s: " fmt), __FILE__, __LINE__, __PRETTY_FUNCTION__, ##__VA_ARGS__);
#define error(fmt, ...) fprintf(stderr, ("[%s:%d] %s: " fmt), __FILE__, __LINE__, __PRETTY_FUNCTION__, ##__VA_ARGS__);

#include "gff.h"
#include "gfftypes.h"
#include "dsl-scmd.h"

typedef struct _name_t {
    uint16_t addr; // Address of DSL routine
    uint16_t file; // file # holding DSL routine
    uint16_t name; // name header ? negative vale mean name/id
    uint8_t global; // is global?  If not erase on region change!
} name_t;

typedef struct _name2_t {
    uint16_t addr; // Address of DSL routine
    uint16_t file; // file # holding DSL routine
    int16_t name1; // name #1
    int16_t name2; // name #2
    uint8_t global; // is global?  if not erase on region change!
} name2_t;

#define check_index_t uint16_t
#define NULL_CHECK (0xFFFF)

#define MAX_PARAMETERS (8)
typedef struct _param_t {
    int32_t val[MAX_PARAMETERS];
    int32_t *ptr[MAX_PARAMETERS];
} param_t;

extern param_t param;

void dsl_init();
void dsl_scmd_print(int gff_file, int res_id);
scmd_t* dsl_scmd_get(const int gff_file, const int res_id, const int index);
int dsl_scmd_is_default(const scmd_t *scmd, const int scmd_index);

void dsl_execute_function(const int gff_idx, const int res_id, const int file_id);
void dsl_change_region(const int region_id);

int do_dsl_command(uint8_t cmd);
uint32_t dsl_request_impl(int16_t token, int16_t name,
        int32_t num1, int32_t num2);

/* Parsing functions */
uint8_t peek_one_byte();
uint8_t get_byte();
/* End Parsing functions */ 

extern uint8_t command_implemented; // Temporary while I figure out each function.
int32_t data_field(int16_t header, uint16_t fi);

#define MAX_OBJECTS        (1500)
#define MAX_OBJECT_ENTRIES (2000)
#define MAX_OBJECT_PATH    (1000)
#endif
