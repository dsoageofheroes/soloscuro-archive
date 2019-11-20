#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "dsl.h"
#include "dsl-scmd.h"
#include "dsl-string.h"
#include "dsl-var.h"

/* MAS */

void dsl_zero(void);
void dsl_long_divide_equal(void);
void dsl_byte_dec(void);
void dsl_word_dec(void);
void dsl_long_dec(void);
void dsl_byte_inc(void);
void dsl_word_inc(void);
void dsl_long_inc(void);
void dsl_hunt(void);
void dsl_getxy(void);
void dsl_string_copy(void);
void dsl_p_damage(void);
void dsl_changemoney(void);
void dsl_setvar(void);
void dsl_toggle_accum(void);
void dsl_getstatus(void);
void dsl_getlos(void);
void dsl_long_times_equal(void);
void dsl_jump(void);
void dsl_local_sub(void);
void dsl_global_sub(void);
void dsl_local_ret(void);
void dsl_load_variable(void);
void dsl_compare(void);
void dsl_load_accum(void);
void dsl_global_ret(void);
void dsl_nextto(void);
void dsl_inloscheck(void);
void dsl_notinloscheck(void);
void dsl_clear_los(void);
void dsl_nametonum(void);
void dsl_numtoname(void);
void dsl_bitsnoop(void);
void dsl_award(void);
void dsl_request(void);
void dsl_source_trace(void);
void dsl_shop(void);
void dsl_clone(void);
void dsl_default(void);
void dsl_ifis(void);
void dsl_trace_var(void);
void dsl_orelse(void);
void dsl_clearpic(void);
void dsl_continue(void);
void dsl_log(void);
void dsl_damage(void);
void dsl_source_line_num(void);
void dsl_drop(void);
void dsl_passtime(void);
void dsl_exit_gpl(void);
void dsl_fetch(void);
void dsl_search(void);
void dsl_getparty(void);
void dsl_fight(void);
void dsl_flee(void);
void dsl_follow(void);
void dsl_getyn(void);
void dsl_give(void);
void dsl_go(void);
void dsl_input_bignum(void);
void dsl_goxy(void);
void dsl_readorders(void);
void dsl_if(void);
void dsl_else(void);
void dsl_setrecord(void);
void dsl_setother(void);
void dsl_input_string(void);
void dsl_input_number(void);
void dsl_input_money(void);
void dsl_joinparty(void);
void dsl_leaveparty(void);
void dsl_lockdoor(void);
void dsl_menu(void);
void dsl_setthing(void);
//void dsl_default(void);
void dsl_local_sub_trace(void);
//void dsl_default(void);
//void dsl_default(void);
//void dsl_default(void);
void dsl_print_string(void);
void dsl_print_number(void);
void dsl_printnl(void);
void dsl_rand(void);
//void dsl_default(void);
void dsl_showpic(void);
//void dsl_default(void);
//void dsl_default(void);
//void dsl_default(void);
void dsl_skillroll(void);
void dsl_statroll(void);
void dsl_string_compare(void);
void dsl_match_string(void);
void dsl_take(void);
void dsl_sound(void);
void dsl_tport(void);
void dsl_music(void);
//void dsl_default(void);
void dsl_cmpend(void);
void dsl_wait(void);
void dsl_while(void);
void dsl_wend(void);
void dsl_attackcheck(void);
void dsl_lookcheck(void);
void dsl_endif(void);
void dsl_move_tilecheck(void);
void dsl_door_tilecheck(void);
void dsl_move_boxcheck(void);
void dsl_door_boxcheck(void);
void dsl_pickup_itemcheck(void);
void dsl_usecheck(void);
void dsl_talktocheck(void);
void dsl_noorderscheck(void);
void dsl_usewithcheck(void);
//void dsl_default(void);
//void dsl_default(void);
//void dsl_default(void);
//void dsl_default(void);
//void dsl_default(void);
void dsl_byte_plus_equal(void);
void dsl_byte_minus_equal(void);
void dsl_byte_times_equal(void);
void dsl_byte_divide_equal(void);
void dsl_word_plus_equal(void);
void dsl_word_minus_equal(void);
void dsl_word_times_equal(void);
void dsl_word_divide_equal(void);
void dsl_long_plus_equal(void);
void dsl_long_minus_equal(void);
void dsl_get_range(void);

typedef struct _name_t {
    uint16_t addr;
    uint16_t file;
    uint16_t name;
    uint8_t global;
} name_t;

#define DSL_MAX_COMMANDS (0x81)

#define check_index_t uint16_t
#define NULL_CHECK (0xFFFF)
typedef struct _dsl_check_s {
    union {
        //box_t box_check;
        //tile_t tile_check;
        name_t name_check;
        name_t name_2check;
    } data;
    uint16_t next;
} dsl_check_t;
static dsl_check_t *check;
static check_index_t gunused_checks;
static check_index_t gtalk_to = NULL_CHECK;

typedef struct _dsl_operation {
    void (*func)(void);
    const char *name;
} dsl_operation;

dsl_operation dsl_operations[] = {
    { dsl_zero, "dsl zero" }, // 0x0
    { dsl_long_divide_equal, "dsl long divide equal" }, // 0x1
    { dsl_byte_dec, "dsl byte dec" }, // 0x2
    { dsl_word_dec, "dsl word dec" }, // 0x3
    { dsl_long_dec, "dsl long dec" }, // 0x4
    { dsl_byte_inc, "dsl byte inc" }, // 0x5
    { dsl_word_inc, "dsl word inc" }, // 0x6
    { dsl_long_inc, "dsl long inc" }, // 0x7
    { dsl_hunt, "dsl hunt" }, // 0x8
    { dsl_getxy, "dsl getxy" }, // 0x9
    { dsl_string_copy, "dsl string copy" }, // 0xA
    { dsl_p_damage, "dsl p damage" }, // 0xB
    { dsl_changemoney, "dsl changemoney" }, // 0xC
    { dsl_setvar, "dsl setvar" }, // 0xD
    { dsl_toggle_accum, "dsl toggle accum" }, // 0xE
    { dsl_getstatus, "dsl getstatus" }, // 0xF
    { dsl_getlos, "dsl getlos" }, // 0x10
    { dsl_long_times_equal, "dsl long times equal" }, // 0x11
    { dsl_jump, "dsl jump" }, // 0x12
    { dsl_local_sub, "dsl local sub" }, // 0x13
    { dsl_global_sub, "dsl global sub" }, // 0x14
    { dsl_local_ret, "dsl local ret" }, // 0x15
    { dsl_load_variable, "dsl load variable" }, // 0x16
    { dsl_compare, "dsl compare" }, // 0x17
    { dsl_load_accum, "dsl load accum" }, // 0x18
    { dsl_global_ret, "dsl global ret" }, // 0x19
    { dsl_nextto, "dsl nextto" }, // 0x1A
    { dsl_inloscheck, "dsl inloscheck" }, // 0x1B
    { dsl_notinloscheck, "dsl notinloscheck" }, // 0x1C
    { dsl_clear_los, "dsl clear los" }, // 0x1D
    { dsl_nametonum, "dsl nametonum" }, // 0x1E
    { dsl_numtoname, "dsl numtoname" }, // 0x1F
    { dsl_bitsnoop, "dsl bitsnoop" }, // 0x20
    { dsl_award, "dsl award" }, // 0x21
    { dsl_request, "dsl request" }, // 0x22
    { dsl_source_trace, "dsl source trace" }, // 0x23
    { dsl_shop, "dsl shop" }, // 0x24
    { dsl_clone, "dsl clone" }, // 0x25
    { dsl_default, "dsl default" }, // 0x26
    { dsl_ifis, "dsl ifis" }, // 0x27
    { dsl_trace_var, "dsl trace var" }, // 0x28
    { dsl_orelse, "dsl orelse" }, // 0x29
    { dsl_clearpic, "dsl clearpic" }, // 0x2A
    { dsl_continue, "dsl continue" }, // 0x2B
    { dsl_log, "dsl log" }, // 0x2C
    { dsl_damage, "dsl damage" }, // 0x2D
    { dsl_source_line_num, "dsl source line num" }, // 0x2E
    { dsl_drop, "dsl drop" }, // 0x2F
    { dsl_passtime, "dsl passtime" }, // 0x30
    { dsl_exit_gpl, "dsl exit gpl" }, // 0x31
    { dsl_fetch, "dsl fetch" }, // 0x32
    { dsl_search, "dsl search" }, // 0x33
    { dsl_getparty, "dsl getparty" }, // 0x34
    { dsl_fight, "dsl fight" }, // 0x35
    { dsl_flee, "dsl flee" }, // 0x36
    { dsl_follow, "dsl follow" }, // 0x37
    { dsl_getyn, "dsl getyn" }, // 0x38
    { dsl_give, "dsl give" }, // 0x39
    { dsl_go, "dsl go" }, // 0x3A
    { dsl_input_bignum, "dsl input bignum" }, // 0x3B
    { dsl_goxy, "dsl goxy" }, // 0x3C
    { dsl_readorders, "dsl readorders" }, // 0x3D
    { dsl_if, "dsl if" }, // 0x3E
    { dsl_else, "dsl else" }, // 0x3F
    { dsl_setrecord, "dsl setrecord" }, // 0x40
    { dsl_setother, "dsl setother" }, // 0x41
    { dsl_input_string, "dsl input string" }, // 0x42
    { dsl_input_number, "dsl input number" }, // 0x43
    { dsl_input_money, "dsl input money" }, // 0x44
    { dsl_joinparty, "dsl joinparty" }, // 0x45
    { dsl_leaveparty, "dsl leaveparty" }, // 0x46
    { dsl_lockdoor, "dsl lockdoor" }, // 0x47
    { dsl_menu, "dsl menu" }, // 0x48
    { dsl_setthing, "dsl setthing" }, // 0x49
    { dsl_default, "dsl default" }, // 0x4A
    { dsl_local_sub_trace, "dsl local sub trace" }, // 0x4B
    { dsl_default, "dsl default" }, // 0x4C
    { dsl_default, "dsl default" }, // 0x4D
    { dsl_default, "dsl default" }, // 0x4E
    { dsl_print_string, "dsl print string" }, // 0x4F
    { dsl_print_number, "dsl print number" }, // 0x50
    { dsl_printnl, "dsl printnl" }, // 0x51
    { dsl_rand, "dsl rand" }, // 0x52
    { dsl_default, "dsl default" }, // 0x53
    { dsl_showpic, "dsl showpic" }, // 0x54
    { dsl_default, "dsl default" }, // 0x55
    { dsl_default, "dsl default" }, // 0x56
    { dsl_default, "dsl default" }, // 0x57
    { dsl_skillroll, "dsl skillroll" }, // 0x58
    { dsl_statroll, "dsl statroll" }, // 0x59
    { dsl_string_compare, "dsl string compare" }, // 0x5A
    { dsl_match_string, "dsl match string" }, // 0x5B
    { dsl_take, "dsl take" }, // 0x5C
    { dsl_sound, "dsl sound" }, // 0x5D
    { dsl_tport, "dsl tport" }, // 0x5E
    { dsl_music, "dsl music" }, // 0x5F
    { dsl_default, "dsl default" }, // 0x60
    { dsl_cmpend, "dsl cmpend" }, // 0x61
    { dsl_wait, "dsl wait" }, // 0x62
    { dsl_while, "dsl while" }, // 0x63
    { dsl_wend, "dsl wend" }, // 0x64
    { dsl_attackcheck, "dsl attackcheck" }, // 0x65
    { dsl_lookcheck, "dsl lookcheck" }, // 0x66
    { dsl_endif, "dsl endif" }, // 0x67
    { dsl_move_tilecheck, "dsl move tilecheck" }, // 0x68
    { dsl_door_tilecheck, "dsl door tilecheck" }, // 0x69
    { dsl_move_boxcheck, "dsl move boxcheck" }, // 0x6A
    { dsl_door_boxcheck, "dsl door boxcheck" }, // 0x6B
    { dsl_pickup_itemcheck, "dsl pickup itemcheck" }, // 0x6C
    { dsl_usecheck, "dsl usecheck" }, // 0x6D
    { dsl_talktocheck, "dsl talktocheck" }, // 0x6E
    { dsl_noorderscheck, "dsl noorderscheck" }, // 0x6F
    { dsl_usewithcheck, "dsl usewithcheck" }, // 0x70
    { dsl_default, "dsl default" }, // 0x71
    { dsl_default, "dsl default" }, // 0x72
    { dsl_default, "dsl default" }, // 0x73
    { dsl_default, "dsl default" }, // 0x74
    { dsl_default, "dsl default" }, // 0x75
    { dsl_byte_plus_equal, "dsl byte plus equal" }, // 0x76
    { dsl_byte_minus_equal, "dsl byte minus equal" }, // 0x77
    { dsl_byte_times_equal, "dsl byte times equal" }, // 0x78
    { dsl_byte_divide_equal, "dsl byte divide equal" }, // 0x79
    { dsl_word_plus_equal, "dsl word plus equal" }, // 0x7A
    { dsl_word_minus_equal, "dsl word minus equal" }, // 0x7B
    { dsl_word_times_equal, "dsl word times equal" }, // 0x7C
    { dsl_word_divide_equal, "dsl word divide equal" }, // 0x7D
    { dsl_long_plus_equal, "dsl long plus equal" }, // 0x7E
    { dsl_long_minus_equal, "dsl long minus equal" }, // 0x7F
    { dsl_get_range, "dsl get range" }, // 0x80
};

/* Globals */
void get_parameters(int16_t amt);
int32_t read_number();

uint8_t command_implemented = 0; // Temporary while I figure out each function.

/* If command data */
#define MAX_IFDEPTH (32)
int8_t ifptr = 0;
int8_t ifstate[MAX_IFDEPTH+1];
#define YES (1)
#define NO (0)

name_t new_name;
name_t new_name2;

uint16_t this_gpl_file = 0;
uint16_t this_gpl_type = 0;

enum {NOFILE, GPLFILE, MASFILE};

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
/* End Globals */

void mas_print(const int gff_file, const int res_id) {
    unsigned long len;
    unsigned char *mas = (unsigned char*)gff_get_raw_bytes(gff_file, GT_MAS, res_id, &len);
    this_gpl_file = GLOBAL_MAS;
    this_gpl_type = MASFILE;
    command_implemented = 1;
    printf("------------------------Executing MAS/GPL\n");
    set_data_ptr(mas, mas + 1);
    printf("command byte = 0x%x (%s)\n", *(mas), dsl_operations[mas[0]].name);
    (*dsl_operations[mas[0]].func)();
    while (command_implemented) {
        command_implemented = 1;
        uint8_t command = get_byte();
        printf("command byte = 0x%x", command);
        fflush(stdout);
        printf("(%s)\n", dsl_operations[command].name);
        (*dsl_operations[command].func)();
    }
    if (!command_implemented) {
        printf("last command needs to be implemented!\n");
    }
    printf("---------------------Ending Execution----------------------\n");
}

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

int dsl_scmd_is_default(const scmd_t *scmd, const int scmd_index) {
    if (scmd == NULL) { return 0; }
    if (scmd_index < 0 || scmd_index >= SCMD_MAX_SIZE) {
        fprintf(stderr, "index for get_script is out of bounds!(%d)\n", scmd_index);
        return 0;
    }
    uint16_t scmd_idx = *((uint16_t*)(((unsigned char *)scmd) + scmd_index));
    return scmd_idx == 0;
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

static void initialize_dsl_stack() {
    ifptr = 0;
    gunused_checks = 0;
    gGstringvar = (dsl_string_t*) malloc(GSTRINGVARSIZE);
    memset(gGstringvar, 0, GSTRINGVARSIZE);
    gTextstring = (uint8_t*)malloc(TEXTSTRINGSIZE);
}

#define DSL_CHECKS (200)
void dsl_init() {
    printf("Initalizing DSL.\n");
    initialize_dsl_stack();
    dsl_init_vars();
    check = (dsl_check_t*) malloc(sizeof(dsl_check_t) * DSL_CHECKS);
    for (int i = 0; i < DSL_CHECKS; i++) {
        check[i].next = i + 1;
    }
    check[DSL_CHECKS - 1].next = NULL_CHECK;
}


void dsl_check_for_updates() {
    gff_file_t *dsl_file = open_files + GPLDATA_GFF_INDEX;
    //unsigned long len;
    if (dsl_file != NULL && dsl_file->filename != NULL) {
        /*
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
        */
    }
}

void get_parameters(int16_t amt) {
    for (int16_t i = 0; i < amt; i++) {
        param.val[i] = read_number();
        param.ptr[i] = gBignumptr;
    }
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

static void generic_name_check(check_index_t *cindex, name_t newname) {
    while ((*cindex != NULL_CHECK) && (check[*cindex].data.name_check.name < new_name.name)) {
        generic_name_check(&check[*cindex].next, new_name);
        return;
    }
    if ((*cindex == NULL_CHECK) || (check[*cindex].data.name_check.name != new_name.name)) {
        insert_check(cindex);
    }
    if ((*cindex == NULL_CHECK)) { return; }
    check[*cindex].data.name_check = newname;
}

/* All those commands... */
static void global_addr_name(param_t *par) {
    new_name.addr = par->val[0];
    new_name.file = par->val[1];
    new_name.name = par->val[2];
    new_name.global = 0;

    if (this_gpl_file == GLOBAL_MAS) {
        if (this_gpl_type == MASFILE) {
            printf("GLOBAL!!!!!!!!!!!!!\n");
            new_name.global = 1;
        }
    }
}

void dsl_in_los_check(void) {
    get_parameters(4);
    global_addr_name(&param);
    printf("LOS Check ignored: addr = %d, file = %d, name = %d, global = %d\n", new_name.addr,
        new_name.file, new_name.name, new_name.global);
    //set_los_order(DSL_IN_LOS, new_name, param.val[3]);
    command_implemented = 1;
}

static void move_dsl_ptr(uint16_t dest) {
    uint8_t *start = get_data_start_ptr();
    set_data_ptr(start, start + dest);
}

void dsl_zero(void) {
    command_implemented = 0;
}

void dsl_long_divide_equal(void) {
    command_implemented = 0;
}

void dsl_byte_dec(void) {
    command_implemented = 0;
}

void dsl_word_dec(void) {
    command_implemented = 0;
}

void dsl_long_dec(void) {
    command_implemented = 0;
}

void dsl_byte_inc(void) {
    command_implemented = 0;
}

void dsl_word_inc(void) {
    command_implemented = 0;
}

void dsl_long_inc(void) {
    command_implemented = 0;
}

void dsl_hunt(void) {
    command_implemented = 0;
}

void dsl_getxy(void) {
    command_implemented = 0;
}

void dsl_string_copy(void) {
    get_parameters(2);
    printf("string in MAS = %s\n", (char*)param.ptr[1]);
    strcpy((char*)param.ptr[0], (char*)param.ptr[1]);
}

void dsl_p_damage(void) {
    command_implemented = 0;
}

void dsl_changemoney(void) {
    command_implemented = 0;
}

void dsl_setvar(void) {
    command_implemented = 0;
}

void dsl_toggle_accum(void) {
    command_implemented = 0;
}

void dsl_getstatus(void) {
    command_implemented = 0;
}

void dsl_getlos(void) {
    command_implemented = 0;
}

void dsl_long_times_equal(void) {
    command_implemented = 0;
}

void dsl_jump(void) {
    command_implemented = 0;
}

void dsl_local_sub(void) {
    command_implemented = 0;
}

void dsl_global_sub(void) {
    command_implemented = 0;
}

void dsl_local_ret(void) {
    command_implemented = 0;
}

void dsl_load_variable(void) {
    dsl_load_accum();
    load_variable();
    command_implemented = 0;
}

void dsl_compare(void) {
    command_implemented = 0;
}

void dsl_load_accum(void) {
    set_accumulator(read_number());
}


void dsl_global_ret(void) {
    command_implemented = 0;
}

void dsl_nextto(void) {
    command_implemented = 0;
}

void dsl_inloscheck(void) {
    get_parameters(4);
    global_addr_name(&param);
    //set_low_order(IN_LOS, new_name, param.val[3]);
}

void dsl_notinloscheck(void) {
    command_implemented = 0;
}

void dsl_clear_los(void) {
    command_implemented = 0;
}

void dsl_nametonum(void) {
    command_implemented = 0;
}

void dsl_numtoname(void) {
    command_implemented = 0;
}

void dsl_bitsnoop(void) {
    command_implemented = 0;
}

void dsl_award(void) {
    command_implemented = 0;
}

void dsl_request(void) {
    command_implemented = 0;
}

void dsl_source_trace(void) {
    command_implemented = 0;
}

void dsl_shop(void) {
    command_implemented = 0;
}

void dsl_clone(void) {
    command_implemented = 0;
}

void dsl_default(void) {
    command_implemented = 0;
}

void dsl_ifis(void) {
    command_implemented = 0;
}

void dsl_trace_var(void) {
    command_implemented = 0;
}

void dsl_orelse(void) {
    command_implemented = 0;
}

void dsl_clearpic(void) {
    command_implemented = 0;
}

void dsl_continue(void) {
    command_implemented = 0;
}

void dsl_log(void) {
    command_implemented = 0;
}

void dsl_damage(void) {
    command_implemented = 0;
}

void dsl_source_line_num(void) {
    command_implemented = 0;
}

void dsl_drop(void) {
    command_implemented = 0;
}

void dsl_passtime(void) {
    command_implemented = 0;
}

void dsl_exit_gpl(void) {
    command_implemented = 0;
}

void dsl_fetch(void) {
    command_implemented = 0;
}

void dsl_search(void) {
    command_implemented = 0;
}

void dsl_getparty(void) {
    command_implemented = 0;
}

void dsl_fight(void) {
    command_implemented = 0;
}

void dsl_flee(void) {
    command_implemented = 0;
}

void dsl_follow(void) {
    command_implemented = 0;
}

void dsl_getyn(void) {
    command_implemented = 0;
}

void dsl_give(void) {
    command_implemented = 0;
}

void dsl_go(void) {
    command_implemented = 0;
}

void dsl_input_bignum(void) {
    command_implemented = 0;
}

void dsl_goxy(void) {
    command_implemented = 0;
}

void dsl_readorders(void) {
    command_implemented = 0;
}

void dsl_if(void) {
    get_parameters(1);
    ifptr++;
    if (ifptr > MAX_IFDEPTH) {
        fprintf(stderr, "ERROR: to many nested if statement in GPL!!!\n");
        exit(1);
    }
    ifstate[ifptr] = get_accumulator();
    if (get_accumulator() == NO) {
        move_dsl_ptr(param.val[0]);
    }
}

void dsl_else(void) {
    command_implemented = 0;
}

void dsl_setrecord(void) {
    command_implemented = 0;
}

void dsl_setother(void) {
    command_implemented = 0;
}

void dsl_input_string(void) {
    command_implemented = 0;
}

void dsl_input_number(void) {
    command_implemented = 0;
}

void dsl_input_money(void) {
    command_implemented = 0;
}

void dsl_joinparty(void) {
    command_implemented = 0;
}

void dsl_leaveparty(void) {
    command_implemented = 0;
}

void dsl_lockdoor(void) {
    command_implemented = 0;
}

void dsl_menu(void) {
    command_implemented = 0;
}

void dsl_setthing(void) {
    command_implemented = 0;
}

void dsl_local_sub_trace(void) {
    command_implemented = 0;
}

void dsl_print_string(void) {
    command_implemented = 0;
}

void dsl_print_number(void) {
    command_implemented = 0;
}

void dsl_printnl(void) {
    command_implemented = 0;
}

void dsl_rand(void) {
    command_implemented = 0;
}

void dsl_showpic(void) {
    command_implemented = 0;
}

void dsl_skillroll(void) {
    command_implemented = 0;
}

void dsl_statroll(void) {
    command_implemented = 0;
}

void dsl_string_compare(void) {
    command_implemented = 0;
}

void dsl_match_string(void) {
    command_implemented = 0;
}

void dsl_take(void) {
    command_implemented = 0;
}

void dsl_sound(void) {
    command_implemented = 0;
}

void dsl_tport(void) {
    command_implemented = 0;
}

void dsl_music(void) {
    command_implemented = 0;
}

void dsl_cmpend(void) {
    command_implemented = 0;
}

void dsl_wait(void) {
    command_implemented = 0;
}

void dsl_while(void) {
    command_implemented = 0;
}

void dsl_wend(void) {
    command_implemented = 0;
}

void dsl_attackcheck(void) {
    command_implemented = 0;
}

void dsl_lookcheck(void) {
    command_implemented = 0;
}

void dsl_endif(void) {
    if (--ifptr < 0) {
        fprintf(stderr, "ERROR: a negativly nested if statement in GPL.\n");
        exit(1);
    }
}

void dsl_move_tilecheck(void) {
    command_implemented = 0;
}

void dsl_door_tilecheck(void) {
    command_implemented = 0;
}

void dsl_move_boxcheck(void) {
    command_implemented = 0;
}

void dsl_door_boxcheck(void) {
    command_implemented = 0;
}

void dsl_pickup_itemcheck(void) {
    command_implemented = 0;
}

void dsl_usecheck(void) {
    command_implemented = 0;
}

void dsl_talktocheck(void) {
    get_parameters(3);
    global_addr_name(&param);
    generic_name_check(&gtalk_to, new_name);
    command_implemented = 0;
}

void dsl_noorderscheck(void) {
    command_implemented = 0;
}

void dsl_usewithcheck(void) {
    command_implemented = 0;
}

void dsl_byte_plus_equal(void) {
    command_implemented = 0;
}

void dsl_byte_minus_equal(void) {
    command_implemented = 0;
}

void dsl_byte_times_equal(void) {
    command_implemented = 0;
}

void dsl_byte_divide_equal(void) {
    command_implemented = 0;
}

void dsl_word_plus_equal(void) {
    command_implemented = 0;
}

void dsl_word_minus_equal(void) {
    command_implemented = 0;
}

void dsl_word_times_equal(void) {
    command_implemented = 0;
}

void dsl_word_divide_equal(void) {
    command_implemented = 0;
}

void dsl_long_plus_equal(void) {
    command_implemented = 0;
}

void dsl_long_minus_equal(void) {
    command_implemented = 0;
}

void dsl_get_range(void) {
    command_implemented = 0;
}
