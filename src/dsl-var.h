#ifndef DSL_VAR_H
#define DSL_VAR_H
#include <stdint.h>

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
#define DSL_GFLAGVAR_SIZE (((MAXGFLAGS*sizeof(uint8_t))/8)+1)
//extern uint8_t gGflagvar[DSL_GFLAGVAR_SIZE];
extern int32_t gBignum;
extern int32_t *gBignumptr;

void dsl_init_vars();

void load_variable();
void read_simple_num_var();

void set_data_ptr(unsigned char *start, unsigned char *cpos);
unsigned char* get_data_start_ptr();

void set_accumulator(int32_t a);
int32_t get_accumulator();

#endif
