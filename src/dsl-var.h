#ifndef DSL_VAR_H
#define DSL_VAR_H
#include <stdint.h>
#include <lua5.1/lua.h>
#include "dsl.h"

#define OPERATOR_OFFSET (0xD0)
#define DSL_OP_ADD      (0xD1)
#define DSL_OP_MINUS    (0xD2)
#define DSL_OP_TIMES    (0xD3)
#define DSL_OP_DIVIDE   (0xD4)
#define DSL_OP_AND      (0xD5)
#define DSL_OP_OR       (0xD6)
#define DSL_OP_EQUAL    (0xD7)
#define DSL_OP_NEQUAL   (0xD8)
#define DSL_OP_GTR      (0xD9)
#define DSL_OP_LESS     (0xDA)
#define DSL_OP_GTE      (0xDB)
#define DSL_OP_LTE      (0xDC)
#define DSL_OP_BAND     (0xDD)
#define DSL_OP_BOR      (0xDE)
#define DSL_OP_BCLR     (0xDF)

#define DSL_CLOSE_PAREN    (0x61)
#define DSL_OPEN_PAREN     (0x62)
#define DSL_HI_CLOSE_PAREN (0xE1)
#define DSL_HI_OPEN_PAREN  (0xE2)

// Data Types
#define DSL_ACCM         (0x0)
#define DSL_LSTRING      (0x1)
#define DSL_LNUM         (0x2)
#define DSL_LBYTE        (0x3)
#define DSL_LNAME        (0x4)
#define DSL_LBIGNUM      (0x5)
#define DSL_GSTRING      (0x6)
#define DSL_GNUM         (0x7)
#define DSL_GBYTE        (0x8)
#define DSL_GNAME        (0x9)
#define DSL_GBIGNUM      (0xA)
#define DSL_IMMED_BIGNUM (0xB)
#define DSL_RETVAL       (0xC)
#define DSL_HI_RETVAL    (0x8C)
#define DSL_GFLAG        (0xD)
#define DSL_LFLAG        (0xE)
#define DSL_IMMED_BYTE   (0xF)
#define DSL_IMMED_WORD   (0x10)
#define DSL_IMMED_NAME   (0x11)
#define DSL_IMMED_STRING (0x12)

#define DSL_COMPLEX_LOW       (0x30)
#define DSL_COMPLEX_PTR       (0x30)
#define DSL_COMPLEX_VAL       (0x31)
#define DSL_COMPLEX_SET       (0x32)
#define DSL_COMPLEX_SHORTHAND (0x33)
#define DSL_COMPLEX_HIGH      (0x3F)

#define DSL_OP_OFFSET (0xD0)
#define DSL_PLUS      (0xD1)
#define DSL_MINUS     (0xD2)
#define DSL_MULT      (0xD3)
#define DSL_DIV       (0xD4)
#define DSL_AND       (0xD5) // Boolean And
#define DSL_OR        (0xD6) // Boolean OR
#define DSL_EQUAL     (0xD7)
#define DSL_NEQUAL    (0xD8)
#define DSL_GT        (0xD9) // Greater Than
#define DSL_LT        (0xDA) // Less Than
#define DSL_GTEQUAL   (0xDB) // Greater Than or equal
#define DSL_LTEQUAL   (0xDC) // Less Than or equal
#define DSL_BAND      (0xDD) // bitwise and
#define DSL_BOR       (0xDE) // bitwise OR
#define DSL_CLR       (0xDF) // &=~
#define OPERATOR_LAST (DSL_CLR)

#define MAX_PARENS (8)
#define EXTENDED_VAR    (0x40)
#define MAXGFLAGS         (800)
#define MAXLFLAGS         (64)
#define MAXGNUMS          (400)
#define MAXGBIGNUMS       (40)
#define MAXLBIGNUMS       (40)
#define MAXLNUMS          (32)

#define MAX_SEARCH_STACK (32)

#define DSL_GFLAGVAR_SIZE (((MAXGFLAGS*sizeof(uint8_t))/8)+1)
#define DSL_LFLAGVAR_SIZE (((MAXLFLAGS*sizeof(uint8_t))/8)+1)
#define DSL_GBIGNUMVAR_SIZE (MAXGBIGNUMS * sizeof(int32_t))
#define DSL_GNUMVAR_SIZE (MAXGNUMS * sizeof(int16_t))
#define DSL_LBIGNUMVAR_SIZE (MAXLBIGNUMS * sizeof(int32_t))
#define DSL_LNUMVAR_SIZE (MAXLNUMS * sizeof(int16_t))

#define GLOBAL_MAS (99)
enum {NOFILE, DSLFILE, MASFILE};

#define MAX_DSL_CHECKS (200)
enum {UNUSED_CHECK_INDEX,
    TALK_TO_CHECK_INDEX,
    ATTACK_CHECK_INDEX,
    POV_CHECK_INDEX,
    PICKUP_CHECK_INDEX,
    OTHER_CHECK_INDEX,
    OTHER1_CHECK_INDEX,
    MOVE_TILE_CHECK_INDEX,
    LOOK_CHECK_INDEX,
    USE_CHECK_INDEX,
    USE_WITH_CHECK_INDEX,
    MOVE_BOX_CHECK_INDEX,
    MAX_CHECK_TYPES
};

typedef struct _name_s {
    uint16_t addr; // Address of DSL routine
    uint16_t file; // file # holding DSL routine
    int16_t name; // name header ? negative vale mean name/id
    uint8_t global; // is global?  If not erase on region change!
} name_t;

typedef struct _name2_s {
    uint16_t addr; // Address of DSL routine
    uint16_t file; // file # holding DSL routine
    int16_t name1; // name #1
    int16_t name2; // name #2
    uint8_t global; // is global?  if not erase on region change!
} name2_t;


typedef struct box_s {
    uint16_t addr; // Addr of DSL routine in file
    uint16_t file; // the file
    uint16_t x;    // x coordinate
    uint16_t y;    // y coordinate
    uint8_t xd;    // x dimention (width/height)
    uint8_t yd;    // y dimention (width/height)
    uint8_t trip;  // Is this a PC only, or can anyone trip this event?
} box_t;

typedef struct tile_s {
    uint16_t addr; // addr of DSL rout in file
    uint16_t file; // the file
    uint16_t x;    // x coordinate of the tile
    uint16_t y;    // y coordinate of the tile
    uint8_t trip;  // Is this PC only, or can anyone trip the event?
} tile_t;

typedef struct dsl_check_s {
    union {
        box_t box_check;
        tile_t tile_check;
        name_t name_check;
        name2_t name2_check;
    } data;
    uint8_t type;
    uint16_t next;
} dsl_check_t;

#define DSL_MAX_CALL (2)
typedef struct dsl_control_s {
    int16_t destx;
    int16_t desty;
    int16_t cmd[DSL_MAX_CALL];
    int16_t addr[DSL_MAX_CALL];
    int16_t file[DSL_MAX_CALL];
    uint8_t flags;
} dsl_control_t;

#define MAX_OBJECT_PATH (1000)

extern int32_t gBignum;
extern int32_t *gBignumptr;
extern uint16_t this_gpl_file;
extern uint16_t this_gpl_type;
extern lua_State *lua_state;

void dsl_init_vars();

void load_variable();
void read_simple_num_var();

void set_data_ptr(unsigned char *start, unsigned char *cpos);
unsigned char* get_data_start_ptr();
unsigned char* get_data_ptr();
void push_data_ptr(unsigned char *data);
unsigned char* pop_data_ptr();
void clear_local_vars();
uint16_t peek_half_word();
void global_addr_name(param_t *par);
void generic_name_check(int check_index);
void generic_tile_check(int check_index, tile_t tile);
void generic_box_check(int check_index, box_t box);
void name_name_global_addr(param_t *par);
void set_any_order(name_t *name, int16_t to, int16_t los_order, int16_t range);
void use_with_check();
void set_new_order();
void set_los_order(int16_t los_order, int16_t range);
int32_t read_number();

void set_accumulator(int32_t a);
int32_t get_accumulator();

void setrecord();
dsl_check_t* dsl_find_check(int32_t type, int32_t id);
dsl_check_t* dsl_find_tile_check(const int32_t x, const int32_t y);
dsl_check_t* dsl_find_box_check(const int32_t x, const int32_t y);

void print_vars(int what);
void print_all_checks();

#endif
