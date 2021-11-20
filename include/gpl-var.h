#ifndef GPL_VAR_H
#define GPL_VAR_H
#include <stdint.h>
#include "gpl.h"
#include "gpl-lua.h"

#define OPERATOR_OFFSET (0xD0)
#define GPL_OP_ADD      (0xD1)
#define GPL_OP_MINUS    (0xD2)
#define GPL_OP_TIMES    (0xD3)
#define GPL_OP_DIVIDE   (0xD4)
#define GPL_OP_AND      (0xD5)
#define GPL_OP_OR       (0xD6)
#define GPL_OP_EQUAL    (0xD7)
#define GPL_OP_NEQUAL   (0xD8)
#define GPL_OP_GTR      (0xD9)
#define GPL_OP_LESS     (0xDA)
#define GPL_OP_GTE      (0xDB)
#define GPL_OP_LTE      (0xDC)
#define GPL_OP_BAND     (0xDD)
#define GPL_OP_BOR      (0xDE)
#define GPL_OP_BCLR     (0xDF)

#define GPL_CLOSE_PAREN    (0x61)
#define GPL_OPEN_PAREN     (0x62)
#define GPL_HI_CLOSE_PAREN (0xE1)
#define GPL_HI_OPEN_PAREN  (0xE2)

// Data Types
#define GPL_ACCM         (0x0)
#define GPL_LSTRING      (0x1)
#define GPL_LNUM         (0x2)
#define GPL_LBYTE        (0x3)
#define GPL_LNAME        (0x4)
#define GPL_LBIGNUM      (0x5)
#define GPL_GSTRING      (0x6)
#define GPL_GNUM         (0x7)
#define GPL_GBYTE        (0x8)
#define GPL_GNAME        (0x9)
#define GPL_GBIGNUM      (0xA)
#define GPL_IMMED_BIGNUM (0xB)
#define GPL_RETVAL       (0xC)
#define GPL_HI_RETVAL    (0x8C)
#define GPL_GFLAG        (0xD)
#define GPL_LFLAG        (0xE)
#define GPL_IMMED_BYTE   (0xF)
#define GPL_IMMED_WORD   (0x10)
#define GPL_IMMED_NAME   (0x11)
#define GPL_IMMED_STRING (0x12)

#define GPL_COMPLEX_LOW       (0x30)
#define GPL_COMPLEX_PTR       (0x30)
#define GPL_COMPLEX_VAL       (0x31)
#define GPL_COMPLEX_SET       (0x32)
#define GPL_COMPLEX_SHORTHAND (0x33)
#define GPL_COMPLEX_HIGH      (0x3F)

#define GPL_OP_OFFSET (0xD0)
#define GPL_PLUS      (0xD1)
#define GPL_MINUS     (0xD2)
#define GPL_MULT      (0xD3)
#define GPL_DIV       (0xD4)
#define GPL_AND       (0xD5) // Boolean And
#define GPL_OR        (0xD6) // Boolean OR
#define GPL_EQUAL     (0xD7)
#define GPL_NEQUAL    (0xD8)
#define GPL_GT        (0xD9) // Greater Than
#define GPL_LT        (0xDA) // Less Than
#define GPL_GTEQUAL   (0xDB) // Greater Than or equal
#define GPL_LTEQUAL   (0xDC) // Less Than or equal
#define GPL_BAND      (0xDD) // bitwise and
#define GPL_BOR       (0xDE) // bitwise OR
#define GPL_CLR       (0xDF) // &=~
#define OPERATOR_LAST (GPL_CLR)

#define MAX_PARENS (8)
#define EXTENDED_VAR    (0x40)
#define MAXGFLAGS         (800)
#define MAXLFLAGS         (64)
#define MAXGNUMS          (400)
#define MAXGBIGNUMS       (40)
#define MAXLBIGNUMS       (40)
#define MAXLNUMS          (32)

#define MAX_SEARCH_STACK (32)

#define GPL_GFLAGVAR_SIZE (((MAXGFLAGS*sizeof(uint8_t))/8)+1)
#define GPL_LFLAGVAR_SIZE (((MAXLFLAGS*sizeof(uint8_t))/8)+1)
#define GPL_GBIGNUMVAR_SIZE (MAXGBIGNUMS * sizeof(int32_t))
#define GPL_GNUMVAR_SIZE (MAXGNUMS * sizeof(int16_t))
#define GPL_LBIGNUMVAR_SIZE (MAXLBIGNUMS * sizeof(int32_t))
#define GPL_LNUMVAR_SIZE (MAXLNUMS * sizeof(int16_t))

#define GLOBAL_MAS (99)
enum {NOFILE, DSLFILE, MASFILE};

#define MAX_GPL_CHECKS (200)
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

typedef struct gpl_check_s {
    union {
        box_t box_check;
        tile_t tile_check;
        name_t name_check;
        name2_t name2_check;
    } data;
    uint8_t type;
    uint16_t next;
} gpl_check_t;

#define GPL_MAX_CALL (2)
typedef struct gpl_control_s {
    int16_t destx;
    int16_t desty;
    int16_t cmd[GPL_MAX_CALL];
    int16_t addr[GPL_MAX_CALL];
    int16_t file[GPL_MAX_CALL];
    uint8_t flags;
} gpl_control_t;

#define MAX_OBJECT_PATH (1000)

extern int32_t        gpl_global_big_num;
extern int32_t       *gpl_global_big_numptr;
extern uint16_t       gpl_current_file;
extern uint16_t       gpl_current_type;

extern void           gpl_init_vars();
extern void           gpl_cleanup_vars();
extern void           gpl_set_data_ptr(unsigned char *start, unsigned char *cpos);
extern unsigned char* gpl_get_data_start_ptr();
extern unsigned char* gpl_get_data_ptr();
extern void           gpl_push_data_ptr(unsigned char *data);
extern unsigned char* gpl_pop_data_ptr();
extern uint16_t       gpl_peek_half_word();
extern uint8_t        gpl_preview_byte(uint8_t offset);

#endif
