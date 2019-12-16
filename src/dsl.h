// DarkSun Scripting Language (DSL)
#ifndef DSL_H
#define DSL_H

#include "gff.h"
#include "gfftypes.h"
#include "dsl-scmd.h"

typedef struct _box_t {
    uint16_t addr; // Addr of DSL routine in file
    uint16_t file; // the file
    uint16_t x;    // x coordinate
    uint16_t y;    // y coordinate
    uint8_t xd;    // x dimention (width/height)
    uint8_t yd;    // y dimention (width/height)
    uint8_t trip;  // Is this a PC only, or can anyone trip this event?
} box_t;

typedef struct _tile_t {
    uint16_t addr; // addr of DSL rout in file
    uint16_t file; // the file
    uint16_t x;    // x coordinate of the tile
    uint16_t y;    // y coordinate of the tile
    uint8_t trip;  // Is this PC only, or can anyone trip the event?
} tile_t;

#define MAX_PARAMETERS (8)
typedef struct _param_t {
    int32_t val[MAX_PARAMETERS];
    int32_t *ptr[MAX_PARAMETERS];
} param_t;

extern param_t param;

void dsl_init();
void dsl_check_for_updates();
void dsl_scmd_print(int gff_file, int res_id);
scmd_t* dsl_scmd_get(const int gff_file, const int res_id, const int index);
int dsl_scmd_is_default(const scmd_t *scmd, const int scmd_index);

void mas_print(const int gff_file, const int res_id);
void dsl_print(const int gff_file, const int res_id);

void do_dsl_command(uint8_t cmd);
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
