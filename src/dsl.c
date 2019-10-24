#include <stdio.h>
#include <string.h>
#include "dsl.h"
#include "dsl-scmd.h"

/* SCMD */

static scmd_t* get_script(unsigned char* scmd_entry, const int index) {
    if (scmd_entry == NULL) { return NULL; }
    if (index < 0 || index >= SCMD_MAX_SIZE) {
        fprintf(stderr, "index for get_script is out of bounds!(%d)\n", index);
        return NULL;
    }
    scmd_t *scmds = (scmd_t*)(scmd_entry + (SCMD_MAX_SIZE * 2));
    uint16_t scmd_idx = *((uint16_t*)scmd_entry + index);
    return scmds + scmd_idx;
}

static void print_scmd(scmd_t *scmd) {
    printf("bmp_idx = %d, delay = %d, flags = 0x%x, xoffset = %d, yoffset = %d, "
        "xoffsethot = %d, yoffsethot = %d, soundidx = %d\n",
        scmd->bmp_idx,
        scmd->delay,
        scmd->flags,
        scmd->xoffset,
        scmd->yoffset,
        scmd->xoffsethot,
        scmd->yoffsethot,
        scmd->soundidx
    );
}

static void print_script(scmd_t *script) {
    if (script == NULL) { return; }
    while((script->flags & SCMD_LAST) == 0) {
        print_scmd(script);
        script++;
    }
    print_scmd(script);
}

scmd_t* dsl_scmd_get(const int gff_file, const int res_id, const int index) {
    unsigned long len;
    char *scmd_entry = gff_get_raw_bytes(gff_file, GT_SCMD, res_id, &len);
    return get_script((unsigned char*) scmd_entry, index);
}

void dsl_scmd_print(int gff_file, int res_id) {
    unsigned long len;
    char *scmd_entry = gff_get_raw_bytes(gff_file, GT_SCMD, res_id, &len);
    if (scmd_entry == NULL) { return; }
    int script_size = len - (SCMD_MAX_SIZE * 2);
    int script_cmds = script_size / sizeof(scmd_t);
    printf("-------------------------PRINTING SCMD ENTRY----------------------\n");
    printf("len = %lu, script_size = %d, script_cmds = %d\n", len, script_size, script_cmds);
    printf("Default Script (#0):\n");
    print_script(get_script((unsigned char*) scmd_entry, 0));
    uint16_t *scmd_idx = (uint16_t*)scmd_entry;

    int script_count = 1;
    for (int i = 0; i < SCMD_MAX_SIZE; i++) {
        if (scmd_idx[i] > 0) {
            script_count++;
            printf("Script #%d jumps to %d, entry is:\n", i, scmd_idx[i]);
            print_script(get_script((unsigned char*) scmd_entry, i));
        }
    }

    printf("Detected %d scripts\n", script_count);
}
/* END SCMD */

#define STRLEN     (20) // Max length of DSL symbols.
#define DSLSTRLEN  (40) // max letters for a line.
#define GLOBAL_MAS (99)
// POTW: to delete later
//#define GT_GPL  GFFTYPE('G','P','L','\x20') // Compiled GPL files
//#define GT_GPLX GFFTYPE('G','P','L','X')    // GPL index file (GPLSHELL)
//#define GT_MAS  GFFTYPE('M','A','S','\x20') // Compiled MAS (GPL master) files
#define MAX_PARAMETERS (8)
typedef struct _param_t {
    int32_t val[MAX_PARAMETERS];
    int32_t *ptr[MAX_PARAMETERS];
} param_t;

static param_t param;
static uint32_t dsl_pos = 0;

void get_parameters(int16_t amt);
int32_t read_number();

void dsl_zero(void);
void dsl_long_div_eq(void);
void dsl_byte_dec(void);
void dsl_word_dec(void);
void dsl_long_dec(void);
void dsl_byte_inc(void);
void dsl_word_inc(void);
void dsl_long_inc(void);
void dsl_hunt(void);
void dsl_getxy(void);
void dsl_string_copy(void);

#define DSL_MAX_COMMANDS (11)

void (*dsl_commands[DSL_MAX_COMMANDS])(void) = {
    dsl_zero,
    dsl_long_div_eq,
    dsl_byte_dec,
    dsl_word_dec,
    dsl_long_dec,
    dsl_byte_inc,
    dsl_word_inc,
    dsl_long_inc,
    dsl_hunt,
    dsl_getxy,
    dsl_string_copy,
};

void dsl_init() {
    printf("START DSL!\n");
}

static char* dsl_data;

void dsl_check_for_updates() {
    gff_file_t *dsl_file = open_files + GPLDATA_GFF_INDEX;
    unsigned long len;
    if (dsl_file != NULL && dsl_file->filename != NULL) {
        printf("GPL file detected, loading game data.\n");
        //char *data = gff_get_raw_bytes(GPLDATA_GFF_INDEX, GT_MAS, GLOBAL_MAS, &len);
        char *data = gff_get_raw_bytes(GPLDATA_GFF_INDEX, GT_MAS, 42, &len);
        dsl_pos = 0;
        printf("data = %p, len = %lu\n", data, len);
        printf("first byte: 0x%x\n", data[dsl_pos]);
        dsl_data = data;
        if (data[dsl_pos] < DSL_MAX_COMMANDS) {
            dsl_pos++;
            (*dsl_commands[(unsigned int)data[dsl_pos - 1] & 0x00FF])();
        }
    }
}

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

static int32_t accum, number;
#define MAX_PARENS (8)

int32_t read_number() {
    accum = number = 0;
    int32_t paren_level = 0;
    int8_t found_operator = 0; // did we find an operation?
    int16_t opstack[MAX_PARENS];
    int32_t accums[MAX_PARENS];
    int16_t cop = dsl_data[dsl_pos++]; // current operation
    int32_t cval, tval; // current value, temporary value
    memset(opstack, 0, sizeof(opstack));
    memset(accums, 0, sizeof(accums));
    printf("current operation = %d\n", cop);
    if (cop < 0x80) {
        cval = cop * 0x100 + dsl_data[dsl_pos++];
        printf("cval = %d\n", cval);
    } else {
        printf("in ELSE!, not implemented!!!\n");
    }
    
    if (!found_operator) {
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
    }
    accums[paren_level] = tval;
    opstack[paren_level] = 0;
    // Wee need to look if that above else was executed!
    return accums[0];
}

void get_parameters(int16_t amt) {
    for (int16_t i = 0; i < amt; i++) {
        param.val[i] = read_number();
        param.ptr[i] = 0; // gBigNumPtr ?
    }
}

/* All those commands... */
void dsl_zero(void) {
}

void dsl_long_div_eq(void) {
}

void dsl_byte_dec(void) {
}

void dsl_word_dec(void) {
}

void dsl_long_dec(void) {
}

void dsl_byte_inc(void) {
}

void dsl_word_inc(void) {
}

void dsl_long_inc(void) {
}

void dsl_hunt(void) {
}

void dsl_getxy(void) {
}

void dsl_string_copy(void) {
    printf("String copy!\n");
    get_parameters(2);
    printf("num1 = %d\n", param.val[0]);
    printf("num2 = %d\n", param.val[1]);
}

