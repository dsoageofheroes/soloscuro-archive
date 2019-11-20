#include "dsl-var.h"
#include "dsl.h"
#include "dsl-string.h"
#include <string.h>

static uint8_t gGflagvar[DSL_GFLAGVAR_SIZE];
static uint8_t datatype;
static uint16_t varnum;
static int32_t accum; //, number;
static unsigned char* dsl_data;
static unsigned char* dsl_data_start;
static int16_t temps16;
static uint8_t bitmask[8] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80};

static void load_simple_variable(uint16_t type, uint16_t vnum, int32_t val);

int32_t gBignum;
int32_t *gBignumptr;

void dsl_init_vars() {
    memset(gGflagvar, 0x00, DSL_GFLAGVAR_SIZE);
}

void set_data_ptr(unsigned char *start, unsigned char *cpos) {
    dsl_data_start = start;
    dsl_data = cpos;
}

void set_accumulator(int32_t a) {
    accum = a;
    printf("accum = %d\n", accum);
}

int32_t get_accumulator() {
    return accum;
}

unsigned char* get_data_start_ptr() {
    return dsl_data_start;
}

uint8_t get_byte() {
    uint8_t answer = (uint8_t) *dsl_data;
    dsl_data++;
    return answer;
}

uint8_t peek_one_byte() {
    return *dsl_data;
}
uint16_t get_half_word() {
    uint16_t ret = get_byte() * 0x100;
    ret += get_byte();
    return ret;
}

static uint8_t preview_byte(uint8_t offset) {
    return *(dsl_data + offset);
}


int32_t read_number() {
    int32_t paren_level = 0;
    int8_t found_operator = 0; // did we find an operation?
    int8_t do_next;
    int16_t opstack[MAX_PARENS];
    int32_t accums[MAX_PARENS];
    int16_t cop, next_op; // current operation
    int32_t cval = 0, tval = 0; // current value, temporary value
    memset(opstack, 0, sizeof(opstack));
    memset(accums, 0, sizeof(accums));
    gBignumptr = &accum;
    do {
        found_operator = 0;
        do_next = 0;
        cop = get_byte(); // current operation
        printf("current operation = 0x%x\n", cop);
        if (cop < 0x80) {
            cval = cop * 0x100 + get_byte();
            printf("cval = %d\n", cval);
        } else {
            if (cop < OPERATOR_OFFSET) {
                if (cop & EXTENDED_VAR) { // variable is > 255
                    cop -= EXTENDED_VAR;
                    gBignum = EXTENDED_VAR;
                } else {
                    gBignum = 0;
                }
            }
            switch (cop) {
                case DSL_ACCM|0x80: {
                    cval = accum;
                    break;
                }
                case DSL_LNAME|0x80:
                case DSL_GNAME|0x80:
                case DSL_LNUM|0x80:
                case DSL_LBYTE|0x80:
                case DSL_LBIGNUM|0x80:
                case DSL_GNUM|0x80:
                case DSL_GBYTE|0x80:
                case DSL_GBIGNUM|0x80:
                case DSL_GFLAG|0x80:
                case DSL_LFLAG|0x80:
                case DSL_GSTRING|0x80:
                case DSL_LSTRING|0x80: {
                    gBignum += cop & 0x7F;
                    read_simple_num_var();
                    cval = gBignum;
                    break;
                }
                case DSL_RETVAL|0x80: {
                    printf("DSL_RETVAL Not implemented\n");
                    command_implemented = 0;
                    break;
                }
                //case DSL_HI_RETVAL|0x80:
                case DSL_IMMED_BIGNUM|0x80: {
                    printf("IMMED_BIGNUM not implemented!\n");
                    command_implemented = 0;
                    break;
                }
                case DSL_IMMED_BYTE|0x80: {
                    cval = (int32_t)((int8_t)get_byte());
                    break;
                }
                case DSL_IMMED_WORD|0x80: {
                    printf("IMMED_WORD not implemented!\n");
                    command_implemented = 0;
                    break;
                }
                case DSL_IMMED_NAME|0x80: {
                    cval = (int32_t)((int16_t)get_half_word() * -1);
                    break;
                }
                case DSL_COMPLEX_VAL|0x80: {
                    printf("COMPLEX_VAL not implemented!\n");
                    command_implemented = 0;
                    break;
                }
                case DSL_IMMED_STRING|0x80: {
                    cval = 0;
                    gBignumptr = (int32_t*) read_text();
                    break;
                }
                case DSL_OP_ADD:
                case DSL_OP_MINUS:
                case DSL_OP_TIMES:
                case DSL_OP_DIVIDE:
                case DSL_OP_AND:
                case DSL_OP_OR:
                case DSL_OP_EQUAL:
                case DSL_OP_NEQUAL:
                case DSL_OP_GTR:
                case DSL_OP_LESS:
                case DSL_OP_GTE:
                case DSL_OP_LTE:
                case DSL_OP_BAND:
                case DSL_OP_BOR:
                case DSL_OP_BCLR: {
                    printf("DSL OPERATOR NOT IMPLEMENTED\n");
                    command_implemented = 0;
                    break;
                }
                case DSL_HI_CLOSE_PAREN: {
                    printf("DSL HI CLOSE PAREN ERROR!\n");
                    command_implemented = 0;
                    break;
                }
                case DSL_HI_OPEN_PAREN: {
                    printf("DSL HI OPEN PAREN ERROR!\n");
                    command_implemented = 0;
                    break;
                }
                default: {
                    printf(" UNKNOWN OP: %d\n", cop);
                    command_implemented = 0;
                    break;
                }
            }
        }
        
        if (!found_operator) {
            printf("operator not found, opstack[%d] = %d\n", paren_level, opstack[paren_level]);
            tval = accums[paren_level];
            switch(opstack[paren_level]) {
                case DSL_PLUS:   tval += cval; break;
                case DSL_MINUS:  tval -= cval; break;
                case DSL_MULT:   tval *= cval; break;
                case DSL_DIV:    tval /= cval; break;
                case DSL_AND:    tval = (tval && cval); break;
                case DSL_OR:     tval = (tval || cval); break;
                case DSL_EQUAL:  tval = (tval == cval); break;
                case DSL_NEQUAL: tval = (tval != cval); break;
                case DSL_GT:     tval = (tval > cval); break;
                case DSL_LT:     tval = (tval < cval); break;
                case DSL_BAND:   tval = (tval & cval); break;
                case DSL_BOR:    tval = (tval | cval); break;
                case DSL_CLR:    tval &= ~cval; break;
                default:         tval = cval; break;
            }
            accums[paren_level] = tval;
            opstack[paren_level] = 0;
        }
        //printf("*************Need to put in a loop!*******************\n");
    } while (do_next || 
        (((next_op = preview_byte(0)) > OPERATOR_OFFSET && next_op <= OPERATOR_LAST)
            || (paren_level > 0 && next_op == DSL_HI_CLOSE_PAREN)));
    // We need to look if that above else was executed!
    return accums[0];
}

void load_variable() {
    int8_t extended = 0;
    datatype = get_byte();
    datatype &= 0x7F;
    if ((datatype & EXTENDED_VAR) != 0) {
        extended = 1;
        datatype -= EXTENDED_VAR;
    }
    if (datatype < 0x10) { // simple data type
        printf("Loading a simple variable not implemented!\n");
        varnum = get_byte();
        if (extended == 1) {
            varnum *= 0x100;
            varnum += get_byte();
        }
        load_simple_variable(datatype, varnum, accum);
    } else {
        printf("Loading a complex variable not implemented!\n");
        command_implemented = 0;
    }
}

static void load_simple_variable(uint16_t type, uint16_t vnum, int32_t val) {
    switch (type) {
        case DSL_GBIGNUM:
            printf("load_simple_variable: DSL_GBIGNUM not implemented\n");
            command_implemented = 0;
            break;
        case DSL_LBIGNUM:
            printf("load_simple_variable: DSL_LBIGNUM not implemented\n");
            command_implemented = 0;
            break;
        case DSL_GNUM:
            printf("load_simple_variable: DSL_GNUM not implemented\n");
            command_implemented = 0;
            break;
        case DSL_LNUM:
            printf("load_simple_variable: DSL_LNUM not implemented\n");
            command_implemented = 0;
            break;
        case DSL_GFLAG:
            printf("load_simple_variable: DSL_GFLAG not implemented\n");
            command_implemented = 0;
            break;
    }
    printf("load_simple_variable not implemented!\n");
    command_implemented = 0;
}

void read_simple_num_var() {
    temps16 = get_byte();

    if ((gBignum & EXTENDED_VAR) != 0) {
        temps16 *= 0x100;
        temps16 += get_byte();
        gBignum &= 0x3F;
    }

    switch(gBignum) {
        case DSL_GFLAG: {
            gBignumptr = (int32_t*)((int8_t*)&gGflagvar[temps16/8]);
            gBignum = gGflagvar[temps16/8] & bitmask[temps16%8];
            if (gBignum > 0) { gBignum = 0; }
            break;
        }
        case DSL_LFLAG: {
            printf("read_simple_num_var: Unimplemented LFLAG\n");
            command_implemented = 0;
            break;
        }
        case DSL_GNUM: {
            printf("read_simple_num_var: Unimplemented GNUM\n");
            command_implemented = 0;
            break;
        }
        case DSL_LNUM: {
            printf("read_simple_num_var: Unimplemented LNUM\n");
            command_implemented = 0;
            break;
        }
        case DSL_GBIGNUM: {
            printf("read_simple_num_var: Unimplemented GBIGNUM\n");
            command_implemented = 0;
            break;
        }
        case DSL_LBIGNUM: {
            printf("read_simple_num_var: Unimplemented LBIGNUM\n");
            command_implemented = 0;
            break;
        }
        case DSL_GNAME: {
            printf("read_simple_num_var: Unimplemented GNAME\n");
            command_implemented = 0;
            break;
        }
        case DSL_GSTRING: {
            gBignumptr = (int32_t*) gGstringvar[temps16];
            break;
        }
        case DSL_LSTRING: {
            printf("read_simple_num_var: Unimplemented LSTRING\n");
            command_implemented = 0;
            break;
        }
        default:
            printf("ERROR: Unknown type in read_simple_num_var.\n");
            command_implemented = 0;
            break;
    }
}
