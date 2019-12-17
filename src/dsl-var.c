#include "dsl-var.h"
#include "dsl.h"
#include "dsl-string.h"
#include <string.h>

static uint8_t gGflagvar[DSL_GFLAGVAR_SIZE];
static uint8_t gLflagvar[DSL_LFLAGVAR_SIZE];
static uint8_t datatype;
static uint16_t varnum;
static int32_t accum; //, number;
static unsigned char* dsl_data;
static unsigned char* dsl_data_start;
static int16_t temps16;
static uint8_t bitmask[8] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80};
static uint8_t bytemask[8] = {0xFE, 0xFD, 0xFB, 0xF7, 0xEF, 0xDF, 0xBF, 0x7F};

static void load_simple_variable(uint16_t type, uint16_t vnum, int32_t val);
static int32_t read_complex(void);

typedef struct _dsl_state_t {
    unsigned char *dsl_data_start;
    unsigned char *dsl_data;
} dsl_state_t;

int32_t gBignum;
int32_t *gBignumptr;
int16_t *gGnumvar = 0;
int16_t *gLnumvar = 0;
int32_t *gGbignumvar = 0;
int32_t *gLbignumvar = 0;
#define MAX_DSL_STATES     (100)
static int dsl_state_pos = -1;
static dsl_state_t states[MAX_DSL_STATES];

static int16_t gFight = 0;  // We in a fight?
static int16_t gplX = 0;    // Current X
static int16_t gplY = 0;    // Current Y
static int16_t gplZ = 0;    // Current Z
static int16_t gRegion = 0; // Current Region
static int16_t gPov = 0;
static int16_t gActive = 0;
static int16_t gPassive = 0;
static int16_t gOther = 0;
static int16_t gOther1 = 0;
static int16_t gThing = 0;
static int32_t gTime = 0;
static int32_t gPartyMoney = 0;

static int16_t* gSimpleVar[] = {
    &gFight,    // 0x20
    &gplX,      // 0x21
    &gplY,      // 0x22
    &gplZ,      // 0x23
    &gRegion,   // 0x24
    &gPov,      // 0x25
    &gActive,   // 0x26
    &gPassive,  // 0x27
    &gOther,    // 0x28
    (int16_t *) &gTime,  // 0x29
    (int16_t *) &gPartyMoney,  // 0x2a
    &gThing,   // 0x2b
    &gOther1   // 0x2c
};

#define MAX_PARAM_DEPTH (2)
int16_t param_depth = 0; // How many commands we can store.
param_t old_params[MAX_PARAM_DEPTH];

static void push_params() {
    if (param_depth < MAX_PARAM_DEPTH) {
        memcpy(&old_params[param_depth], &param, sizeof(param));
        param_depth++;
    } else {
        fprintf(stderr, "push_param ERROR: to many params!\n");
        exit(1);
    }
}

static void pop_params() {
    if (param_depth > 0) {
        param_depth--;
        memcpy(&param, &old_params[param_depth], sizeof(param));
    } else {
        fprintf(stderr, "pop_params ERROR: tried to pop empty stack of params!\n");
    }
}

void dsl_init_vars() {
    memset(gGflagvar, 0x00, DSL_GFLAGVAR_SIZE);
    memset(gLflagvar, 0x00, DSL_LFLAGVAR_SIZE);
    gGbignumvar = malloc(DSL_GBIGNUMVAR_SIZE * sizeof(int32_t));
    memset(gGbignumvar, 0x00, DSL_GBIGNUMVAR_SIZE * sizeof(int32_t));
    gGnumvar = malloc(DSL_GNUMVAR_SIZE);
    memset(gGnumvar, 0x00, DSL_GNUMVAR_SIZE);
    gLbignumvar = malloc(DSL_LBIGNUMVAR_SIZE * sizeof(int32_t));
    memset(gLbignumvar, 0x00, DSL_LBIGNUMVAR_SIZE * sizeof(int32_t));
    gLnumvar = malloc(DSL_LNUMVAR_SIZE);
    memset(gLnumvar, 0x00, DSL_LNUMVAR_SIZE);
}

void set_data_ptr(unsigned char *start, unsigned char *cpos) {
    dsl_data_start = start;
    dsl_data = cpos;
}

void set_accumulator(int32_t a) {
    accum = a;
    //printf("accum = %d\n", accum);
}

int32_t get_accumulator() {
    return accum;
}

unsigned char* get_data_start_ptr() {
    return dsl_data_start;
}

unsigned char* get_data_ptr() {
    return dsl_data;
}

void push_data_ptr(unsigned char *data) {
    // The first one isn't pushed on the stack.
    if (dsl_state_pos < 0) {
        dsl_state_pos = 0;
        return;
    }

    states[dsl_state_pos].dsl_data_start = dsl_data_start;
    states[dsl_state_pos].dsl_data = dsl_data;

    dsl_state_pos++;
}

void clear_local_vars() {
    memset(gGflagvar, 0x0, DSL_GFLAGVAR_SIZE);
}

unsigned char* pop_data_ptr() {
    dsl_state_pos--;

    if (dsl_state_pos < 0) {
        return NULL;
    }

    dsl_data_start = states[dsl_state_pos].dsl_data_start;
    dsl_data = states[dsl_state_pos].dsl_data;

    return states[dsl_state_pos].dsl_data_start;
}

uint8_t get_byte() {
    uint8_t answer = (uint8_t) *dsl_data;
    dsl_data++;
    return answer;
}

static uint16_t get_word() {
    uint16_t ret;
    ret = get_byte() * 0x100;
    ret += get_byte();
    return ret;
}

uint8_t peek_one_byte() {
    return *dsl_data;
}

uint16_t peek_half_word() {
    uint16_t ret;
    ret = (*dsl_data) *0x100;
    ret += *(dsl_data + 1);
    return ret;
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
        //printf("current operation = 0x%x\n", cop);
        if (cop < 0x80) {
            cval = cop * 0x100 + get_byte();
            //printf("cval = %d\n", cval);
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
                    push_params();
                    do_dsl_command(get_byte());
                    pop_params();
                    cval = accum;
                    break;
                }
                //case DSL_HI_RETVAL|0x80:
                case DSL_IMMED_BIGNUM|0x80: {
                    cval = (int32_t)((int16_t)get_word()) * 655356L 
                         + (int32_t)((uint16_t)get_word());
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
                    cval = read_complex();
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
                    opstack[paren_level] = cop;
                    do_next = 1;
                    found_operator = 1;
                    break;
                }
                case DSL_HI_OPEN_PAREN: {
                    if (++paren_level >= MAX_PARENS) {
                        fprintf(stderr, "ERROR: exceeded max paren level!\n");
                        exit(1);
                    }
                    do_next = 1;
                    cval = 0;
                    break;
                }
                case DSL_HI_CLOSE_PAREN: {
                    cval = accums[paren_level];
                    if (--paren_level < 0) {
                        fprintf(stderr, "ERROR: paren level is < 0!\n");
                        exit(1);
                    }
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
            //printf("operator not found, opstack[%d] = 0x%x\n", paren_level, opstack[paren_level]);
            tval = accums[paren_level];
            switch(opstack[paren_level]) {
                case DSL_PLUS:   tval += cval; break;
                case DSL_MINUS:  tval -= cval; break;
                case DSL_MULT:   tval *= cval; break;
                case DSL_DIV:    
                    if (cval == 0) {
                        fprintf(stderr, "ERROR: trying to divide by 0!\n");
                        tval = 0;
                    } else {
                        tval /= cval; 
                    }
                    break;
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

static uint8_t access_complex(int16_t *header, uint16_t *depth, uint16_t *element) {
    uint16_t i;
    int32_t obj_name;
    
    obj_name = get_word();
    if (obj_name < 0x8000) {
        printf("access_complex: I need to convert from ID to header!\n");
    } else {
        printf("access_complex: I need to set the *head to the correct view\n");
        switch (obj_name & 0x7FFF) {
            case 0x25: // POV
            case 0x26: // ACTIVE
            case 0x27: // PASSIVE
            case 0x28: // OTHER
            case 0x2C: // OTHER1
            case 0x2B: // THING
                printf("access_complext:valid obj_name, need to set header (but can't yet...)\n");
                break;
            default:
                return 0;
        }
    }
    *depth = get_byte();
    for (i = 1; i <= *depth; i++) {
        element[i-1] = get_byte();
    }

    return 1;
}

int32_t get_complex_data(int16_t header, uint16_t depth, uint16_t *element) {
    uint16_t i = 0;
    int32_t ret;

    ret = header; // start with header.

    do {
        ret = data_field((int16_t) ret, i);
        element++;
        i++;
    } while (/*(field_error == 0) && */ (i < depth));

    return ret;
}

static void write_data(int16_t header, uint16_t depth, int32_t data) {
    printf("I need to write to obj-header %d, depth = %d, the value %d\n", header, depth, data);
}

static void write_complex_data(int16_t header, uint16_t depth, uint16_t *element, int32_t data) {
    int32_t chead;

    chead = get_complex_data(header, depth-1, element);
    if (/*(field_error == 0) && */chead != NULL_OBJECT) {
        write_data(chead, element[depth-1], data);
    }
}

static void smart_write_data(int16_t header, uint16_t depth, uint16_t element[], int32_t data) {
    if (depth == 1) {
        write_data(header, element[0], data);
    } else {
        write_complex_data(header, depth, element, data);
    }
}

static void write_complex_var(int32_t data) {
    uint16_t depth = 0;
    int16_t header = 0;
    uint16_t element[MAX_SEARCH_STACK];
    memset(element, 0x0, sizeof(uint16_t) * MAX_SEARCH_STACK);
    if (access_complex(&header, &depth, element) == 1) {
        smart_write_data(header, depth, element, data);
    }
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
        varnum = get_byte();
        if (extended == 1) {
            varnum *= 0x100;
            varnum += get_byte();
        }
        load_simple_variable(datatype, varnum, accum);
    } else {
        write_complex_var(accum);
    }
}

static void load_simple_variable(uint16_t type, uint16_t vnum, int32_t val) {
    switch (type) {
        case DSL_GBIGNUM:
            gGbignumvar[vnum] = (int32_t) val;
            break;
        case DSL_LBIGNUM:
            gLbignumvar[vnum] = (int32_t) val;
            break;
        case DSL_GNUM:
            gGnumvar[vnum] = (int16_t) val;
            break;
        case DSL_LNUM:
            gBignumptr = (int32_t*) ((int16_t *)&gLnumvar[temps16]);
            gBignum = gLnumvar[temps16];
            break;
        case DSL_GFLAG:
            if (val == 0) {
                gGflagvar[vnum/8] &= bytemask[vnum%8];
            } else {
                gGflagvar[vnum/8] |= bitmask[vnum%8];
            }
            break;
        case DSL_LFLAG:
            gBignumptr = (int32_t*) ((int8_t*) &gLflagvar[temps16/8]);
            gBignum = gLflagvar[temps16/8] & bitmask[temps16%8];
            break;
        default:
            fprintf(stderr, "ERROR: Unknown simple variable type: 0x%x!\n", type);
            command_implemented = 0;
            break;
    }
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
            gBignumptr = (int32_t*)((int8_t*)&gLflagvar[temps16/8]);
            gBignum = gLflagvar[temps16/8] & bitmask[temps16/8];
            if (gBignum > 0) {
                gBignum = 1;
            }
            break;
        }
        case DSL_GNUM: {
            gBignumptr = (int32_t*) ((int16_t *)&gGnumvar[temps16]);
            gBignum = gGnumvar[temps16];
            break;
        }
        case DSL_LNUM: {
            gBignumptr = (int32_t*) ((int16_t *)&gLnumvar[temps16]);
            gBignum = gLnumvar[temps16];
            break;
        }
        case DSL_GBIGNUM: {
            gBignumptr = &gGbignumvar[temps16];
            gBignum = gGbignumvar[temps16];
            break;
        }
        case DSL_LBIGNUM: {
            gBignumptr = &gLbignumvar[temps16];
            gBignum = gLbignumvar[temps16];
            break;
        }
        case DSL_GNAME: {
            if (temps16 >= 0x20 && temps16 < 0x2F) {
                gBignumptr = (int32_t*)gSimpleVar[temps16 - 0x20];
                if (temps16 >= 0x29 && temps16 <= 0x2A) {
                    gBignum = *((int32_t*)gBignumptr);
                } else {
                    gBignum = *((int16_t*)gBignumptr);
                }
            } else {
                printf("ERROR: No variable GNAME!!!\n");
                exit(1);
            }
            break;
        }
        case DSL_GSTRING: {
            gBignumptr = (int32_t*) gGstringvar[temps16];
            break;
        }
        case DSL_LSTRING: {
            gBignumptr = (int32_t*) gLstringvar[temps16];
            break;
        }
        default:
            printf("ERROR: Unknown type in read_simple_num_var.\n");
            command_implemented = 0;
            break;
    }
}

static int32_t read_complex(void) {
    int32_t ret = 0;
    uint16_t depth = 0;
    int16_t header = 0;
    uint16_t element[MAX_SEARCH_STACK];
    memset(element, 0x0, sizeof(uint16_t) * MAX_SEARCH_STACK);

    if (access_complex(&header, &depth, element) == 1) {
        ret = get_complex_data(header, depth, element);
    } else {
        printf("read_complex: else not implemented!\n");
        command_implemented = 0;
    }

    return ret;
}

void setrecord() {
    uint16_t depth = 0;
    int16_t header = 0;
    uint16_t element[MAX_SEARCH_STACK];
    uint16_t tmp = peek_half_word();

    if (tmp > 0x8000) {
        access_complex(&header, &depth, element);
        accum = read_number();
        smart_write_data(header, depth, element, accum);
        return;
    }
    if (tmp == 0) {
        printf("dsl_setrecord: need to implement party...\n");
        command_implemented = 0;
        return;
    }
    if (tmp < 0x8000) {
        access_complex(&header, &depth, element);
        set_accumulator(read_number());
        printf("I need to write depth/element/accum to list of headers!\n");
        return;
    }
}
