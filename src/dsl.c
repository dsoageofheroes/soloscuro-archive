#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "dsl.h"
#include "dsl-object.h"
#include "dsl-narrate.h"
#include "dsl-scmd.h"
#include "dsl-string.h"
#include "dsl-var.h"

#define DSL_ORDER     (0)
#define DSL_LOS_ORDER (1)

#define DSL_IN_LOS        (1)
#define DSL_NOT_IN_LOS    (2)

#define IS_POV   (0)
#define IS_PARTY (1)
#define IS_NPC (1)

#define CHOSEN (0x7FFD)
#define PARTY  (0x7FFE)
#define ALL    (0x7FFF)

static int take_while = 0; // Debugging variable for forcing a while loop to be taken!

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
void dsl_exit_dsl(void);
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

#define DSL_MAX_COMMANDS (0x81)

#define check_index_t uint16_t
#define NULL_CHECK (0xFFFF)
typedef struct _dsl_check_s {
    union {
        box_t box_check;
        tile_t tile_check;
        name_t name_check;
        name2_t name2_check;
    } data;
    uint16_t next;
} dsl_check_t;
static dsl_check_t *check;
static check_index_t gunused_checks;
static check_index_t gtalk_to = NULL_CHECK;
static check_index_t gAttack = NULL_CHECK;
static check_index_t gPov = NULL_CHECK;
static check_index_t gPickup = NULL_CHECK;
static check_index_t gOther1 = NULL_CHECK;
static check_index_t gMoveTile = NULL_CHECK;
static check_index_t gLook = NULL_CHECK;
static check_index_t gUse = NULL_CHECK;
static check_index_t gUseWith = NULL_CHECK;
static check_index_t gMoveBox = NULL_CHECK;

static uint8_t exit_dsl = 1; // Tell use when the stop reading the DSL.

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
    { dsl_exit_dsl, "dsl exit dsl" }, // 0x31
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
name2_t new_name2;
typedef enum {DSL_ERROR = -1, DSL_OK = 1} dsl_status_t;
dsl_status_t dsl_status = DSL_ERROR;

uint16_t this_gpl_file = 0;
uint16_t this_gpl_type = 0;

enum {NOFILE, DSLFILE, MASFILE};

#define GLOBAL_MAS (99)

param_t param;
/* End Globals */

void do_dsl_command(uint8_t cmd) {
    printf("command byte = 0x%x", cmd);
    fflush(stdout);
    printf("(%s)\n", dsl_operations[cmd].name);
    (*dsl_operations[cmd].func)();
}

void mas_print(const int gff_file, const int res_id) {
    unsigned long len;
    unsigned char *mas = (unsigned char*)gff_get_raw_bytes(gff_file, GT_MAS, res_id, &len);
    this_gpl_file = GLOBAL_MAS;
    this_gpl_type = MASFILE;
    command_implemented = 1;
    exit_dsl = 0;
    printf("------------------------Executing MAS/DSL\n");
    push_data_ptr(mas);
    set_data_ptr(mas, mas + 1);
    printf("command byte = 0x%x (%s)\n", *(mas), dsl_operations[mas[0]].name);
    (*dsl_operations[mas[0]].func)();
    while (!exit_dsl && command_implemented) {
        command_implemented = 1;
        uint8_t command = get_byte();
        do_dsl_command(command);
        if (exit_dsl) {
            exit_dsl = pop_data_ptr() == NULL;
            printf("returing from subroutine.\n");
        }
    }
    if (!command_implemented) {
        printf("last command needs to be implemented!\n");
    }
    pop_data_ptr();
    printf("---------------------Ending Execution----------------------\n");
    printf("%lu of %lu\n", get_data_ptr() - get_data_start_ptr(), len);
}

void dsl_print(const int gff_file, const int res_id) {
    unsigned long len;
    unsigned char *gpl = (unsigned char*)gff_get_raw_bytes(gff_file, GT_GPL, res_id, &len);
    this_gpl_file = GLOBAL_MAS;
    this_gpl_type = MASFILE;
    command_implemented = 1;
    exit_dsl = 0;
    printf("------------------------Executing MAS/DSL\n");
    push_data_ptr(gpl);
    set_data_ptr(gpl, gpl + 1);
    printf("command byte = 0x%x (%s)\n", *(gpl), dsl_operations[gpl[0]].name);
    (*dsl_operations[gpl[0]].func)();
    int i = 0;
    while ((get_data_ptr() - get_data_start_ptr()) < len && command_implemented) {
    //for (int i = 0; i < 2; i++) {
        while (!exit_dsl && command_implemented) {
            command_implemented = 1;
            uint8_t command = get_byte();
            do_dsl_command(command);
            if (exit_dsl) {
                exit_dsl = pop_data_ptr() == NULL;
                printf("returing from subroutine.\n");
            }
        }
        if (!command_implemented) {
            printf("last command needs to be implemented!\n");
        }
        printf("End of %dth exit_dsl, continuing on...\n", i++);
        exit_dsl = 0;
    }
    pop_data_ptr();
    printf("---------------------Ending Execution----------------------\n");
    printf("%lu of %lu\n", get_data_ptr() - get_data_start_ptr(), len);
}

static void add_save_orders(int16_t los_order, name_t name, int16_t range, int ordertype) {
    if (name.name < 0) {
        printf("add_save_orders (with name < 0) not implemented\n");
        command_implemented = 0;
    }
}

static void set_any_order(name_t *name, int16_t to, int16_t los_order, int16_t range) {
    printf("set_any_order: lua callback needed: Get all objects with 'name' and set then to to with los_order and in range\n");
}

void set_los_order(int16_t los_order, name_t name, int16_t range) {
    add_save_orders(los_order, name, range, DSL_LOS_ORDER);
    set_any_order(&name, DSL_LOS_ORDER, los_order, range);
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

/*
static int load_dsl(uint16_t filenum, int filetype) {
    unsigned long len;
    char *data = gff_get_raw_bytes(DSLDATA_GFF_INDEX, 
        (filetype == DSLFILE) ? GT_GPL : GT_MAS,
        filenum, &len);
    if (!data) { return 0; }
    clear_local_vars();
    push_data_ptr((unsigned char*) data);
    return 1;
}

static void local_sub(uint16_t address) {
    unsigned char *data_start = get_data_start_ptr();
    set_data_ptr(data_start, data_start + address);
}

static void global_sub(uint16_t filenum, uint16_t address, int filetype) {
    if (exit_dsl == 1) { return; }
    dsl_status = DSL_OK;

    if (load_dsl(filenum, filetype) == 0) {
        fprintf(stderr, "Unable to load region #%d\n", filenum);
        exit(1);
    }
    if (!exit_dsl) {
        local_sub(address);
    }
    printf("global_sub: jump to file #%d at address %d\n", filenum, address);
}
*/

static int fromgpl = 0;
#define FLEE       (1)
#define ATTACK     (2)
#define COVER      (4)
#define BERSERK    (7)
#define NONCOMBAT  (8)
#define COMBAT     (9)
#define FETCH      (10) // data1 = item ID
#define FOLLOW     (11) // data1 = character ID
#define GIVE       (12) // data1 = quantity, data2 = item ID, data3 = character ID
#define GO_OBJECT  (13) // data1 = object ID
#define GOXY       (15) // data1 = X, data2 = Y
#define TAKE       (16) // data1 = quantity, data2 = item id, data3 = character ID
#define WAIT       (17)
#define HUNT       (18)
#define GO_ATTACK  (19)

static void set_the_orders(int16_t header, int16_t order, int16_t data1, int16_t data2, int send) {
    if (order == GOXY && (!data1) && (!data2)) {
        printf("set_the_order: bug\n");
        command_implemented = 0;
    }
    switch(order) {
        case FLEE:
            printf("set_the_order: FLEE (need to call to lua.)\n");
            break;
        case ATTACK:
            printf("set_the_order: ATTACK (need to call to lua.)\n");
            break;
        case COVER:
            printf("set_the_order: COVER (need to call to lua.)\n");
            break;
        case BERSERK:
            printf("set_the_order: BERSERK (need to call to lua.)\n");
            break;
        case NONCOMBAT:
            printf("set_the_order: NONCOMBAT ???\n");
            break;
        case COMBAT:
            printf("set_the_order: COMBAT ???\n");
            break;
        case FETCH:
            printf("set_the_order: FETCH item #%d (need to call to lua.)\n", data1);
            break;
        case FOLLOW:
            printf("set_the_order: FOLLOW character #%d (need to call to lua.)\n", data1);
            break;
        case GIVE:
            printf("set_the_order: GIVE quantity: %d, item #%d (need to call to lua.)\n", data1, data2);
            break;
        case GO_OBJECT:
            printf("set_the_order: GO_OBJECT object #%d (need to call to lua.)\n", data1);
            break;
        case GOXY:
            printf("set_the_order: GO_XY (%d, %d) (need to call to lua.)\n", data1, data2);
            break;
        case TAKE:
            printf("set_the_order: TAKE quantity: %d, item #%d (need to call to lua.)\n", data1, data2);
            break;
        case WAIT:
            printf("set_the_order: WAIT (need to call to lua.)\n");
            break;
        case HUNT:
            printf("set_the_order: HUNT (need to call to lua.)\n");
            break;
        case GO_ATTACK:
            printf("set_the_order: GO_ATTACK (need to call to lua.)\n");
            break;
        default:
            printf("Unknown order: %d\n", order);
            command_implemented = 0;
        break;
    }
}

static void set_orders(int16_t header, int16_t order, int16_t data1, int16_t data2) {
    set_the_orders(header, order, data1, data2, 1);
}

void set_new_order(name_t name) {
    if (!name.file) {
        printf("name.file != NULL not implemented!\n");
        command_implemented = 0;
        if (name.name < 0) {
            printf("set_new_order name < 0 not implemented!\n");
            command_implemented = 0;
        }
    } else {
        add_save_orders(0, name, 0, DSL_ORDER);
    }
    set_any_order(&name, DSL_ORDER, 0, 0);
}

static int8_t comparePtr = 0;
#define MAX_COMPAREDEPTH (8)
int8_t compared[MAX_COMPAREDEPTH + 1];
int32_t *compareval = NULL;

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
    memset(check, 0x0, sizeof(dsl_check_t) * DSL_CHECKS);
    compareval = malloc(sizeof(int32_t) * MAX_COMPAREDEPTH);
    memset(compareval, 0x0, sizeof(int32_t) * MAX_COMPAREDEPTH);
    for (int i = 0; i < DSL_CHECKS; i++) {
        check[i].next = i + 1;
    }
    check[DSL_CHECKS - 1].next = NULL_CHECK;
}


void dsl_check_for_updates() {
    gff_file_t *dsl_file = open_files + DSLDATA_GFF_INDEX;
    //unsigned long len;
    if (dsl_file != NULL && dsl_file->filename != NULL) {
        /*
        printf("DSL file detected, loading game data.\n");
        //char *data = gff_get_raw_bytes(DSLDATA_GFF_INDEX, GT_MAS, GLOBAL_MAS, &len);
        char *data = gff_get_raw_bytes(DSLDATA_GFF_INDEX, GT_MAS, 42, &len);
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

static void use_with_check(check_index_t *cindex, name2_t name) {
    while ((*cindex != NULL_CHECK) && (check[*cindex].data.name2_check.name1 < name.name1 )) {
        use_with_check(&check[*cindex].next, name);
        return;
    }
    while ((*cindex != NULL_CHECK) && (check[*cindex].data.name2_check.name2 < name.name2 )) {
        use_with_check(&check[*cindex].next, name);
        return;
    }
    insert_check(cindex);
    if (*cindex == NULL_CHECK) {
        return;
    }
    check[*cindex].data.name2_check = name;
}

static void generic_box_check(check_index_t *cindex, box_t box) {
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
}

static void generic_tile_check(check_index_t *cindex, tile_t tile) {
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
}

static box_t get_box(param_t *p) {
    box_t ret;

    ret.x = p->val[0];
    ret.y = p->val[1];
    ret.xd = p->val[2];
    ret.yd = p->val[3];
    ret.addr = p->val[4];
    ret.file = p->val[5];
    ret.trip = p->val[6];

    return ret;
}

static tile_t get_tile(param_t *p) {
    tile_t ret;

    ret.x = p->val[0];
    ret.y = p->val[1];
    ret.addr = p->val[2];
    ret.file = p->val[3];
    ret.trip = p->val[4];

    return ret;
}

/* All those commands... */
static void global_addr_name(param_t *par) {
    new_name.addr = par->val[0];
    new_name.file = par->val[1];
    new_name.name = par->val[2];
    new_name.global = 0;

    if (this_gpl_file == GLOBAL_MAS) {
        if (this_gpl_type == MASFILE) {
            //printf("GLOBAL!!!!!!!!!!!!!\n");
            new_name.global = 1;
        }
    }
}

static void name_name_global_addr(param_t *par) {
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

void dsl_in_los_check(void) {
    get_parameters(4);
    global_addr_name(&param);
    printf("LOS Check ignored: addr = %d, file = %d, name = %d, global = %d\n", new_name.addr,
        new_name.file, new_name.name, new_name.global);
    set_los_order(DSL_IN_LOS, new_name, param.val[3]);
}

static void move_dsl_ptr(uint16_t dest) {
    uint8_t *start = get_data_start_ptr();
    set_data_ptr(start, start + dest);
}

void dsl_zero(void) {
    command_implemented = 0;
}

void dsl_long_divide_equal(void) {
    get_parameters(2);
    *param.ptr[0] /= param.val[1];
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
    get_parameters(1);
    (*((uint16_t*)param.ptr[0]))++;
}

void dsl_long_inc(void) {
    command_implemented = 0;
}

void dsl_hunt(void) {
    set_orders(read_number(), HUNT, gPov, 0);
}

int32_t data_field(int16_t header, uint16_t fi) {
    printf("I need to get the %dth field of object_header[%d]\n", fi, header);
    return 1;
}

/* Find an object! */
typedef struct _object_header_t {
    uint8_t rec_type;
    int16_t data;
} object_header_t;
#define NOBODY NULL_OBJECT
object_header_t object_header[MAX_OBJECTS + 200 + 5];
int16_t match_header(int16_t *num, int16_t header) {
    int16_t i, object = NULL_OBJECT;
    
    if (header >=0 || header == NOBODY) { // Call with a header...
        object = header;
        *num = MAX_OBJECTS;
    } else if (header > NULL_OBJECT) { // ALL/Party called (wierd)
        object = NULL_OBJECT;
        *num = MAX_OBJECTS;
    } else {
        for (i = *num; i < MAX_OBJECTS; i++) {
            if (object_header[i].rec_type != 0 && header == (int)data_field(i, DSL_OBJECT_ID)) {
                object = i;
                *num = i + 1;
                i = MAX_OBJECTS;
            }
        }
    }

    if (object == NULL_OBJECT) {*num = NULL_OBJECT;}

    return object;
}

uint16_t real_x(int16_t ctrl) {
    printf("Need to get the x coordinate of object %d\n", ctrl);
    return 0;
}

uint16_t real_y(int16_t ctrl) {
    printf("Need to get the y coordinate of object %d\n", ctrl);
    return 0;
}

uint16_t real_z(int16_t ctrl) {
    printf("Need to get the z coordinate of object %d\n", ctrl);
    return 0;
}

uint16_t dslX, dslY, dslZ;
static void get_xyz(int16_t id) {
    int16_t ctrl, num = 0;
    do{
        if ((ctrl = match_header(&num, id)) != NULL_OBJECT) {
            dslX = real_x(ctrl);
            dslY = real_y(ctrl);
            dslZ = 0;
        }
    }while(num < MAX_OBJECTS);
}

void dsl_getxy(void) {
    get_xyz(read_number());
}

void dsl_string_copy(void) {
    get_parameters(2);
    printf("string in MAS = %s\n", (char*)param.ptr[1]);
    strcpy((char*)param.ptr[0], (char*)param.ptr[1]);
}

void do_damage(int is_percent) {
    get_parameters(2);
    switch (param.val[0]) {
        case ALL:
            fprintf(stderr, "illegal parameter ALL to do_damage!\n");
            exit(1);
            break;
        case PARTY:
            if (is_percent) {
                printf("need to do <kill-damage> of %d%% to all party members\n", param.val[1]);
            } else {
                printf("need to do <kill-damage> of amt %d to all party members\n", param.val[1]);
            }
            break;
        default:
            if (param.val[0] >= 0 ) {
                if (is_percent) {
                    printf("need to do <kill-damage> of %d%% to %d.\n", param.val[1], param.val[0]);
                } else {
                    printf("need to do <kill-damage> of amt %d to %d.\n", param.val[1], param.val[0]);
                }
            } else {
                if (is_percent) {
                    printf("need to do <kill-damage> of %d%% to all objects with id %d.\n", param.val[1], param.val[0]);
                } else {
                    printf("need to do <kill-damage> of amt %d to all objects with id %d.\n", param.val[1], param.val[0]);
                }
            }
            break;
    }
}

void dsl_p_damage(void) {
    do_damage(1);
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

void local_sub(uint16_t offset) {
    printf("local_sub: I need to jump to offset %d of this GPL!\n", offset);
}

void dsl_local_sub(void) {
    local_sub(read_number());
}

void dsl_global_sub(void) {
    get_parameters(2);
//    global_sub(param.val[1], param.val[0], DSLFILE);
    printf("global subs temporarly disabled.\n");
}

void dsl_local_ret(void) {
    printf("dsl_local_ret: need to return from local subroutine.\n");
}

void dsl_load_variable(void) {
    dsl_load_accum();
    load_variable();
}

void dsl_compare(void) {
    dsl_load_accum();
    comparePtr++;
    if (comparePtr >= MAX_COMPAREDEPTH) {
        fprintf(stderr, "MAX COMPARE DEPTH!\n");
        exit(1);
    }
    compared[comparePtr] = NO;
    compareval[comparePtr] = get_accumulator();
}

void dsl_load_accum(void) {
    set_accumulator(read_number());
}

static void global_ret() {
    printf("I need to return from this global function/subroutine!\n");
}

void dsl_global_ret(void) {
    global_ret();
}

void dsl_nextto(void) {
    command_implemented = 0;
}

void dsl_inloscheck(void) {
    get_parameters(4);
    global_addr_name(&param);
    set_los_order(DSL_IN_LOS, new_name, param.val[3]);
}

void dsl_notinloscheck(void) {
    get_parameters(4);
    global_addr_name(&param);
    set_los_order(DSL_NOT_IN_LOS, new_name, param.val[3]);
}

static void clear_los_order(int16_t header_num) {
    printf("clear_los_order: need to clear #%d\n", header_num);
}

void dsl_clear_los(void) {
    clear_los_order(read_number());
}

void dsl_nametonum(void) {
    set_accumulator(read_number() * -1);
}

void dsl_numtoname(void) {
    set_accumulator(read_number() * -1);
}

void dsl_bitsnoop(void) {
    command_implemented = 0;
}

void dsl_award(void) {
    command_implemented = 0;
}

void dsl_request(void) {
    get_parameters(4);
    set_accumulator(dsl_request_impl(param.val[0], param.val[1], param.val[2], param.val[3]));
}

void dsl_source_trace(void) {
    command_implemented = 0;
}

void dsl_shop(void) {
    command_implemented = 0;
}

void dsl_clone(void) {
    get_parameters(6);
    for (int i = 0; i < 6; i++) {
        printf("param.val[%d] = %d\n", i, param.val[i]);
    }
    int16_t header_num = param.val[0];
    int16_t qty = param.val[1];
    int16_t x = param.val[2];
    int16_t y = param.val[3];
    uint8_t priority = param.val[4];
    uint8_t placement = param.val[5];
    printf("dsl_clone: must create %d of %d objects at (%d, %d) with priority = %d, placement = %d\n",
        header_num, qty, x, y, priority, placement);
    //command_implemented = 0;
}

void dsl_default(void) {
    command_implemented = 0;
}

void dsl_ifis(void) {
    get_parameters(2);
    if ((int32_t) compareval[comparePtr] != (int32_t)param.val[0]) {
        printf("dsl_ifis: taking ifis.\n");
        move_dsl_ptr(param.val[1]);
    } else {
        compared[comparePtr] = YES;
    }
}

void dsl_trace_var(void) {
    command_implemented = 0;
}

void dsl_orelse(void) {
    int16_t address = read_number();
    if (compared[comparePtr] == YES) {
        move_dsl_ptr(address);
    }
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
    do_damage(0);
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

void dsl_exit_dsl(void) {
    exit_dsl = 1; // We are done!
}

void dsl_fetch(void) {
    command_implemented = 0;
}

#define OBJ_QUALIFIER  (0x53)
#define OBJ_INTRODUCE  (0x01)

#define LOW_SEARCH     (0)
#define HIGH_SEARCH    (1)
#define TOTAL_SEARCH   (2)
#define COUNT_SEARCH   (3)
#define EQU_SEARCH     (4)
#define LT_SEARCH      (5)
#define GT_SEARCH      (6)

int32_t search() {
    uint32_t answer = 0L;
    int16_t object = read_number();
    int16_t low_field = get_byte();
    int16_t high_field = get_byte();
    int16_t field_level = -1, depth = 1;
    int32_t search_for = 0, temp_for = 0;
    uint16_t field[16];
    uint8_t type = 0;
    int16_t i;
    do {
        if (peek_one_byte() == OBJ_QUALIFIER) {
            get_byte();
        }
        field_level++;
        field[field_level] = get_byte();
        type = get_byte();
        if (type >= EQU_SEARCH && type <= GT_SEARCH) {
            temp_for = read_number();
            if (field_level > 0) {
                field[++field_level] = (uint16_t) temp_for;
            } else {
                search_for = temp_for;
            }
        } else {
            printf("search: unknown type = %d\n", type);
            command_implemented = 0;
            return 0;
        }
        depth--;
    } while ( peek_one_byte() == OBJ_QUALIFIER);

    i = (object == PARTY || object < 0)
        ? NULL_OBJECT : object;

    /*
    do {
        if (type == LOW_SEARCH) {
            answer = 0x7FFFFFFFL;
        }
    } while ( (object == PARTY) && (i != NULL_OBJECT));
    */
    printf("I need to find object %d, i = %d, low_field = %d, high_field = %d\n",
        object, i, low_field, high_field);
    printf("Field parameters:");
    for (int idx = 0; idx < depth; idx++) {
        printf("%d ", field[idx]);
    }
    printf("\n");
    printf("search_for = %d\n", search_for);

    printf("search command to be implemented later...\n");
    return answer;
}

void dsl_search(void) {
    set_accumulator(search());
}

void dsl_getparty(void) {
    command_implemented = 0;
}

void dsl_fight(void) {
    command_implemented = 0;
}

void dsl_flee(void) {
    set_orders(read_number(), FLEE, 0, 0);
}

void dsl_follow(void) {
    get_parameters(2);
    fromgpl = 1;
    set_orders(param.val[1], FOLLOW, param.val[0], 0);
    fromgpl = 0;
}

void dsl_getyn(void) {
    command_implemented = 0;
}

void dsl_give(void) {
    command_implemented = 0;
}

void dsl_go(void) {
    get_parameters(2);
    set_orders(param.val[1], GO_OBJECT, param.val[0], 0);
}

void dsl_input_bignum(void) {
    command_implemented = 0;
}

void dsl_goxy(void) {
    get_parameters(3);
    set_orders(param.val[2], GOXY, param.val[0], param.val[1]);
}

void dsl_readorders(void) {
    command_implemented = 0;
}

void dsl_if(void) {
    get_parameters(1);
    ifptr++;
    if (ifptr > MAX_IFDEPTH) {
        fprintf(stderr, "ERROR: to many nested if statements in DSL!!!\n");
        exit(1);
    }
    ifstate[ifptr] = get_accumulator();
    if (get_accumulator() == NO) {
        move_dsl_ptr(param.val[0]);
        /*
        printf("************************************\n");
        for (int i = 0; i < 16; i++) {
        printf("TODO FIXME: RE-ENABLE IFs YOU DUMMY!\n");
        }
        printf("************************************\n");
        */
    }
}

void dsl_else(void) {
    get_parameters(1);
    if (ifstate[ifptr] == 0) {
        // Don't execute the else if the IF part was taken.
        move_dsl_ptr(param.val[0]);
    }
}

void dsl_setrecord(void) {
    command_implemented = 0;
}

static int32_t id_to_header(int32_t header) {
    // guess: is this to find the actual object header?  I'm just return 1234 for now...
    printf("id_to_header: guessing 1234\n");
    return 1234;
}
void dsl_setother(void) {
    int32_t header = 0;
    if ((( header = read_number()) >= 0 && header != NULL_OBJECT)
        || (header = id_to_header(header)) != NULL_OBJECT) {
        gOther1 = header;
        set_accumulator(1);
    } else {
        set_accumulator(0);
    }
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

#define SOURCE_TRACE   (0x23)
#define VARNAME_TRACE  (0x28)
#define LINENUM_TRACE  (0x2E)
#define LOCALSUB_TRACE (0x4B)

void trace_protect() {
    uint8_t inst;

    inst = peek_one_byte();
    while ((inst == VARNAME_TRACE)
        || (inst == SOURCE_TRACE)
        || (inst == LINENUM_TRACE)
        || (inst == LOCALSUB_TRACE)
        ) {
        printf("*********NEED TO COMPLETE THE trace_protect FUNCTION!!!!\n");
        command_implemented = 0;
        return;
    }
}

uint16_t menu_functions[12];
uint8_t menu_bytes[12];
#define MAXMENU   (24)

void display_menu() {
    int items = 0;
    trace_protect();
    read_number();
    char *menu = (char*) gBignumptr;
    narrate_open(NAR_ADD_MENU, menu, 0);
    while ((peek_one_byte() != 0x4A) && (items <= MAXMENU)) {
        trace_protect();
        read_number();
        menu = (char*) gBignumptr;
        menu_functions[items] = read_number();
        menu_bytes[items] = (uint8_t) read_number();
        if (menu_bytes[items] == 1) {
            narrate_open(NAR_ADD_MENU, menu, items + 1);
            items++;
        } else {
            printf("Not available at this time: '%s'\n", menu);
        }
        trace_protect();
    }
    get_byte();  // get rid of the mend...
    printf("Now wait for user input.\n");
    for (int i = 0; i < items; i++) {
        printf("choice %d: goes to local subroutine %d\n", i, menu_functions[i]);
    }
    //printf("Need to implement display_menu()\n");
    //command_implemented = 0;
}

void dsl_menu(void) {
    display_menu();
}

void dsl_setthing(void) {
    command_implemented = 0;
}

void dsl_local_sub_trace(void) {
    command_implemented = 0;
}

void dsl_print_string(void) {
    get_parameters(2);
    printf("param.ptr[1] = %p\n", param.ptr[1]);
    narrate_open(NAR_SHOW_TEXT, (char *)param.ptr[1], param.val[0]);
    printf("Now I need to wait for the dialog to close.\n");
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

void show(int16_t portrait_index) {
    narrate_open(NAR_PORTRAIT, NULL, portrait_index);
}

void dsl_showpic(void) {
    show(read_number());
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

static int16_t take(int16_t qty, int16_t item, int16_t giver, int16_t receiver) {
    printf("I need to take %d of type %d from %d to %d\n", qty, item, giver, receiver);
    return qty;
}

static int16_t grafted(int16_t global_idx, int16_t id) {
    printf("I need to check if PC %d's item %d is grafted\n", global_idx, id);
    return 0;
}

void dsl_take(void) {
    get_parameters(4);
    if (param.val[2] == PARTY) {
        set_accumulator(0);
        // If item is not grafted to character then:
        int party_character = 0;
        // for each party_chacter....
        if (!grafted(party_character, param.val[1])) {
            set_accumulator(get_accumulator() + take(param.val[0], param.val[1], party_character, param.val[3]));
        }
    } else {
        if (!grafted(param.val[2], param.val[1])) {
            set_accumulator(take(param.val[0], param.val[1], param.val[2], param.val[3]));
        }
    }
}

void dsl_sound(void) {
    command_implemented = 0;
}

void dsl_tport(void) {
    get_parameters(5);
    if ((param.val[0] != PARTY) && (param.val[0] != IS_POV)) {
        printf("I need to teleport everything to region %d at (%d, %d) priority: %d, onscreen %d\n",
            param.val[1], param.val[2], param.val[3], param.val[4], param.val[5]);
    } else {
        printf("I need to teleport party to region %d at (%d, %d) priority: %d, onscreen %d\n",
            param.val[1], param.val[2], param.val[3], param.val[4], param.val[5]);
    }
}

void dsl_music(void) {
    command_implemented = 0;
}

void dsl_cmpend(void) {
    comparePtr--;
    if (comparePtr < 0) {
        fprintf(stderr, "dsl_cmpend: ERROR: comparePtr < 0!\n");
        exit(1);
    }
}

void dsl_wait(void) {
    command_implemented = 0;
}

void dsl_while(void) {
    get_parameters(1);
    if (get_accumulator() == NO) {
        printf("taking while loop...\n");
        move_dsl_ptr(param.val[0]);
    }
    if (take_while) {
        printf("dsl_while: debug: Taking while loop on second pass...\n");
        take_while = 0;
        move_dsl_ptr(param.val[0]);
    }
}

void dsl_wend(void) {
    take_while = 1; // TODO FIXME: take this out when done implementing!
    move_dsl_ptr(read_number());
}

void dsl_attackcheck(void) {
    get_parameters(3);
    global_addr_name(&param);
    generic_name_check(&gAttack, new_name);
}

void dsl_lookcheck(void) {
    get_parameters(3);
    global_addr_name(&param);
    generic_name_check(&gLook, new_name);
}

void dsl_endif(void) {
    if (--ifptr < 0) {
        fprintf(stderr, "ERROR: a negativly nested if statement in DSL.\n");
        exit(1);
    }
}

void dsl_move_tilecheck(void) {
    get_parameters(5);
    generic_tile_check(&gMoveTile, get_tile(&param));
}

void dsl_door_tilecheck(void) {
    command_implemented = 0;
}

void dsl_move_boxcheck(void) {
    get_parameters(7);
    generic_box_check(&gMoveBox, get_box(&param));
}

void dsl_door_boxcheck(void) {
    command_implemented = 0;
}

void dsl_pickup_itemcheck(void) {
    get_parameters(3);
    global_addr_name(&param);
    generic_name_check(&gPickup, new_name);
}

void dsl_usecheck(void) {
    get_parameters(3);
    global_addr_name(&param);
    generic_name_check(&gUse, new_name);
}

void dsl_talktocheck(void) {
    get_parameters(3);
    global_addr_name(&param);
    generic_name_check(&gtalk_to, new_name);
}

void dsl_noorderscheck(void) {
    get_parameters(3);
    global_addr_name(&param);
    set_new_order(new_name);
}

void dsl_usewithcheck(void) {
    get_parameters(4);
    name_name_global_addr(&param);
    use_with_check(&gUseWith, new_name2);
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

uint16_t range(int16_t obj0, int16_t obj1) {
    printf("Must compute range from %d to %d\n", obj0, obj1);
    return 10;//Totally bogus
}

void dsl_get_range(void) {
    get_parameters(2);
    set_accumulator(range(param.val[0], param.val[1]));
}
