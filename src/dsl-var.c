#include "dsl-execute.h"
#include "dsl-string.h"
#include "dsl-var.h"
#include <string.h>

/* externals first */
uint16_t this_gpl_file = 0;
uint16_t this_gpl_type = 0;

/* Now static to file... */
static uint8_t dsl_global_flags[DSL_GFLAGVAR_SIZE];
static uint8_t dsl_local_flags[DSL_LFLAGVAR_SIZE];
static uint8_t datatype;
static uint16_t varnum;
static int32_t accum; //, number;
static unsigned char* dsl_data;
static unsigned char* dsl_data_start;
static int16_t temps16;
static uint8_t bitmask[8] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80};
static uint8_t bytemask[8] = {0xFE, 0xFD, 0xFB, 0xF7, 0xEF, 0xDF, 0xBF, 0x7F};
static int16_t *dsl_global_nums = 0;
static int16_t *dsl_local_nums = 0;
static int32_t *dsl_global_bnums = 0;
static int32_t *dsl_local_bnums = 0;
static dsl_control_t control_table[MAX_OBJECT_PATH];
static check_index_t gunused_checks;

static dsl_check_t *check;
lua_State *lua_state = NULL;

// For Debugging
const char* debug_index_names[] = {
    "UNSUED CHECK",
    "TALK CHECK",
    "ATTACK CHECK",
    "POV CHECK",
    "PICKUP CHECK",
    "OTHER CHECK",
    "OTHER1 CHECK",
    "MOVE TILE CHECK",
    "LOOK CHECK",
    "USE CHECK",
    "USE WITH CHECK",
    "MOVE BOX CHECK",
};

static void load_simple_variable(uint16_t type, uint16_t vnum, int32_t val);
static int32_t read_complex(void);

static dsl_check_t checks[MAX_CHECK_TYPES][MAX_DSL_CHECKS];
static int checks_pos[MAX_CHECK_TYPES];
static name_t new_name;
static name2_t new_name2;

typedef struct _dsl_state_t {
    unsigned char *dsl_data_start;
    unsigned char *dsl_data;
} dsl_state_t;

int32_t gBignum;
int32_t *gBignumptr;
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

/* All those commands... */
void global_addr_name(param_t *par) {
    new_name.addr = par->val[0];
    new_name.file = par->val[1];
    new_name.name = par->val[2];
    new_name.global = 0;

    if (this_gpl_file == GLOBAL_MAS) {
        if (this_gpl_type == MASFILE) {
            new_name.global = 1;
        }
    }
}

void name_name_global_addr(param_t *par) {
    new_name2.name1 = par->val[0];
    new_name2.name2 = par->val[1];
    new_name2.addr = par->val[2];
    new_name2.file = par->val[3];
    new_name2.global = 0;
    if (this_gpl_file == GLOBAL_MAS) {
        if (this_gpl_type == MASFILE) {
            //printf("GLOBAL!!!!!!!!!!!!!\n");
            new_name2.global = 1;
        }
    }
}

static char buf[1024];

static const char* get_so_name(so_object_t *so) {
    disk_object_t *dobj = NULL;
    switch(so->type) {
        case SO_DS1_COMBAT:
            return so->data.ds1_combat.name;
            break;
        case SO_DS1_ITEM:
            dobj = gff_get_object(so->data.ds1_item.id);
            //dobj = gff_get_object(so->data.ds1_item.name_index);
            //sprintf(buf, "item (index = %d, bmp_id = %d)", so->data.ds1_item.name_index, dobj->bmp_id);
            sprintf(buf, "item (id = %d, bmp_id = %d, script_id = %d)", so->data.ds1_item.id, dobj->bmp_id,
            dobj->script_id);
            //return "<ITEM-need to implement>";
            return buf;
            break;
    }
    return "UNKNOWN";
}

static void print_name_check(int check_index) {
    so_object_t *so = gff_object_inspect(OBJEX_GFF_INDEX, abs(new_name.name));
    debug("When I %s to '%s' (%d) goto file: %d, addr: %d, global = %d\n",
        debug_index_names[check_index], get_so_name(so), new_name.name,
        new_name.file, new_name.addr, new_name.global);
}

void generic_name_check(int check_index) {
    int cpos = checks_pos[check_index]; // Where in the list we are.
    if (cpos > MAX_DSL_CHECKS) {
        fprintf(stderr, "FATAL ERROR: Max checks reached! ci = %d\n", check_index);
        exit(1);
    }
    checks[check_index][cpos].data.name_check = new_name;
    checks[check_index][cpos].next = cpos + 1;
    checks[check_index][cpos].type = check_index;

    print_name_check(check_index);

    checks_pos[check_index]++;
}

// This should use a hash table, FIXME/REFACTOR!
dsl_check_t* dsl_find_check(int32_t type, int32_t id) {
    int pos = -1;
    if (type < 0 || type >= MAX_CHECK_TYPES) { return NULL; }

    id = abs(id);
    for (int i = 0; i < checks_pos[type]; i++) {
        if (abs(checks[type][i].data.name_check.name) == id) {
            pos = i;
            debug ("find check %d", pos);
            //return checks[type]+i;
        }
    }

    if (pos >= 0) { return checks[type] + pos; }

    return NULL;
}

#define DSL_CHECKS (200)
void dsl_init_vars() {
    memset(dsl_global_flags, 0x00, DSL_GFLAGVAR_SIZE);
    memset(dsl_local_flags, 0x00, DSL_LFLAGVAR_SIZE);
    dsl_global_bnums = malloc(DSL_GBIGNUMVAR_SIZE * sizeof(int32_t));
    memset(dsl_global_bnums, 0x00, DSL_GBIGNUMVAR_SIZE * sizeof(int32_t));
    dsl_global_nums = malloc(DSL_GNUMVAR_SIZE);
    memset(dsl_global_nums, 0x00, DSL_GNUMVAR_SIZE);
    dsl_local_bnums = malloc(DSL_LBIGNUMVAR_SIZE * sizeof(int32_t));
    memset(dsl_local_bnums, 0x00, DSL_LBIGNUMVAR_SIZE * sizeof(int32_t));
    dsl_local_nums = malloc(DSL_LNUMVAR_SIZE);
    memset(dsl_local_nums, 0x00, DSL_LNUMVAR_SIZE);
    memset(checks, 0x00, sizeof(dsl_check_t) * MAX_CHECK_TYPES * MAX_DSL_CHECKS);
    memset(checks_pos, 0x00, sizeof(int) * MAX_CHECK_TYPES);
    memset(control_table, 0x00, sizeof(dsl_control_t) * MAX_OBJECT_PATH);
    gunused_checks = 0;
    check = (dsl_check_t*) malloc(sizeof(dsl_check_t) * DSL_CHECKS);
    memset(check, 0x0, sizeof(dsl_check_t) * DSL_CHECKS);
    for (int i = 0; i < DSL_CHECKS; i++) {
        check[i].next = i + 1;
    }
    check[DSL_CHECKS - 1].next = NULL_CHECK;
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
    dsl_data_start = data;

    debug("pushing %p: %p\n", dsl_data_start, dsl_data);
}

void clear_local_vars() {
    //memset(dsl_global_flags, 0x0, DSL_GFLAGVAR_SIZE);
}

unsigned char* pop_data_ptr() {
    dsl_state_pos--;

    if (dsl_state_pos < 0) {
        return NULL;
    }

    dsl_data_start = states[dsl_state_pos].dsl_data_start;
    dsl_data = states[dsl_state_pos].dsl_data;

    debug("pop %p: %p\n", dsl_data_start, dsl_data);
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
            //debug("immediate = %d\n", cval);
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
                    //debug("DSL_ACCUM cval = %d\n", cval);
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
                    //debug("DSL_RETVAL begin:\n");
                    push_params();
                    do_dsl_command(get_byte());
                    pop_params();
                    cval = accum;
                    //debug("DSL_RETVAL end, cval = %d\n", cval);
                    break;
                }
                case DSL_IMMED_BIGNUM|0x80: {
                    cval = (int32_t)((int16_t)get_word()) * 655356L 
                         + (int32_t)((uint16_t)get_word());
                    //debug("DSL_IMMED_BIGNUM, cval = %d\n", cval);
                    break;
                }
                case DSL_IMMED_BYTE|0x80: {
                    cval = (int32_t)((int8_t)get_byte());
                    //debug("DSL_IMMED_BYTE, cval = %d\n", cval);
                    break;
                }
                case DSL_IMMED_WORD|0x80: {
                    printf("IMMED_WORD not implemented!\n");
                    command_implemented = 0;
                    break;
                }
                case DSL_IMMED_NAME|0x80: {
                    cval = (int32_t)((int16_t)get_half_word() * -1);
                    //debug("DSL_IMMED_NAME, cval = %d\n", cval);
                    break;
                }
                case DSL_COMPLEX_VAL|0x80: {
                    cval = read_complex();
                    break;
                }
                case DSL_IMMED_STRING|0x80: {
                    cval = 0;
                    gBignumptr = (int32_t*) read_text();
                    //debug("DSL_IMMED_STRING, cval = %d, '%s'\n", cval, (char*) gBignumptr);
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
                    printf(" UNKNOWN OP: 0x%x or 0x%x\n", cop&0x7F, cop);
                    command_implemented = 0;
                    break;
                }
            }
        }
        
        if (!found_operator) {
            tval = accums[paren_level];
            //printf("operator not found, opstack[%d] = 0x%x, tval = %d\n", paren_level, opstack[paren_level], tval);
            //debug("operator: %d\n", opstack[paren_level]);
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
            //printf("tval = %d, cval = %d\n", tval, cval);
        }
    } while (do_next || 
        (((next_op = preview_byte(0)) > OPERATOR_OFFSET && next_op <= OPERATOR_LAST)
            || (paren_level > 0 && next_op == DSL_HI_CLOSE_PAREN)));
    // We need to look if that above else was executed!
    //printf("accums[0] = %d\n", accums[0]);
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
        debug("writting header (%d) at depth (%d)\n", header, depth);
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
        warn("------------NEED TO DEBUG WRITE COMPLEX VAR------------\n");
        write_complex_var(accum);
    }
}

static void load_simple_variable(uint16_t type, uint16_t vnum, int32_t val) {
    switch (type) {
        case DSL_GBIGNUM:
            dsl_global_bnums[vnum] = (int32_t) val;
            debug("dsl_global_bnums[%d] = %d\n", vnum, val);
            break;
        case DSL_LBIGNUM:
            dsl_local_bnums[vnum] = (int32_t) val;
            debug("dsl_local_bnums[%d] = %d\n", vnum, val);
            break;
        case DSL_GNUM:
            dsl_global_nums[vnum] = (int16_t) val;
            debug("dsl_global_nums[%d] = %d\n", vnum, val);
            break;
        case DSL_LNUM:
            dsl_local_nums[vnum] = (int16_t) val;
            debug("dsl_local_nums[%d] = %d\n", vnum, val);
            break;
        case DSL_GFLAG:
            if (val == 0) {
                dsl_global_flags[vnum/8] &= bytemask[vnum%8];
            } else {
                dsl_global_flags[vnum/8] |= bitmask[vnum%8];
            }
            debug("dsl_global_flags bit %d = %d\n", vnum, val);
            break;
        case DSL_LFLAG:
            if (val == 0) {
                dsl_local_flags[vnum/8] &= bytemask[vnum%8];
            } else {
                dsl_local_flags[vnum/8] |= bitmask[vnum%8];
            }
            debug("dsl_local_flags bit %d = %d\n", vnum, val);
            break;
        default:
            error("ERROR: Unknown simple variable type: 0x%x!\n", type);
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
            gBignumptr = (int32_t*)((int8_t*)&dsl_global_flags[temps16/8]);
            gBignum = dsl_global_flags[temps16/8] & bitmask[temps16%8];
            if (gBignum > 0) { gBignum = 0; }
            debug("reading gflag @ %d is equal to %d\n", temps16, gBignum);
            break;
        }
        case DSL_LFLAG: {
            //printf("getting LFLAG @ %d\n", temps16);
            gBignumptr = (int32_t*)((int8_t*)&dsl_local_flags[temps16/8]);
            gBignum = dsl_local_flags[temps16/8] & bitmask[temps16/8];
            if (gBignum > 0) {
                gBignum = 1;
            }
            debug("reading lflag @ %d is equal to %d\n", temps16, gBignum);
            break;
        }
        case DSL_GNUM: {
            gBignumptr = (int32_t*) ((int16_t *)&dsl_global_nums[temps16]);
            gBignum = dsl_global_nums[temps16];
            debug("reading gnum @ %d is equal to %d\n", temps16, gBignum);
            break;
        }
        case DSL_LNUM: {
            gBignumptr = (int32_t*) ((int16_t *)&dsl_local_nums[temps16]);
            gBignum = dsl_local_nums[temps16];
            debug("reading lnum @ %d is equal to %d\n", temps16, gBignum);
            break;
        }
        case DSL_GBIGNUM: {
            gBignumptr = &dsl_global_bnums[temps16];
            gBignum = dsl_global_bnums[temps16];
            debug("reading gbignum @ %d is equal to %d\n", temps16, gBignum);
            break;
        }
        case DSL_LBIGNUM: {
            gBignumptr = &dsl_local_bnums[temps16];
            gBignum = dsl_local_bnums[temps16];
            debug("reading lbignum @ %d is equal to %d\n", temps16, gBignum);
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
            debug("reading gname @ %d is equal to %d\n", temps16, gBignum);
            break;
        }
        case DSL_GSTRING: {
            gBignumptr = (int32_t*) dsl_global_strings[temps16];
            debug("reading gstring @ %d is equal to '%s'\n", temps16, (char*)gBignumptr);
            break;
        }
        case DSL_LSTRING: {
            gBignumptr = (int32_t*) dsl_local_strings[temps16];
            debug("reading lstring @ %d is equal to '%s'\n", temps16, (char*)gBignumptr);
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
        debug("reading header (%d) at depth (%d)\n", header, depth);
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

void set_any_order(name_t *name, int16_t to, int16_t los_order, int16_t range) {
    warn("set_any_order: lua callback needed: Get all objects with 'name' and set then to to with los_order and in range\n");
    warn("then set the order!");
    /*
    foreach obj_index, where object's name = pName->name
    control_table[obj_index].addr[to] = name->addr;
    control_table[obj_index].file[to] = file->addr;
    if (to == DSL_LOS_ORDER) {
        control_table[obj_index].command[to] = losOrder;
        control_table[obj_index].flags &= CF_MOVED;
        control_table[obj_index].flags |= range;
    }
    */
}

static check_index_t get_check_node() {
    check_index_t node_index;
    if (gunused_checks == NULL_CHECK) {
        return NULL_CHECK;
    }
    node_index = gunused_checks;
    gunused_checks = check[node_index].next;
    check[node_index].next = NULL_CHECK;
    return node_index;
}

static void insert_check(check_index_t *cindex) {
    check_index_t new_head, old_head;
    if (*cindex == NULL_CHECK) {
        *cindex = get_check_node();
        return;
    }
    old_head = *cindex;
    new_head = get_check_node();
    if (new_head == NULL_CHECK) {
        return;
    }
    check[new_head].next = old_head;
    *cindex = new_head;
}

void use_with_check(check_index_t *cindex) {
    name2_t name = new_name2;
    while ((*cindex != NULL_CHECK) && (check[*cindex].data.name2_check.name1 < name.name1 )) {
        use_with_check(&check[*cindex].next);
        return;
    }
    while ((*cindex != NULL_CHECK) && (check[*cindex].data.name2_check.name2 < name.name2 )) {
        use_with_check(&check[*cindex].next);
        return;
    }
    insert_check(cindex);
    if (*cindex == NULL_CHECK) {
        return;
    }
    check[*cindex].data.name2_check = name;
    debug("insert with check = {file = %d, addr = %d, name1 = %d, name2 = %d, is_global = %d}\n",
        name.file, name.addr, name.name1,
        name.name2, name.global);
}

void generic_box_check(check_index_t *cindex, box_t box) {
    while ((*cindex != NULL_CHECK) && (check[*cindex].data.box_check.x < box.x)) {
        generic_box_check(&check[*cindex].next, box);
        return;
    }
    while ((*cindex != NULL_CHECK) && (check[*cindex].data.box_check.y < box.y)) {
        generic_box_check(&check[*cindex].next, box);
        return;
    }
    if ((*cindex == NULL_CHECK) || (check[*cindex].data.box_check.x != box.x) || (check[*cindex].data.box_check.y !=
        box.y)) {
        insert_check(cindex);
    }
    if (*cindex == NULL_CHECK) {
        return;
    }
    check[*cindex].data.box_check = box;
    debug("tile with check = {file = %d, addr = %d, x = %d, y = %d, xd = %d, yd = %d, trip = %d}\n",
        box.file, box.addr, box.x, box.xd, box.yd,
        box.y, box.trip);
}

void generic_tile_check(check_index_t *cindex, tile_t tile) {
    while ((*cindex != NULL_CHECK) && (check[*cindex].data.tile_check.x < tile.x)) {
        generic_tile_check(&check[*cindex].next, tile);
        return;
    }
    while ((*cindex != NULL_CHECK) && (check[*cindex].data.tile_check.y < tile.y)) {
        generic_tile_check(&check[*cindex].next, tile);
        return;
    }
    if ((*cindex == NULL_CHECK) || (check[*cindex].data.tile_check.x != tile.x)
        || (check[*cindex].data.tile_check.y != tile.y)) {
        insert_check(cindex);
    }
    if (*cindex == NULL_CHECK) {
        return;
    }
    check[*cindex].data.tile_check = tile;
    debug("tile with check = {file = %d, addr = %d, x = %d, y = %d, trip = %d}\n",
        tile.file, tile.addr, tile.x,
        tile.y, tile.trip);
}

static void add_save_orders(int16_t los_order, name_t name, int16_t range, int ordertype) {
    if (name.name < 0) {
        error("*******************add_save_orders (with name < 0) not implemented****************\n");
        //command_implemented = 0;
    }
}

#define DSL_ORDER     (0)
#define DSL_LOS_ORDER (1)

void set_los_order(int16_t los_order, int16_t range) {
    warn("LOS Check ignored: addr = %d, file = %d, name = %d, global = %d\n", new_name.addr,
        new_name.file, new_name.name, new_name.global);
    add_save_orders(los_order, new_name, range, DSL_LOS_ORDER);
    set_any_order(&new_name, DSL_LOS_ORDER, los_order, range);
}

// Uses global new_name!
void set_new_order() {
    warn("new_name->{addr = %d, file = %d, name = %d, global = %d}\n", new_name.addr, new_name.file, new_name.name,
    new_name.global);
    if (!new_name.file) {
        if (new_name.name < 0) {
            error("set_new_order name < 0 not implemented!\n");
            command_implemented = 0;
        }
    } else {
        add_save_orders(0, new_name, 0, DSL_ORDER);
    }
    set_any_order(&new_name, DSL_ORDER, 0, 0);
}

