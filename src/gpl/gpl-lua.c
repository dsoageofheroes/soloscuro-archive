#include "gpl-lua.h"
#include "gpl-state.h"
#include "gpl-string.h"
#include "gpl-var.h"
#include "gpl-manager.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define BUF_SIZE (1<<12)

static int print_cmd();
static int varnum = 0;
static int funcnum = 0;
static int in_func = 0;
static int in_compare = 0;
static size_t cfunc_num = 0;
static char is_master_mas = 0;
static void gpl_lua_load_variable();
static void gpl_lua_load_simple_variable(uint16_t type, uint16_t vnum);
static int gpl_lua_read_simple_num_var(char *buf, const size_t buf_size);
static uint8_t gpl_lua_access_complex(int16_t *header, uint16_t *depth, uint16_t *element, int32_t *obj_name);
static int32_t gpl_lua_read_complex(char *buf, size_t *buf_pos, const size_t size);
static uint16_t gpl_lua_get_word();
static void validate_number(const char *num, const char *message);
static void print_label();

extern void gpl_lua_byte_dec(void);
extern void gpl_lua_word_dec(void);
extern void gpl_lua_long_dec(void);
extern void gpl_lua_byte_inc(void);
extern void gpl_lua_word_inc(void);
extern void gpl_lua_long_inc(void);
extern void gpl_lua_changemoney(void);
extern void gpl_lua_setvar(void);
extern void gpl_lua_toggle_accum(void);
extern void gpl_lua_getstatus(void);
extern void gpl_lua_getlos(void);
extern void gpl_lua_long_times_equal(void);
extern void gpl_lua_jump(void);
extern void gpl_lua_local_sub(void);
extern void gpl_lua_global_sub(void);
extern void gpl_lua_local_ret(void);
extern void gpl_lua_compare(void);
extern void gpl_lua_pdamage(void);
extern void gpl_lua_cmpend(void);
extern void gpl_lua_wait(void);
extern void gpl_lua_while(void);
extern void gpl_lua_wend(void);
extern void gpl_lua_attacktrigger(void);
extern void gpl_lua_looktrigger(void);
extern void gpl_lua_load_accum(void);
extern void gpl_lua_global_ret(void);
extern void gpl_lua_usetrigger(void);
extern void gpl_lua_ifcompare(void);
extern void gpl_lua_clearpic(void);
extern void gpl_lua_orelse(void);
extern void gpl_lua_exit(void);
extern void gpl_lua_fetch(void);
extern void gpl_lua_search(void);
extern void gpl_lua_getparty(void);
extern void gpl_lua_fight(void);
extern void gpl_lua_flee(void);
extern void gpl_lua_follow(void);
extern void gpl_lua_getyn(void);
extern void gpl_lua_give(void);
extern void gpl_lua_go(void);
extern void gpl_lua_input_bignum(void);
extern void gpl_lua_goxy(void);
extern void gpl_lua_readorders(void);
extern void gpl_lua_if(void);
extern void gpl_lua_else(void);
extern void gpl_lua_setrecord(void);
extern void gpl_lua_setother(void);
extern void gpl_lua_menu(void);
extern void gpl_lua_setthing(void);
extern void gpl_lua_print_string(void);
extern void gpl_lua_print_number(void);
extern void gpl_lua_printnl(void);
extern void gpl_lua_rand(void);
extern void gpl_lua_showpic(void);
extern void gpl_lua_skillroll(void);
extern void gpl_lua_statroll(void);
extern void gpl_lua_string_compare(void);
extern void gpl_lua_match_string(void);
extern void gpl_lua_take(void);
extern void gpl_lua_sound(void);
extern void gpl_lua_music(void);
extern void gpl_lua_tport(void);
extern void gpl_lua_bitsnoop(void);
extern void gpl_lua_award(void);
extern void gpl_lua_request(void);
extern void gpl_lua_hunt(void);
extern void gpl_lua_source_trace(void);
extern void gpl_lua_shop(void);
extern void gpl_lua_end_control(void);
extern void gpl_lua_input_string(void);
extern void gpl_lua_input_number(void);
extern void gpl_lua_input_money(void);
extern void gpl_lua_joinparty(void);
extern void gpl_lua_leaveparty(void);
extern void gpl_lua_lockdoor(void);
extern void gpl_lua_nextto(void);
extern void gpl_lua_inlostrigger(void);
extern void gpl_lua_notinlostrigger(void);
extern void gpl_lua_move_tiletrigger(void);
extern void gpl_lua_continue(void);
extern void gpl_lua_log(void);
extern void gpl_lua_damage(void);
extern void gpl_lua_source_line_num(void);
extern void gpl_lua_drop(void);
extern void gpl_lua_passtime(void);
extern void gpl_lua_door_tiletrigger(void);
extern void gpl_lua_move_boxtrigger(void);
extern void gpl_lua_door_boxtrigger(void);
extern void gpl_lua_pickup_itemtrigger(void);
extern void gpl_lua_talktotrigger(void);
extern void gpl_lua_noorderstrigger(void);
extern void gpl_lua_usewithtrigger(void);
extern void gpl_lua_clear_los(void);
extern void gpl_lua_nametonum(void);
extern void gpl_lua_numtoname(void);
extern void gpl_lua_getxy(void);
extern void gpl_lua_clone(void);
extern void gpl_lua_word_plus_equal(void);
extern void gpl_lua_word_minus_equal(void);
extern void gpl_lua_word_times_equal(void);
extern void gpl_lua_word_divide_equal(void);
extern void gpl_lua_long_plus_equal(void);
extern void gpl_lua_long_minus_equal(void);
extern void gpl_lua_get_range(void);

static size_t gpl_lua_read_number(char *buf, const size_t size);
uint16_t get_half_word();

/********************DSL DEFINES ***************************************/
#define GPL_IN_LOS        (1)
#define GPL_NOT_IN_LOS    (2)

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

#define CHOSEN (0x7FFD)
#define PARTY  (0x7FFE)
#define ALL    (0x7FFF)

#define MAX_COMPAREDEPTH (8) // For Ifs...
#define MAX_IFDEPTH (32)
#define YES (1)
#define NO (0)

#define IS_POV   (0)
#define IS_PARTY (1)
#define IS_NPC (1)

// LUA manipulation variables & functions
#define NUM_PARAMS (8)
typedef struct params_s {
    char params[NUM_PARAMS][BUF_SIZE];
} params_t;
static params_t lparams;
//static char master_mas[LUA_MAX_SIZE];
//static char master_gpl[LUA_MAX_SIZE];
static char lua_buf[LUA_MAX_SIZE];
static size_t lua_pos = 0;
static int32_t lua_depth = 0; 
static size_t gpl_lua_start_ptr = 0;

//#define lprintf(...) lua_tab(lua_depth); lua_pos += snprintf(lua_buf + lua_pos, LUA_MAX_SIZE - lua_pos, __VA_ARGS__)
#define LUA_TAB_AMT (4)
static void lua_tab(const int amt) {
    for (int i = 0; i < amt && lua_pos < LUA_MAX_SIZE; i++) {
        for (int j = 0; j < LUA_TAB_AMT && lua_pos < LUA_MAX_SIZE; j++) {
            lua_buf[lua_pos++] = ' ';
        }
    }
    lua_buf[lua_pos >= LUA_MAX_SIZE ? LUA_MAX_SIZE - 1 : lua_pos] = '\0';
}

static void lprintf(const char *str, ...) {
    va_list args;
    va_start(args, str);
    lua_tab(lua_depth);
    lua_pos += vsnprintf(lua_buf + lua_pos, LUA_MAX_SIZE - lua_pos, str, args);
    va_end(args);
}

static size_t script_id;
static int is_mas;
static int collect_labels;

#define MAX_LABELS     (1<<12)
static uint32_t labels[MAX_LABELS];
static int label_pos;

static void lua_goto(const char *str) {
    int64_t num = strtol(str, NULL, 10);
    if (num == LONG_MIN || num == LONG_MAX) {
        error("Could not convert label: '%s' to long!\n", str);
        exit(1);
    }
    if (collect_labels == 1) {
        labels[label_pos++] = num;
    } else {
        lprintf("goto label%ld\n", num);
    }
}

static void lua_exit(const char *msg) {
    printf("lua code at error:\n%s\n", lua_buf);
    printf(msg);
    exit(1);
}

static int needs_quotes(const char *str) {
    if (strncmp("gpl.get_gstr(", str, 12) == 0) { return 0; }
    if (strncmp("gpl.get_element(", str, 15) == 0) { return 0; }
    return 1;
}

static void validate_number(const char *num, const char *message) {
    char msg[BUF_SIZE];
    const char *ptr = num;
    while (isdigit(*ptr) || *ptr == '-') {
        ptr++;
    }
    if (*ptr != '\0') {
        snprintf(msg, BUF_SIZE, "%s: '%s' is not a number.", message, num);
        lua_exit(msg);
    }
}
// END lua manipulation variabels and functions

static void gpl_lua_get_parameters(int16_t amt) {
    for (int16_t i = 0; i < amt; i++) {
        gpl_lua_read_number(lparams.params[i], BUF_SIZE);
        //printf("parameter[%d] = %s\n", i, lparams.params[i]);
    }
}

static void print_label() {
    if (collect_labels) { return; }

    uint64_t label = ((size_t)gpl_get_data_ptr()) - gpl_lua_start_ptr;
    for (int i = 0; i < label_pos; i++) {
        if (labels[i] == label) {
            lprintf("::label%lu::\n", label);
            break;
        }
    }
}

typedef struct {
    void (*func)(void);
    const char *name;
} gpl_lua_operation_t;

static void gpl_lua_string_copy();
static void gpl_lua_load_var();

gpl_lua_operation_t gpl_lua_operations[] = {
    { NULL, "gpl zero" }, // 0x0
    { NULL, "gpl long divide equal" }, // 0x1
    { gpl_lua_byte_dec, "gpl byte dec" }, // 0x2
    { gpl_lua_word_dec, "gpl word dec" }, // 0x3
    { gpl_lua_long_dec, "gpl long dec" }, // 0x4
    { gpl_lua_byte_inc, "gpl byte inc" }, // 0x5
    { gpl_lua_word_inc, "gpl word inc" }, // 0x6
    { gpl_lua_long_inc, "gpl long inc" }, // 0x7
    { gpl_lua_hunt, "gpl hunt" }, // 0x8
    { gpl_lua_getxy, "gpl getxy" }, // 0x9
    { gpl_lua_string_copy, "gpl string copy" }, // 0xA
    { gpl_lua_pdamage, "gpl p damage" }, // 0xB
    { gpl_lua_changemoney, "gpl changemoney" }, // 0xC
    { gpl_lua_setvar, "gpl setvar" }, // 0xD
    { gpl_lua_toggle_accum, "gpl toggle accum" }, // 0xE
    { gpl_lua_getstatus, "gpl getstatus" }, // 0xF
    { gpl_lua_getlos, "gpl getlos" }, // 0x10
    { gpl_lua_long_times_equal, "gpl long times equal" }, // 0x11
    { gpl_lua_jump, "gpl jump" }, // 0x12
    { gpl_lua_local_sub, "gpl local sub" }, // 0x13
    { gpl_lua_global_sub, "gpl global sub" }, // 0x14
    { gpl_lua_local_ret, "gpl local ret" }, // 0x15
    { gpl_lua_load_var, "gpl load variable" }, // 0x16
    { gpl_lua_compare, "gpl compare" }, // 0x17
    { gpl_lua_load_accum, "gpl load accum" }, // 0x18
    { gpl_lua_global_ret, "gpl global ret" }, // 0x19
    { gpl_lua_nextto, "gpl nextto" }, // 0x1A
    { gpl_lua_inlostrigger, "gpl inlostrigger" }, // 0x1B
    { gpl_lua_notinlostrigger, "gpl notinlostrigger" }, // 0x1C
    { gpl_lua_clear_los, "gpl clear los" }, // 0x1D
    { gpl_lua_nametonum, "gpl nametonum" }, // 0x1E
    { gpl_lua_numtoname, "gpl numtoname" }, // 0x1F
    { gpl_lua_bitsnoop, "gpl bitsnoop" }, // 0x20
    { gpl_lua_award, "gpl award" }, // 0x21
    { gpl_lua_request, "gpl request" }, // 0x22
    { gpl_lua_source_trace, "gpl source trace" }, // 0x23
    { gpl_lua_shop, "gpl shop" }, // 0x24
    { gpl_lua_clone, "gpl clone" }, // 0x25
    { NULL, "gpl default" }, // 0x26
    { gpl_lua_ifcompare, "gpl ifcompare" }, // 0x27
    { NULL, "gpl trace var" }, // 0x28
    { gpl_lua_orelse, "gpl orelse" }, // 0x29
    { gpl_lua_clearpic, "gpl clearpic" }, // 0x2A
    { gpl_lua_continue, "gpl continue" }, // 0x2B
    { gpl_lua_log, "gpl log" }, // 0x2C
    { gpl_lua_damage, "gpl damage" }, // 0x2D
    { gpl_lua_source_line_num, "gpl source line num" }, // 0x2E
    { gpl_lua_drop, "gpl drop" }, // 0x2F
    { gpl_lua_passtime, "gpl passtime" }, // 0x30
    { gpl_lua_exit, "gpl exit gpl" }, // 0x31
    { gpl_lua_fetch, "gpl fetch" }, // 0x32
    { gpl_lua_search, "gpl search" }, // 0x33
    { gpl_lua_getparty, "gpl getparty" }, // 0x34
    { gpl_lua_fight, "gpl fight" }, // 0x35
    { gpl_lua_flee, "gpl flee" }, // 0x36
    { gpl_lua_follow, "gpl follow" }, // 0x37
    { gpl_lua_getyn, "gpl getyn" }, // 0x38
    { gpl_lua_give, "gpl give" }, // 0x39
    { gpl_lua_go, "gpl go" }, // 0x3A
    { gpl_lua_input_bignum, "gpl input bignum" }, // 0x3B
    { gpl_lua_goxy, "gpl goxy" }, // 0x3C
    { gpl_lua_readorders, "gpl readorders" }, // 0x3D
    { gpl_lua_if, "gpl if" }, // 0x3E
    { gpl_lua_else, "gpl else" }, // 0x3F
    { gpl_lua_setrecord, "gpl setrecord" }, // 0x40
    { gpl_lua_setother, "gpl setother" }, // 0x41
    { gpl_lua_input_string, "gpl input string" }, // 0x42
    { gpl_lua_input_number, "gpl input number" }, // 0x43
    { gpl_lua_input_money, "gpl input money" }, // 0x44
    { gpl_lua_joinparty, "gpl joinparty" }, // 0x45
    { gpl_lua_leaveparty, "gpl leaveparty" }, // 0x46
    { gpl_lua_lockdoor, "gpl lockdoor" }, // 0x47
    { gpl_lua_menu, "gpl menu" }, // 0x48
    { gpl_lua_setthing, "gpl setthing" }, // 0x49
    { NULL, "gpl default" }, // 0x4A
    { NULL, "gpl local sub trace" }, // 0x4B
    { NULL, "gpl default" }, // 0x4C
    { NULL, "gpl default" }, // 0x4D
    { NULL, "gpl default" }, // 0x4E
    { gpl_lua_print_string, "gpl print string" }, // 0x4F
    { gpl_lua_print_number, "gpl print number" }, // 0x50
    { gpl_lua_printnl, "gpl printnl" }, // 0x51
    { gpl_lua_rand, "gpl rand" }, // 0x52
    { NULL, "gpl default" }, // 0x53
    { gpl_lua_showpic, "gpl showpic" }, // 0x54
    { NULL, "gpl default" }, // 0x55
    { NULL, "gpl default" }, // 0x56
    { NULL, "gpl default" }, // 0x57
    { gpl_lua_skillroll, "gpl skillroll" }, // 0x58
    { gpl_lua_statroll, "gpl statroll" }, // 0x59
    { gpl_lua_string_compare, "gpl string compare" }, // 0x5A
    { gpl_lua_match_string, "gpl match string" }, // 0x5B
    { gpl_lua_take, "gpl take" }, // 0x5C
    { gpl_lua_sound, "gpl sound" }, // 0x5D
    { gpl_lua_tport, "gpl tport" }, // 0x5E
    { gpl_lua_music, "gpl music" }, // 0x5F
    { NULL, "gpl default" }, // 0x60
    { gpl_lua_cmpend, "gpl cmpend" }, // 0x61
    { gpl_lua_wait, "gpl wait" }, // 0x62
    { gpl_lua_while, "gpl while" }, // 0x63
    { gpl_lua_wend, "gpl wend" }, // 0x64
    { gpl_lua_attacktrigger, "gpl attacktrigger" }, // 0x65
    { gpl_lua_looktrigger, "gpl looktrigger" }, // 0x66
    { gpl_lua_end_control, "gpl endif" }, // 0x67
    { gpl_lua_move_tiletrigger, "gpl move tiletrigger" }, // 0x68
    { gpl_lua_door_tiletrigger, "gpl door tiletrigger" }, // 0x69
    { gpl_lua_move_boxtrigger, "gpl move boxtrigger" }, // 0x6A
    { gpl_lua_door_boxtrigger, "gpl door boxtrigger" }, // 0x6B
    { gpl_lua_pickup_itemtrigger, "gpl pickup itemtrigger" }, // 0x6C
    { gpl_lua_usetrigger, "gpl usetrigger" }, // 0x6D
    { gpl_lua_talktotrigger, "gpl talktotrigger" }, // 0x6E
    { gpl_lua_noorderstrigger, "gpl noorderstrigger" }, // 0x6F
    { gpl_lua_usewithtrigger, "gpl usewithtrigger" }, // 0x70
    { NULL, "gpl default" }, // 0x71
    { NULL, "gpl default" }, // 0x72
    { NULL, "gpl default" }, // 0x73
    { NULL, "gpl default" }, // 0x74
    { NULL, "gpl default" }, // 0x75
    { NULL, "gpl byte plus equal" }, // 0x76
    { NULL, "gpl byte minus equal" }, // 0x77
    { NULL, "gpl byte times equal" }, // 0x78
    { NULL, "gpl byte divide equal" }, // 0x79
    { gpl_lua_word_plus_equal, "gpl word plus equal" }, // 0x7A
    { gpl_lua_word_minus_equal, "gpl word minus equal" }, // 0x7B
    { gpl_lua_word_times_equal, "gpl word times equal" }, // 0x7C
    { gpl_lua_word_divide_equal, "gpl word divide equal" }, // 0x7D
    { gpl_lua_long_plus_equal, "gpl long plus equal" }, // 0x7E
    { gpl_lua_long_minus_equal, "gpl long minus equal" }, // 0x7F
    { gpl_lua_get_range, "gpl get range" }, // 0x80
};

static void do_lua_gpl_command(uint8_t cmd) {
    //fflush(stdout);
    //debug("[%p:%d]command byte = 0x%x (%s)\n", (void*)get_data_start_ptr(),
        //(int32_t) (get_data_ptr() - get_data_start_ptr()), cmd, gpl_operations[cmd].name);
    //exit_gpl = 0;
    //printf("cmd = 0x%x (do_lua_gpl_command.)\n", cmd);
    (*gpl_lua_operations[cmd].func)();
    //print_vars(0);
}

/*
 * Warning: This function returns an internal buffer that *does* change
 * with subsequent calls. Make any copies as needed!
 */
static void gpl_lua_pass(unsigned char *gpl, const size_t len, const int pass_num) {
    lua_pos = 0;
    lua_depth = 0; 
    lua_buf[0] = '\0';
    funcnum = 0;
    in_func = 0;
    varnum = 0;

    if (pass_num == 0) {
        collect_labels = 1;
        label_pos = 0;
    } else {
        collect_labels = 0;
    }

    gpl_push_data_ptr(gpl);
    gpl_set_data_ptr(gpl, gpl);
    gpl_lua_start_ptr = (size_t)gpl;
    const size_t start = (size_t)gpl_get_data_ptr();
    size_t diff = (size_t)gpl_get_data_ptr() - start;
    is_master_mas = (script_id == 99 && is_mas);
    //printf("gpl = %p, len = %ld\n", gpl, len);
    while (diff < len && print_cmd()) {
        diff = (size_t)gpl_get_data_ptr() - start;
    }
    while (lua_depth-- > 0) {
        lprintf("end\n");
    }
    //printf("tranversed = %ld, len = %ld\n", (size_t)gpl_get_data_ptr() - (size_t)start, len);
    gpl_pop_data_ptr();
}

extern char* gpl_lua_print(const size_t _script_id, const int _is_mas, size_t *script_len) {
    //size_t len;
    unsigned char *gpl = NULL;
    script_id = _script_id;
    is_mas = _is_mas;

    gff_chunk_header_t chunk = gff_find_chunk_header(DSLDATA_GFF_INDEX,
        is_mas ? GFF_MAS : GFF_GPL,
        script_id);
    gpl = malloc(chunk.length);
    if (!gpl) {
        error ("Unable to alloc for gpl script!\n");
        exit(1);
    }
    gff_read_chunk(DSLDATA_GFF_INDEX, &chunk, gpl, chunk.length);
    
/*
    lua_pos = 0;
    lua_depth = 0; 
    lua_buf[0] = '\0';
    funcnum = 0;
    in_func = 0;
    varnum = 0;

    gpl_push_data_ptr(gpl);
    set_data_ptr(gpl, gpl);
    gpl_lua_start_ptr = (size_t)gpl;
    const size_t start = (size_t)gpl_get_data_ptr();
    size_t diff = (size_t)gpl_get_data_ptr() - start;
    is_master_mas = (script_id == 99 && is_mas);
    //printf("gpl = %p, len = %ld\n", gpl, len);
    while (diff < len && print_cmd()) {
        diff = (size_t)gpl_get_data_ptr() - start;
    }
    while (lua_depth-- > 0) {
        lprintf("end\n");
    }
    //printf("tranversed = %ld, len = %ld\n", (size_t)gpl_get_data_ptr() - (size_t)start, len);
    pop_data_ptr();
    */
    //gpl_lua_pass(gpl, len, 0);
    //gpl_lua_pass(gpl, len, 1);
    gpl_lua_pass(gpl, chunk.length, 0);
    gpl_lua_pass(gpl, chunk.length, 1);
    *script_len = lua_pos - 1;
    lua_buf[lua_pos - 1] = '\0';
    free(gpl);
    return lua_buf;
}

static int print_cmd() {
    if (!in_func) {
        //lprintf("function func%d (accum) -- address %ld\n", funcnum++, ((size_t)gpl_get_data_ptr()) - gpl_lua_start_ptr);
        cfunc_num = ((size_t)gpl_get_data_ptr()) - gpl_lua_start_ptr;
        lprintf("function func%ld ()\n", cfunc_num);
        //lprintf("function %c%ldfunc%ld ()\n", 
            //is_mas ? 'm' : 'g',
            //script_id, ((size_t)gpl_get_data_ptr()) - gpl_lua_start_ptr);
        lua_depth++;
        in_func = 1;
        //lprintf("gpl.debug(\"func%ld\")\n", cfunc_num);
    } else {
        print_label();
    }
    uint8_t command = gpl_get_byte();
    if (!gpl_lua_operations[command].func) {
        printf("Lua code so far:\n%s\n", lua_buf);
        printf("Unimplemented command = 0x%2x\n", command);
        exit(1);
        return 0;
    }
    //printf("print_cmd: command = 0x%x\n", command);
    (*gpl_lua_operations[command].func)();
    return 1;
}

static void gpl_lua_string_copy() {
    char buf[BUF_SIZE];

    gpl_lua_get_parameters(2);
    lprintf("var%d = \"%s\"\n", varnum++, (char*)lparams.params[1]);
    if (strncmp("gpl.get_gstr(", lparams.params[0], 13) == 0) {
        strncpy(buf, lparams.params[0], BUF_SIZE);
        buf[4] = 's';
        buf[strlen(buf) - 1] = '\0';
        lprintf("%s, var%d)\n", buf, varnum - 1);
    } else {
        lprintf("--string_copy, unknown parameter '%s'\n", lparams.params[0]);
    }
}

// Does this variable come from parameter passing?
extern void gpl_lua_load_accum(void) {
    char buf[BUF_SIZE];
    gpl_lua_read_number(buf, BUF_SIZE);
    lprintf("accum = %s -- load_accum\n", buf);
}

extern void gpl_lua_global_ret(void) {
    in_func = 0;
    //lprintf("gpl.debug(\"return func%ld\")\n", cfunc_num);
    lua_depth--;
    lprintf("end --return\n");
    if (lua_depth < 0) { lua_depth = 0; }
}

extern void gpl_lua_nextto(void) {
    gpl_lua_get_parameters(2);
    lprintf("accum = gpl.objects_are_adjacent(%s, %s)\n", lparams.params[0], lparams.params[1]);
}

extern void gpl_lua_inlostrigger(void) {
    gpl_lua_get_parameters(4);
    uint32_t addr = atoi(lparams.params[0]);
    uint32_t file = atoi(lparams.params[1]);
    uint32_t name = atoi(lparams.params[2]);
    //uint32_t global = in_mas ? 1 : 0;
    //global_addr_name(&param);
    //set_los_order(GPL_IN_LOS, param.val[3]);
    //lprintf("-- need to set line of sight order on %d\n", param.val[3]);
    lprintf("-- If in line of sight of %d goto file %d:%d (?: %s)\n", name, file, addr, lparams.params[3]);
}

extern void gpl_lua_notinlostrigger(void) {
    gpl_lua_get_parameters(4);
    //global_addr_name(&param);
    //set_los_order(GPL_NOT_IN_LOS, param.val[3]);
    lprintf("-- need to set not line of sight order on %d\n", param.val[3]);
}

extern void gpl_lua_move_tiletrigger(void) {
    gpl_lua_get_parameters(5);
    lprintf("gpl.tile_trigger(%s, %s, %s, %s, %s)\n",
        lparams.params[0], lparams.params[1], lparams.params[3], lparams.params[2], lparams.params[4]);
}

extern void gpl_lua_continue(void) {
    lprintf("gpl.narrate_open(NAR_ADD_MENU, \"Press Continue\", 0)\n");
    lprintf("gpl.narrate_open(NAR_ADD_MENU, \"Continue\", 0)\n");
    lprintf("gpl.narrate_open(NAR_SHOW_MENU, \"\", 0)\n");
    lprintf("--wait for input from user!\n");
}

extern void gpl_lua_log(void) {
    lua_exit("gpl_log not implemented.\n");
}

static void do_lua_damage(int is_percent) {
    gpl_lua_get_parameters(2);
    lprintf("if %s == %d then\n", lparams.params[0], ALL);
    lua_depth++;
    lprintf("--illegal parameter ALL to do_damage\n");
    lua_depth--;
    lprintf("elseif %s == %d then\n", lparams.params[0], PARTY);
    lua_depth++;
    if (is_percent) {
        lprintf("--need to do %s%% to all party members\n", lparams.params[1]);
    } else {
        lprintf("--need to do %s amt of damage to all party members\n", lparams.params[1]);
    }
    lua_depth--;
    lprintf("else\n");
    lua_depth++;
    lprintf("if %s >= 0 then\n", lparams.params[0]);
    lua_depth++;
    if (is_percent) {
        lprintf("--need to do %s%% to (party members) %s\n", lparams.params[1], lparams.params[0]);
    } else {
        lprintf("--need to do %s amt of damage to (party members) %s\n", lparams.params[1], lparams.params[0]);
    }
    lua_depth--;
    lprintf("else\n");
    lua_depth++;
    if (is_percent) {
        lprintf("--need to do %s%% to (object id) %s\n", lparams.params[1], lparams.params[0]);
    } else {
        lprintf("--need to do %s amt of damage to (object id) %s\n", lparams.params[1], lparams.params[0]);
    }
    lua_depth--;
    lprintf("end\n");
    lua_depth--;
    lprintf("end\n");
}

extern void gpl_lua_damage(void) {
    do_lua_damage(0);
}

extern void gpl_lua_source_line_num(void) {
    command_implemented = 0;
    lua_exit("source line num not implemented\n");
}

extern void gpl_lua_drop(void) {
    gpl_lua_get_parameters(3);
    lprintf("if %s == %d then \n", lparams.params[2], PARTY);
    lua_depth++;
    lprintf("-- all pary members need to drop %s of %s\n", lparams.params[0], lparams.params[1]);
    lua_depth--;
    lprintf("elseif %s == %d then\n", lparams.params[2], ALL);
    lua_depth++;
    lprintf("--do nothing\n");
    lua_depth--;
    lprintf("else\n");
    lua_depth++;
    lprintf("accum = gpl.drop(%s, %s, %s) -- %s needs to drop %s of %s\n",
       lparams.params[0], lparams.params[1], lparams.params[2],
       lparams.params[0], lparams.params[1], lparams.params[2]);
    lua_depth--;
    lprintf("end -- drop\n");
}

extern void gpl_lua_passtime(void) {
    lua_exit("gpl_passtime not implemented\n");
}

extern void gpl_lua_door_tiletrigger(void) {
    lua_exit("Can't parse door_tiletrigger yet.\n");
    lprintf("--unimplmented move box trigger.\n");
}

extern void gpl_lua_move_boxtrigger(void) {
    gpl_lua_get_parameters(7);
    lprintf("gpl.box_trigger(%s, %s, %s, %s, %s, %s, %s)\n",
        lparams.params[0], lparams.params[1], lparams.params[2], lparams.params[3], lparams.params[4],
        lparams.params[5], lparams.params[6]);
}

extern void gpl_lua_door_boxtrigger(void) {
    lua_exit("Can't parse lua_door_boxtrigger yet.\n");
    lprintf("--unimplmented door box trigger.\n");
    lprintf("gpl.box_trigger(%s, %s, %s, %s, %s, %s, %s)\n",
        lparams.params[0], lparams.params[1], lparams.params[2], lparams.params[3], lparams.params[4],
        lparams.params[5], lparams.params[6]);
}

extern void gpl_lua_pickup_itemtrigger(void) {
    gpl_lua_get_parameters(3);
    //global_addr_name(&param);
    lprintf("--unimplmented pickup item trigger.\n");
}

extern void gpl_lua_talktotrigger(void) {
    gpl_lua_get_parameters(3);
    char *addr = lparams.params[0];
    char *file = lparams.params[1];
    char *name = lparams.params[2];
    int is_global = (gpl_current_file == GLOBAL_MAS) && (gpl_current_type == MASFILE);

    lprintf("gpl.talk_to_trigger(%s, %s, %s, %d)\n",
        name, file, addr, is_global);
    //lprintf("When I %s to '%s' (%s) goto file: %s, addr: %s, global = %d\n",
        //"talk to", name, name, file, addr, is_global);
    //debug("When I %s to '%s' (%d) goto file: %d, addr: %d, global = %d\n",
        //"talk to", get_so_name(so), new_name.name,
        //new_name.file, new_name.addr, new_name.global);
    //global_addr_name(&param);
}

extern void gpl_lua_noorderstrigger(void) {
    gpl_lua_get_parameters(3);
    lprintf("gpl.noorders_trigger%s(%s, %s, %s)\n",
        is_master_mas ? "_global" : "",
        lparams.params[2], lparams.params[1], lparams.params[0]);
}

extern void gpl_lua_usewithtrigger(void) {
    gpl_lua_get_parameters(4);
    lprintf("gpl.use_with_trigger%s(%s, %s, %s, %s)\n",
        is_master_mas ? "_global" : "",
        lparams.params[0], lparams.params[1], lparams.params[3], lparams.params[2]);
}

extern void gpl_lua_cmpend(void) {
    if (in_compare) {
        lua_depth--;
        lprintf("end\n");
        in_compare = 0;
    } else {
        lprintf("-- warning: encountered cmpend, but not in cmpend!\n");
    }
}

extern void gpl_lua_wait(void) {
    char buf[BUF_SIZE];
    gpl_lua_read_number(buf, BUF_SIZE);
    lprintf("gpl.set_order(%s, WAIT, 0, 0)\n", buf);
}

extern void gpl_lua_while(void) {
    gpl_lua_get_parameters(1);
    //lprintf("while accum == true do\n");
    lprintf("if accum == true then -- \"while\" loop\n");
    lua_depth++;
}

extern void gpl_lua_wend(void) {
    char buf[BUF_SIZE];
    gpl_lua_read_number(buf, BUF_SIZE);
    lua_goto(buf);
    lua_depth--;
    lprintf("end\n");
}

extern void gpl_lua_attacktrigger(void) {
    gpl_lua_get_parameters(3);
    //global_addr_name(&param);
    //generic_name_trigger(ATTACK_CHECK_INDEX);
    lprintf("gpl.attack_trigger%s(%s, %s, %s)\n",
        is_master_mas ? "_global" : "",
        lparams.params[2], lparams.params[1], lparams.params[0]);
}

extern void gpl_lua_looktrigger(void) {
    gpl_lua_get_parameters(3);
    //global_addr_name(&param);
    //generic_name_trigger(LOOK_CHECK_INDEX);
    lprintf("gpl.look_trigger%s(%s, %s, %s)\n",
        is_master_mas ? "_global" : "",
        lparams.params[2], lparams.params[1], lparams.params[0]);
}

extern void gpl_lua_usetrigger(void) {
    gpl_lua_get_parameters(3);
    lprintf("gpl.use_trigger%s(%s, %s, %s) -- When using %s go to file %s address %s\n",
        is_master_mas ? "_global" : "",
        lparams.params[2], lparams.params[1], lparams.params[0],
        lparams.params[2], lparams.params[1], lparams.params[0]);
}

extern void gpl_lua_fetch(void) {
    gpl_lua_get_parameters(2);
    lprintf("gpl.set_orders(%s, FETCH, %s, 0)\n", lparams.params[1], lparams.params[0]);
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

extern void gpl_lua_search(void) {
    char object[BUF_SIZE];
    char buf[BUF_SIZE - 128];
    //uint32_t answer = 0L;
    gpl_lua_read_number(object, BUF_SIZE);
    int16_t low_field = gpl_get_byte();
    int16_t high_field = gpl_get_byte();
    int16_t field_level = -1, depth = 1;
    int32_t search_for = 0, temp_for = 0;
    uint16_t field[16];
    uint8_t type = 0;
    char *i;
    do {
        if (gpl_peek_one_byte() == OBJ_QUALIFIER) {
            gpl_get_byte();
        }
        field_level++;
        field[field_level] = gpl_get_byte();
        type = gpl_get_byte();
        if (type >= EQU_SEARCH && type <= GT_SEARCH) {
            gpl_lua_read_number(buf, BUF_SIZE - 128);
            validate_number(buf, "gpl_lua_search");
            //temp_for = read_number();
            temp_for = atoi(buf);
            if (field_level > 0) {
                field[++field_level] = (uint16_t) temp_for;
            } else {
                search_for = temp_for;
            }
        }
        depth--;
    } while ( gpl_peek_one_byte() == OBJ_QUALIFIER);

    //i = (object == PARTY || object < 0)
        //? NULL_OBJECT : object;
    i = object;

    /*
    do {
        if (type == LOW_SEARCH) {
            answer = 0x7FFFFFFFL;
        }
    } while ( (object == PARTY) && (i != NULL_OBJECT));
    */
    lprintf("--I need to find object %s, i = %s, low_field = %d, high_field = %d\n",
        object, i, low_field, high_field);
    lprintf("--Field parameters:");
    for (int idx = 0; idx < depth; idx++) {
        warn("%d ", field[idx]);
    }
    lprintf("--search_for = %d\n", search_for);

    lprintf("--search command to be implemented later...\n");
    lprintf("accum = answer\n");
}

extern void gpl_lua_getparty(void) {
    char buf[BUF_SIZE];
    gpl_lua_read_number(buf, BUF_SIZE);
    lprintf("gpl.get_party(%s)\n", buf);
    //lprintf("gOther = %s\n", buf);
    //lprintf("accum = %s\n", buf);
}

extern void gpl_lua_fight(void) {
    lprintf("--I need to enter fight mode!\n");
    //gpl_exit_gpl();
}

extern void gpl_lua_flee(void) {
    char buf[BUF_SIZE];
    gpl_lua_read_number(buf, BUF_SIZE);
    lprintf("--I need to flee!\n");
    //set_orders(read_number(), FLEE, 0, 0);
}

extern void gpl_lua_follow(void) {
    gpl_lua_get_parameters(2);
    lprintf("--I need to follow\n");
    //set_orders(param.val[1], FOLLOW, param.val[0], 0);
}

extern void gpl_lua_getyn(void) {
    lprintf("gpl.ask_yes_no()\n");
}

extern void gpl_lua_give(void) {
    gpl_lua_get_parameters(4);
    //lprintf("--need to give and set accumulator!");
    //set_accumulator(give(param.val[0], param.val[1], param.val[2], param.val[3], GPL_NEW_SLOT));
    lprintf("gpl.give(%s, %s, %s, %s, GPL_NEW_SLOT)\n",
        lparams.params[0], lparams.params[1], lparams.params[2], lparams.params[3]);
}

extern void gpl_lua_go(void) {
    gpl_lua_get_parameters(2);
    lprintf("--need to give GO order!\n");
    lprintf("print (\"GO ORDER\")\n");
    //set_orders(param.val[1], GO_OBJECT, param.val[0], 0);
}

extern void gpl_lua_input_bignum(void) {
    lua_exit("input bignum not implemented\n");
}

extern void gpl_lua_goxy(void) {
    gpl_lua_get_parameters(3);
    lprintf("--I need to goxy1\n");
    //set_orders(param.val[2], GOXY, param.val[0], param.val[1]);
}

extern void gpl_lua_readorders(void) {
    char buf[BUF_SIZE];
    gpl_lua_read_number(buf, BUF_SIZE);
    lprintf("accum = gpl.read_order(%s)\n", buf);
}

extern void gpl_lua_if(void) {
    // The paramter is ignored, in the original it probably was the address
    // to jump to if the if was not taken.
    gpl_lua_get_parameters(1);
    lprintf("if gpl.is_true(accum) then \n");
    lua_depth++;
}

extern void gpl_lua_else(void) {
    // The paramter is ignored, in the original it probably was the address
    // to jump to if the else was not taken.
    gpl_lua_get_parameters(1);
    lua_depth--;
    lprintf("else\n");
    lua_depth++;
}

extern void gpl_lua_setrecord(void) {
    uint16_t depth = 0;
    int16_t header = 0;
    uint16_t element[MAX_SEARCH_STACK];
    uint16_t tmp = gpl_peek_half_word();
    int32_t obj_name;
    char buf[BUF_SIZE];

    if (tmp > 0x8000) {
        gpl_lua_access_complex(&header, &depth, element, &obj_name);
        gpl_lua_read_number(buf, BUF_SIZE);
        lprintf("accum = %s\n", buf);
        lprintf("--setrecordneed to do a smart write for setrecord.\n");
        //smart_write_data(header, depth, element, accum);
        return;
    }
    if (tmp == 0) {
        lua_exit("gpl_setrecord: need to implement party...\n");
        return;
    }
    if (tmp < 0x8000) {
        gpl_lua_access_complex(&header, &depth, element, &obj_name);
        gpl_lua_read_number(buf, BUF_SIZE);
        lprintf("accum = %s\n", buf);
        lprintf("--setrecord:I need to write depth/element/accum to list of headers!\n");
        return;
    }
}

extern void gpl_lua_setother(void) {
    char buf[BUF_SIZE];
    int32_t header = 0;
    gpl_lua_read_number(buf, BUF_SIZE);
    lprintf("if (%s >= 0 and head ~= NULL_OBJECT) or (%s == gpl.id_to_header(%s)) ~= NULL_OBJECT then\n", buf, buf, buf);
    lua_depth++;
    lprintf("other1 = %d\n", header);
    lprintf("accum = 1\n");
    lua_depth--;
    lprintf("else\n");
    lua_depth++;
    lprintf("accum = 0\n");
    lua_depth--;
    lprintf("end\n");
}

extern void gpl_lua_end_control(void) {
    lua_depth--;
    lprintf("end --end_control\n");
}

extern void gpl_lua_input_string(void) {
    gpl_lua_get_parameters(1);
    lprintf("gpl.ask_for_string()\n");
}

extern void gpl_lua_input_number(void) {
    gpl_lua_get_parameters(1);
    lprintf("gpl.ask_for_number()\n");
}

extern void gpl_lua_input_money(void) {
    gpl_lua_get_parameters(1);
    lprintf("gpl.ask_for_money()\n");
}

extern void gpl_lua_joinparty(void) {
    lua_exit("join party not implemented\n");
}

extern void gpl_lua_leaveparty(void) {
    lua_exit("leave party not implemented\n");
}

extern void gpl_lua_lockdoor(void) {
    lprintf("gpl.lockdoor(1)\n");
}

#define MAXMENU   (24)

extern void gpl_lua_menu(void) {
    int items = 0;
    char buf[BUF_SIZE];
    char mfunction[BUF_SIZE];
    char mbytes[BUF_SIZE];
    gpl_lua_read_number(buf, BUF_SIZE);
    char *menu = (char*) buf;
    size_t menu_len;

    if (needs_quotes(buf)) {
        lprintf("gpl.narrate_open(NAR_ADD_MENU, \"%s\", 0)\n", menu);
    } else {
        lprintf("gpl.narrate_open(NAR_ADD_MENU, %s, 0)\n", menu);
    }
    while ((gpl_peek_one_byte() != 0x4A) && (items <= MAXMENU)) {
        menu_len = gpl_lua_read_number(buf, BUF_SIZE);
        //menu = (char*) gpl_global_big_numptr;
        menu = (char*) buf;
        while (menu[menu_len - 1] == '\n') {
            menu[--menu_len] = '\0';
        }
        gpl_lua_read_number(mfunction, BUF_SIZE);
        gpl_lua_read_number(mbytes, BUF_SIZE);
        //menu_functions[items] = read_number();
        //menu_bytes[items] = (uint8_t) read_number();
        //if (menu_bytes[items] == 1) {
            //lprintf("narrate_open(NAR_ADD_MENU, \"%s\", %d)\n", menu, items - 1);
        items++;
        //} else {
            //warn("Not available at this time: '%s'\n", menu);
        //}
        lprintf("if %s == 1 then\n", mbytes);
        lua_depth++;
        lprintf("gpl.narrate_open(NAR_ADD_MENU, \"%s\", %s) -- choice param1 goes to addr param2\n", menu, mfunction);
        lua_depth--;
        lprintf("end\n");
    }
    gpl_get_byte();  // get rid of the mend...
    //for (int i = 0; i < items; i++) {
        //lprintf("narrate_choice( %d, %d) -- choice param1 goes to addr param2\n", i, menu_functions[i]);
    //}
    //lprintf("accum = gpl.narrate_show() --narrate_wait for input\n");
    lprintf("gpl.narrate_show() --narrate_wait for input\n");
    //printf("Need to implement display_menu()\n");
    //command_implemented = 0;
}

extern void gpl_lua_setthing(void) {
    gpl_lua_get_parameters(2);
    lprintf("--move character's %s's itme of type %s\n", lparams.params[0], lparams.params[1]);
}

extern void gpl_lua_print_string(void) {
    gpl_lua_get_parameters(2);
    char nq = needs_quotes(lparams.params[1]) ? '\"' : ' ';
    lprintf("gpl.narrate_open(NAR_SHOW_TEXT, %c%s%c, %s)\n",
        nq, lparams.params[1], nq, lparams.params[0]);
}

extern void gpl_lua_print_number(void) {
    gpl_lua_get_parameters(2);
    lprintf("gpl.narrate_open(NAR_SHOW_TEXT, \"%s\", %s)\n",
        lparams.params[1], lparams.params[0]);
}

extern void gpl_lua_printnl(void) {
    lprintf("gpl.narrate_open(NAR_SHOW_TEXT, \"\\n\", 0)\n");
}

extern void gpl_lua_rand(void) {
    char buf[BUF_SIZE];
    gpl_lua_read_number(buf, BUF_SIZE);
    lprintf("gpl.rand() %% %d\n", atoi(buf) + 1);
    //set_accumulator((int32_t)rand() % (read_number() + 1));
}

extern void gpl_lua_showpic(void) {
    char buf[BUF_SIZE];
    gpl_lua_read_number(buf, BUF_SIZE);
    lprintf("gpl.narrate_open(NAR_PORTRAIT, \"\", %s)\n", buf);
}

extern void gpl_lua_skillroll(void) {
    lua_exit("skillroll not implemented\n");
}

extern void gpl_lua_statroll(void) {
    gpl_lua_get_parameters(3);
    lprintf("accum = 0\n");
    lprintf("if lparams.params[0] == PARTY then\n");
    lua_depth++;
    lprintf("gpl.gpl_stat_roll(%s)\n", lparams.params[0]);
    lua_depth--;
    lprintf("else\n");
    lua_depth++;
    lprintf("gpl.gpl_stat_roll_party()\n");
    lua_depth--;
    lprintf("end\n");
}

extern void gpl_lua_string_compare(void) {
    lprintf("accum = 0\n");
    gpl_lua_get_parameters(2);
    lprintf("if %s == %s then\n", lparams.params[0], lparams.params[1]);
    lua_depth++;
    lprintf("accum = 1\n");
    lua_depth--;
    lprintf("end\n");
}

extern void gpl_lua_match_string(void) {
    lua_exit("match_string not implmeneted\n");
}

extern void gpl_lua_take(void) {
    gpl_lua_get_parameters(4);
    int32_t who = atoi(lparams.params[2]);
    int32_t item = atoi(lparams.params[1]);
    if (who == PARTY) {
        lprintf("accum = 0\n");
        lprintf("--For each party member:\n");
        lprintf("    -- if %d is not grafted take it and update accumulator.\n", item);
        //if (!grafted(party_character, param.val[1])) {
            //set_accumulator(get_accumulator() + take(param.val[0], param.val[1], party_character, param.val[3]));
        //}
    } else {
        lprintf("-- if %d is not grafted take it and update accumulator.\n", item);
        lprintf("accum = 0 -- take out after updating gpl_lua_take\n");
        //if (!grafted(param.val[2], param.val[1])) {
            //set_accumulator(take(param.val[0], param.val[1], param.val[2], param.val[3]));
        //}
    }
}

extern void gpl_lua_sound(void) {
    char buf[BUF_SIZE];
    gpl_lua_read_number(buf, BUF_SIZE);
    lprintf("gpl.play_sound(%s) -- parameter is bvoc index\n", buf);
}

extern void gpl_lua_music(void) {
    char buf[BUF_SIZE];
    gpl_lua_read_number(buf, BUF_SIZE);
    lprintf("gpl.play_music(%s) -- parameter is xmi index\n", buf);
}

#define CHOSEN (0x7FFD)
#define PARTY  (0x7FFE)
#define ALL    (0x7FFF)
#define IS_POV   (0)
#define IS_PARTY (1)
#define IS_NPC (1)

extern void gpl_lua_tport(void) {
    gpl_lua_get_parameters(5);
    if ((param.val[0] != PARTY) && (param.val[0] != IS_POV)) {
        //warn("I need to teleport everything to region %d at (%d, %d) priority: %d, onwindow %d\n",
            //param.val[1], param.val[2], param.val[3], param.val[4], param.val[5]);
        lprintf("gpl.tport_everything( %s, %s, %s, %s, %s)\n",
            lparams.params[1], lparams.params[2], lparams.params[3], lparams.params[4], lparams.params[5]);
    } else {
        //warn("I need to teleport party to region %d at (%d, %d) priority: %d, onwindow %d\n",
            //param.val[1], param.val[2], param.val[3], param.val[4], param.val[5]);
        lprintf("gpl.tport_party( %s, %s, %s, %s, %s)\n",
            lparams.params[1], lparams.params[2], lparams.params[3], lparams.params[4], lparams.params[5]);
    }
}

extern void gpl_lua_bitsnoop(void) {
    gpl_lua_get_parameters(2);
    lprintf("((%s & %s) == %s)\n", lparams.params[0], lparams.params[1], lparams.params[1]);
    /*
    lprintf("if %s & %s == %s then\n", lparams.params[0], lparams.params[1], lparams.params[1]);
    lua_depth++;
    lprintf("accum = 1\n");
    lua_depth--;
    lprintf("else\n");
    lua_depth++;
    lprintf("accum = 0\n");
    lua_depth--;
    lprintf("end\n");
    */
}

extern void gpl_lua_award(void) {
    gpl_lua_get_parameters(2);
    lprintf("if %s == %d then\n", lparams.params[0], ALL);
    lua_depth++;
    lprintf("accum = accum\n");
    lua_depth--;
    lprintf("elseif %s == %d then\n", lparams.params[0], PARTY);
    lua_depth++;
    lprintf("gpl.award_party(%s)\n", lparams.params[1]);
    lua_depth--;
    lprintf("else\n");
    lua_depth++;
    lprintf("gpl.award_object(%s, %s)\n", lparams.params[0], lparams.params[1]);
    lua_depth--;
    lprintf("end\n");
}

extern void gpl_lua_request(void) {
    gpl_lua_get_parameters(4);
    lprintf("accum = gpl.request(%s, %s, %s, %s)\n",
        (lparams.params[0]),
        (lparams.params[1]),
        (lparams.params[2]),
        (lparams.params[3]));
    lprintf("obj = accum\n");
}

static void print_change(const char *stmt, const char *op) {
    char buf[128];
    int pos;
    if (!strncmp(stmt, "gpl.get_", 8)) {
        strcpy(buf, stmt); // create something to edit
        for (pos = 1; pos < strlen(stmt) && buf[pos] != 'g'; pos++) { ; }
        buf[pos] = 's';
        buf[strlen(buf) - 1] = '\0'; // chop off the final ')'
        lprintf("%s, %s %s)\n", buf, stmt, op);
        return;
    }
    lua_exit("Unable to convert stmt\n");
}

extern void gpl_lua_byte_dec(void) {
    gpl_lua_get_parameters(1);
    //( *((uint8_t *) param.ptr[0]) )--;
    lprintf("--((uint8_t)lparams.params[0])--\n");
    lprintf("-- byte_dec: %s\n", lparams.params[0]);
}

extern void gpl_lua_word_dec(void) {
    gpl_lua_get_parameters(1);
    lprintf("--((uint16_t)lparams.params[0])--\n");
    lprintf("-- word_dec: %s\n", lparams.params[0]);
    print_change(lparams.params[0], "- 1");
}

extern void gpl_lua_long_dec(void) {
    gpl_lua_get_parameters(1);
    lprintf("--((uint32_t)lparams.params[0])--\n");
    //command_implemented = 0;
    lprintf("-- long_dec: %s\n", lparams.params[0]);
}

extern void gpl_lua_byte_inc(void) {
    gpl_lua_get_parameters(1);
    lprintf("--((uint8_t)lparams.params[0])++\n");
    //( *((uint8_t *) param.ptr[0]) )++;
    lprintf("-- byte_inc: %s\n", lparams.params[0]);
}

extern void gpl_lua_word_inc(void) {
    gpl_lua_get_parameters(1);
    lprintf("--WORD INC\n");
    //lprintf("--((uint16_t)lparams.params[0])++\n");
    lprintf("--%s\n", lparams.params[0]);
    print_change(lparams.params[0], "+ 1");
}

extern void gpl_lua_long_inc(void) {
    gpl_lua_get_parameters(1);
    lprintf("--((uint32_t)lparams.params[0])++\n");
}

extern void gpl_lua_hunt(void) {
    char buf[BUF_SIZE];
    gpl_lua_read_number(buf, BUF_SIZE);
    lprintf("gpl.hunt(%s)\n", buf);
}

extern void gpl_lua_source_trace(void) {
    lua_exit("lua_source not implemented\n");;
}

extern void gpl_lua_shop(void) {
    char buf[BUF_SIZE];
    gpl_lua_read_number(buf, BUF_SIZE);
    lprintf("gpl.shop(%s)\n", buf);
}

extern void gpl_lua_clone(void) {
    gpl_lua_get_parameters(6);

    lprintf("obj = gpl.clone(%s, %s, %s, %s, %s, %s)\n",
        lparams.params[0],
        lparams.params[1],
        lparams.params[2],
        lparams.params[3],
        lparams.params[4],
        lparams.params[5]);
}

extern void gpl_lua_changemoney(void) {
    char buf[BUF_SIZE];
    gpl_lua_read_number(buf, BUF_SIZE);
    lprintf("gpl.give_money(%s)\n", buf);
}

extern void gpl_lua_setvar(void) {
    lua_exit("setvar not implemented");
}

extern void gpl_lua_toggle_accum(void) {
    lprintf("accum = !accum\n");
}

extern void gpl_lua_getstatus(void) {
    char buf[BUF_SIZE];
    gpl_lua_read_number(buf, BUF_SIZE);
    lprintf("accum = gpl.get_character_status(%s);\n", buf);
}

extern void gpl_lua_getlos(void) {
    gpl_lua_get_parameters(3);
    lprintf("accum = gpl.los(%s, %s, %s) -- los from %s to %s < %s\n",
        lparams.params[0], lparams.params[1], lparams.params[2],
        lparams.params[0], lparams.params[1], lparams.params[2]);
}

extern void gpl_lua_long_times_equal(void) {
    gpl_lua_get_parameters(2);
    lprintf("--*lparams.params[0] *= lparams.params[1]\n");
}

extern void gpl_lua_jump(void) {
    lua_exit("jump not implemented!\n");
}

extern void gpl_lua_local_sub(void) {
    char buf[BUF_SIZE];
    gpl_lua_read_number(buf, BUF_SIZE);
    lprintf("func%s()\n", buf);
}

extern void gpl_lua_global_sub(void) {
    gpl_lua_get_parameters(2);
    lprintf("gpl.call_function(%s, %s) -- Jump to addr %s in file %s (GPL.)\n",
        lparams.params[1], lparams.params[0],
        lparams.params[0], lparams.params[1]);
}

extern void gpl_lua_local_ret(void) {
    in_func = 0;
    //lprintf("gpl.debug(\"return func%ld\")\n", cfunc_num);
    lua_depth--;
    lprintf("end --return\n");
}

static void do_damage(int is_percent) {
    gpl_lua_get_parameters(2);
    int32_t type = atoi(lparams.params[0]);
    int32_t amt = atoi(lparams.params[1]);
    switch (type) {
        case ALL:
            lua_exit("illegal parameter ALL to do_damage!\n");
            break;
        case PARTY:
            if (is_percent) {
                lprintf("--need to do <kill-damage> of %d%% to all party members\n", amt);
            } else {
                lprintf("--need to do <kill-damage> of amt %d to all party members\n", amt);
            }
            break;
        default:
            if (type >= 0 ) {
                if (is_percent) {
                    lprintf("--need to do <kill-damage> of %d%% to %d.\n", amt, type);
                } else {
                    lprintf("--need to do <kill-damage> of amt %d to %d.\n", amt, type);
                }
            } else {
                if (is_percent) {
                    lprintf("--need to do <kill-damage> of %d%% to all objects with id %d.\n", amt, type);
                } else {
                    lprintf("--need to do <kill-damage> of amt %d to all objects with id %d.\n", amt, type);
                }
            }
            break;
    }
}

extern void gpl_lua_pdamage(void) {
    do_damage(1);
}

extern void gpl_lua_word_plus_equal(void) {
    gpl_lua_get_parameters(2);
    lprintf("--*((uint16_t *)lparams.params[0]) += lparam.params[1];\n");
    lprintf("gpl.error()");
    //*((uint16_t *)param.ptr[0]) += param.val[1];
}

extern void gpl_lua_word_minus_equal(void) {
    gpl_lua_get_parameters(2);
    lprintf("--*((uint16_t *)lparams.params[0]) -= lparam.params[1];\n");
    lprintf("gpl.error()");
    //*((uint16_t *)param.ptr[0]) -= param.val[1];
}

extern void gpl_lua_word_times_equal(void) {
    gpl_lua_get_parameters(2);
    lprintf("--*((uint16_t *)lparams.params[0]) *= lparam.params[1];\n");
    lprintf("gpl.error()");
    //*((uint16_t *)param.ptr[0]) *= param.val[1];
}

extern void gpl_lua_word_divide_equal(void) {
    gpl_lua_get_parameters(2);
    lprintf("--*((uint16_t *)lparams.params[0]) /= lparam.params[1];\n");
    lprintf("gpl.error()");
    //*((uint16_t *)param.ptr[0]) /= param.val[1];
}

extern void gpl_lua_long_plus_equal(void) {
    gpl_lua_get_parameters(2);
    lprintf("--*((uint32_t *)lparams.params[0]) += lparam.params[1];\n");
    lprintf("gpl.error()");
    //*((uint32_t *)param.ptr[0]) += param.val[1];
}

extern void gpl_lua_long_minus_equal(void) {
    gpl_lua_get_parameters(2);
    lprintf("--*((uint32_t *)lparams.params[0]) -= lparam.params[1];\n");
    lprintf("gpl.error()");
    //*((uint32_t *)param.ptr[0]) -= param.val[1];
}

uint16_t gpl_range(int16_t obj0, int16_t obj1) {
    lprintf("--Must compute range from %d to %d\n", obj0, obj1);
    lprintf("gpl.error()");
    return 10;//Totally bogus
}

extern void gpl_lua_get_range(void) {
    gpl_lua_get_parameters(2);
    lprintf("gpl.range(%s, %s)\n",
        lparams.params[0], lparams.params[1]);
}

extern void gpl_lua_compare(void) {
    gpl_lua_load_accum();
    lprintf("compare = accum\n");
}

static void gpl_lua_load_var() {
    gpl_lua_load_accum();
    gpl_lua_load_variable();
}

/********* EXTRA variable functions ********/
static void gpl_lua_write_complex_var() {
    uint16_t depth = 0;
    int16_t header = 0;
    uint16_t element[MAX_SEARCH_STACK];
    int32_t obj_name;
    memset(element, 0x0, sizeof(uint16_t) * MAX_SEARCH_STACK);
    if (gpl_lua_access_complex(&header, &depth, element, &obj_name) == 1) {
        lprintf("--complex_write: I need to write 'accum' to header (%d) at depth (%d)\n", header, depth);
        //lprintf("print (\"writing complex: header=\", %d, \"depth = \", %d, \"obj_name = \", %d)", header, depth, obj_name);
        //smart_write_data(header, depth, element, data);
        lprintf("gpl.set_element(%d, %d, %d, %d, accum)", obj_name, header, depth, *element);
    } else {
        lprintf("print (\"write complex failed\")\n");
    }
}

static void gpl_lua_load_variable() {
    int8_t extended = 0;
    uint8_t datatype;
    uint16_t varnum;
    datatype = gpl_get_byte();
    datatype &= 0x7F;
    if ((datatype & EXTENDED_VAR) != 0) {
        extended = 1;
        datatype -= EXTENDED_VAR;
    }
    if (datatype < 0x10) { // simple data type
        varnum = gpl_get_byte();
        if (extended == 1) {
            varnum *= 0x100;
            varnum += gpl_get_byte();
        }
        //load_simple_variable(datatype, varnum, 0);
        gpl_lua_load_simple_variable(datatype, varnum);
    } else {
        gpl_lua_write_complex_var();
        //lua_exit("write complex_var not implemented!\n");
    }
}

extern void gpl_lua_ifcompare(void) {
    gpl_lua_get_parameters(2);
    if (!in_compare) { lprintf("--cmp start\n"); }
    if (in_compare) { lua_depth--;}
    lprintf("%sif compare == %s then\n", 
        in_compare ? "else" : "",
        lparams.params[0]);
    in_compare = 1;
    lua_depth++;
    //lprintf("--go to %s (lparams.params[1])\n", lparams.params[1]);
}

extern void gpl_lua_clearpic(void) {
    lprintf("gpl.narrate_open(NAR_PORTRAIT, \"\", 0) -- clearpic\n");
}

extern void gpl_lua_orelse(void) {
    char buf[BUF_SIZE];
    gpl_lua_read_number(buf, BUF_SIZE);
    lprintf("--or else (%s)\n", buf);
    //lua_exit("OR ELSE?");
    //if (compared[comparePtr] == YES) {
        //move_gpl_ptr(address);
    //}
}

extern void gpl_lua_exit(void) {
    lprintf("gpl.exit()\n");
    if (lua_depth == 1) {
        in_func = 0;
        //lprintf("gpl.debug(\"return func%ld\")\n", cfunc_num);
        lua_depth--;
        lprintf("end\n");
    }
}

extern void gpl_lua_clear_los(void) {
    char buf[BUF_SIZE];
    gpl_lua_read_number(buf, BUF_SIZE);
    lprintf("--clear_los_order: need to clear #%s\n", buf);
}

extern void gpl_lua_nametonum(void) {
    char buf[BUF_SIZE];
    gpl_lua_read_number(buf, BUF_SIZE);
    if (buf[0] != '-') {
        lprintf("-%s\n", buf);
    } else {
        lprintf("%s\n", buf + 1);
    }
    //set_accumulator(gpl_lua_read_number() * -1);
}

extern void gpl_lua_numtoname(void) {
    char buf[BUF_SIZE];
    gpl_lua_read_number(buf, BUF_SIZE);
    if (buf[0] != '-') {
        lprintf("-%s\n", buf);
    } else {
        lprintf("%s\n", buf + 1);
    }
    //set_accumulator(gpl_lua_read_number() * -1);
}

extern void gpl_lua_getxy(void) {
    char buf[BUF_SIZE];
    gpl_lua_read_number(buf, BUF_SIZE);
    lprintf("-- getxyz\n");
    lprintf("gpl.set_gname(1, gpl.getX(%s))\n", buf);
    lprintf("gpl.set_gname(2, gpl.getY(%s))\n", buf);
    lprintf("gpl.set_gname(3, 0)\n");
    //get_xyz(read_number());
}

static void gpl_lua_load_simple_variable(uint16_t type, uint16_t vnum) {
    switch (type) {
        case GPL_GBIGNUM:
            //gpl_global_bnums[vnum] = (int32_t) val;
            lprintf("gpl.set_gbn(%d, accum)\n", vnum);
            break;
        case GPL_LBIGNUM:
            //gpl_local_bnums[vnum] = (int32_t) val;
            lprintf("gpl.set_lbn(%d, accum)\n", vnum);
            break;
        case GPL_GNUM:
            //gpl_global_nums[vnum] = (int16_t) val;
            lprintf("gpl.set_gn(%d,  accum)\n", vnum);
            break;
        case GPL_LNUM:
            //gpl_local_nums[vnum] = (int16_t) val;
            lprintf("gpl.set_ln(%d, accum)\n", vnum);
            break;
        case GPL_GFLAG:
            lprintf("gpl.set_gf(%d, accum)\n", vnum);
            break;
        case GPL_LFLAG:
            lprintf("gpl.set_lf(%d, accum)\n", vnum);
            break;
        default:
            lua_exit("ERROR: Unknown simple variable type! 0x%x!\n");
            break;
    }
}

static size_t gpl_lua_read_number(char *buf, const size_t size) {
    int32_t paren_level = 0;
    int8_t do_next;
    int16_t opstack[MAX_PARENS];
    int32_t accums[MAX_PARENS];
    //char taccum[BUF_SIZE];
    int16_t cop, next_op; // current operation
    int32_t cval = 0; // current value, temporary value
    size_t buf_pos = 0;
    memset(opstack, 0, sizeof(opstack));
    memset(accums, 0, sizeof(accums));
    memset(buf, 0, size); // TODO: FIXME: we shouldn't need to memset this EVERY time.
    //lprintf("---- read_number_start\n");
    do {
     //   taccum[0] = '\0';
        do_next = 0;
        //printf("buf_pos = %d\n", buf_pos);
        cop = gpl_get_byte(); // current operation
        //printf("current operation = 0x%x\n", cop);
        if (cop < 0x80) {
            cval = cop * 0x100 + gpl_get_byte();
            //debug("immediate = %d\n", cval);
            buf_pos += snprintf(buf + buf_pos, size - buf_pos, "%d", cval);
        } else {
            if (cop < OPERATOR_OFFSET) {
                if (cop & EXTENDED_VAR) { // variable is > 255
                    cop -= EXTENDED_VAR;
                    gpl_global_big_num = EXTENDED_VAR;
                } else {
                    gpl_global_big_num = 0;
                }
            }
            switch (cop) {
                case GPL_ACCM|0x80: {
                    //cval = accum;
                    lua_exit("accumulator not implemented.");
                    lprintf("accum");
                    break;
                }
                case GPL_LNAME|0x80:
                case GPL_GNAME|0x80:
                case GPL_LNUM|0x80:
                case GPL_LBYTE|0x80:
                case GPL_LBIGNUM|0x80:
                case GPL_GNUM|0x80:
                case GPL_GBYTE|0x80:
                case GPL_GBIGNUM|0x80:
                case GPL_GFLAG|0x80:
                case GPL_LFLAG|0x80:
                case GPL_GSTRING|0x80:
                case GPL_LSTRING|0x80: {
                    gpl_global_big_num += cop & 0x7F;
                    //read_simple_num_var();
                    cval = gpl_global_big_num;
                    buf_pos += gpl_lua_read_simple_num_var(buf + buf_pos, size - buf_pos);
                    //printf("updating buf_pos = %d\n", buf_pos);
                    break;
                }
                case GPL_RETVAL|0x80: {
                    //debug("GPL_RETVAL begin:\n");
                    /*
                    push_params();
                    do_gpl_command(gpl_get_byte());
                    pop_params();
                    */
                    int tpos = lua_pos;
                    char *tptr = lua_buf + lua_pos;

                    lprintf("((");
                    int cmd = gpl_get_byte();
                    do_lua_gpl_command(cmd);
                    lua_pos--;
                    lprintf("))");
                    //printf("------------------------>'%s'\n", tptr);
                    buf_pos += strlen(strncpy(buf + buf_pos, tptr, size - buf_pos));
                    //strncpy(taccum, BUF_SIZE, tptr);
                    lua_pos = tpos;
                    lua_buf[lua_pos] = '\0';
                    //debug("GPL_RETVAL end, cval = %d\n", cval);
                    //printf("updating buf_pos = %d\n", buf_pos);
                    break;
                }
                case GPL_IMMED_BIGNUM|0x80: {
                    //cval = (int32_t)((int16_t)get_word()) * 655356L 
                         //+ (int32_t)((uint16_t)get_word());
                    cval = (int32_t)gpl_lua_get_word() * (int32_t)655356L;
                    cval += (int32_t)gpl_lua_get_word();
                    buf_pos += snprintf(buf + buf_pos, size - buf_pos, "%d", cval);
                    break;
                }
                case GPL_IMMED_BYTE|0x80: {
                    cval = (int32_t)((int8_t)gpl_get_byte());
                    //debug("GPL_IMMED_BYTE, cval = %d\n", cval);
                    buf_pos += snprintf(buf + buf_pos, size - buf_pos, "%d", cval);
                    //printf("updating buf_pos = %d\n", buf_pos);
                    break;
                }
                case GPL_IMMED_WORD|0x80: {
                    lua_exit("GPL_IMMED_WORD not implemented!\n");
                    break;
                }
                case GPL_IMMED_NAME|0x80: {
                    cval = (int32_t)((int16_t)get_half_word() * -1);
                    buf_pos += snprintf(buf + buf_pos, size - buf_pos, "%d", cval);
                    //printf("updating buf_pos = %d\n", buf_pos);
                    //lua_exit("GPL_IMMED_NAME not implemented!\n");
                    break;
                }
                case GPL_COMPLEX_VAL|0x80: {
                    cval = gpl_lua_read_complex(buf, &buf_pos, size);
                    //lua_exit("GPL_COMPLEX_VAL not implemented!\n");
                    break;
                }
                case GPL_IMMED_STRING|0x80: {
                    //cval = 0;
                    //gpl_global_big_numptr = (int32_t*) read_text();
                    //debug("GPL_IMMED_STRING, cval = %d, '%s'\n", cval, (char*) gpl_global_big_numptr);
                    //TODO FIXME: This should just copy directly in!
                    char *tmp = gpl_read_text();
                    buf_pos += strlen(strncpy(buf + buf_pos, tmp, size - buf_pos));
                    //printf("updating buf_pos = %d\n", buf_pos);
                    break;
                }
                case GPL_OP_ADD:
                    buf_pos += snprintf(buf + buf_pos, size - buf_pos, " + ");
                    do_next = 1;
                    break;
                case GPL_OP_MINUS:
                    buf_pos += snprintf(buf + buf_pos, size - buf_pos, " - ");
                    do_next = 1;
                    break;
                case GPL_OP_TIMES:
                    buf_pos += snprintf(buf + buf_pos, size - buf_pos, " * ");
                    do_next = 1;
                    break;
                case GPL_OP_DIVIDE:
                    buf_pos += snprintf(buf + buf_pos, size - buf_pos, " / ");
                    do_next = 1;
                    break;
                case GPL_OP_AND:
                    buf_pos += snprintf(buf + buf_pos, size - buf_pos, " and ");
                    do_next = 1;
                    break;
                case GPL_OP_OR:
                    buf_pos += snprintf(buf + buf_pos, size - buf_pos, " or ");
                    do_next = 1;
                    break;
                case GPL_OP_EQUAL:
                    buf_pos += snprintf(buf + buf_pos, size - buf_pos, " == ");
                    do_next = 1;
                    break;
                case GPL_OP_NEQUAL:
                    buf_pos += snprintf(buf + buf_pos, size - buf_pos, " ~= ");
                    do_next = 1;
                    break;
                case GPL_OP_GTR:
                    buf_pos += snprintf(buf + buf_pos, size - buf_pos, " > ");
                    do_next = 1;
                    break;
                case GPL_OP_LESS:
                    buf_pos += snprintf(buf + buf_pos, size - buf_pos, " < ");
                    do_next = 1;
                    break;
                case GPL_OP_GTE:
                    buf_pos += snprintf(buf + buf_pos, size - buf_pos, " >= ");
                    do_next = 1;
                    break;
                case GPL_OP_LTE:
                    buf_pos += snprintf(buf + buf_pos, size - buf_pos, " <= ");
                    do_next = 1;
                    break;
                case GPL_OP_BAND:
                    buf_pos += snprintf(buf + buf_pos, size - buf_pos, " & ");
                    do_next = 1;
                    break;
                case GPL_OP_BOR:
                    buf_pos += snprintf(buf + buf_pos, size - buf_pos, " | ");
                    do_next = 1;
                    break;
                case GPL_OP_BCLR: {
                    do_next = 1;
                    buf_pos += snprintf(buf + buf_pos, size - buf_pos, " | ");
                    //lua_exit ("BCLR not implemented\n");
                    //opstack[paren_level] = cop;
                    //found_operator = 1;
                    break;
                }
                case GPL_HI_OPEN_PAREN: {
                    /*
                    lua_exit ("Basic Paren structures not implemented\n");
                    if (++paren_level >= MAX_PARENS) {
                        fprintf(stderr, "ERROR: exceeded max paren level!\n");
                        exit(1);
                    }
                    */
                    ++paren_level;
                    buf_pos += snprintf(buf + buf_pos, size - buf_pos, "(");
                    //printf("updating buf_pos = %d\n", buf_pos);
                    do_next = 1;
                    cval = 0;
                    break;
                }
                case GPL_HI_CLOSE_PAREN: {
                    //lua_exit ("Basic close Paren structures not implemented\n");
                    //cval = accums[paren_level];
                    //if (--paren_level < 0) {
                        //fprintf(stderr, "ERROR: paren level is < 0!\n");
                        //exit(1);
                    //}
                    --paren_level;
                    //printf("CLOSEbuf_pos = %d\n", buf_pos);
                    buf_pos += snprintf(buf + buf_pos, size - buf_pos, ")");
                    //printf("buf = '%s'\n", buf);
                    break;
                }
                default: {
                    printf(" UNKNOWN OP: 0x%x or 0x%x\n", cop&0x7F, cop);
                    lua_exit ("UNKNOWN OP\n");
                    break;
                }
            }
        }
    } while (do_next || 
        (((next_op = gpl_preview_byte(0)) > OPERATOR_OFFSET && next_op <= OPERATOR_LAST)
            || (paren_level > 0 && next_op == GPL_HI_CLOSE_PAREN)));
    // We need to look if that above else was executed!
    //printf("accums[0] = %d\n", accums[0]);
    //return accums[0];
    //printf("exiting with buf_pos = %d, buf = '%s'\n", buf_pos, buf);
    //lprintf("---- read_number_end, buf  '%s'\n", buf);
    return buf_pos;
}

static uint16_t gpl_lua_get_word() {
    uint16_t ret;
    ret = gpl_get_byte() * 0x100;
    ret += gpl_get_byte();
    return ret;
}

static uint8_t gpl_lua_access_complex(int16_t *header, uint16_t *depth, uint16_t *element, int32_t *obj_name) {
    uint16_t i;
    
    *obj_name = gpl_lua_get_word();
    //debug("header = %d, depth = %d, element = %d, obj_name = %d\n", *header, *depth, *element, obj_name);
    if (*obj_name < 0x8000) {
        lprintf("--access_complex: I need to convert from ID to header!\n");
    } else {
        lprintf("--access_complex: I need to set the *head to the correct view\n");
        switch (*obj_name & 0x7FFF) {
            case 0x25: // POV, which is active character.
                //lprintf("obj = gpl.get_pov(%d) -- %d \n", *obj_name & 0x7FFF, *obj_name);
                lprintf("--access_complex: get POV, valid obj_name(%d), need to set header\n", *obj_name & 0x7FFF);
                break;
            case 0x26: // ACTIVE
                lprintf("--access_complex: get ACTIVE, valid obj_name(%d), need to set header\n", *obj_name & 0x7FFF);
                break;
            case 0x27: // PASSIVE
                //lprintf("obj = gpl.get_gname(%d) -- passive\n", GNAME_PASSIVE);
                lprintf("--access_complex: get PASSIVE, valid obj_name(%d), need to set header\n", *obj_name & 0x7FFF);
                break;
            case 0x28: // OTHER
                lprintf("--access_complex: get OTHER, valid obj_name(%d), need to set header\n", *obj_name & 0x7FFF);
                //lprintf("other = obj \n");
                break;
            case 0x2C: // OTHER1
                lprintf("--access_complex: get OTHER1, valid obj_name(%d), need to set header\n", *obj_name & 0x7FFF);
                break;
            case 0x2B: // THING
                lprintf("--access_complex: get THING, valid obj_name(%d), need to set header\n", *obj_name & 0x7FFF);
                break;
            default:
                lprintf("--access_complex: unknown arg: %d", *obj_name & 0x7FFF);
                return 0;
        }
    }
    *depth = gpl_get_byte();
    //debug("--access_compledx:depth = %d\n", *depth);
    for (i = 1; i <= *depth; i++) {
        element[i-1] = gpl_get_byte();
        //debug("--access_complex:element[%d] = %d\n", i-1, element[i-1]);
    }

    return 1;
}

int32_t gpl_lua_get_complex_data(int16_t header, uint16_t depth, uint16_t *element) {
    /*
    uint16_t i = 0;
    int32_t ret;

    ret = header; // start with header.

    do {
        ret = data_field((int16_t) ret, i);
        element++;
        i++;
    } while ((i < depth));
    */
    //} while (/*(field_error == 0) && */ (i < depth));
    //return ret;
    lprintf("--get_complex_data: header:%d, depth:%d element:%d\n", header, depth, *element);
    return 1;
}

static int32_t gpl_lua_read_complex(char *buf, size_t *buf_pos, const size_t size) {
    int32_t ret = 0;
    uint16_t depth = 0;
    int16_t header = 0;
    uint16_t element[MAX_SEARCH_STACK];
    int32_t obj_name;
    memset(element, 0x0, sizeof(uint16_t) * MAX_SEARCH_STACK);

    if (gpl_lua_access_complex(&header, &depth, element, &obj_name) == 1) {
        // So if depth == 0 then get type, is the type, 1 is the id (like -319)
        switch (depth) {
            case 0:
                //lprintf("gpl.get_type(obj)\n"); 
                *buf_pos += snprintf(buf + *buf_pos, size - *buf_pos, "gpl.get_type(obj)");
                break;
            case 1:
                //lprintf("gpl.get_id(obj)\n");
                //printf("read_complex: %d %d %d\n", obj_name, header, depth);
                //*buf_pos += snprintf(buf + *buf_pos, size - *buf_pos, "gpl.get_id(obj, %d)", obj_name);
                *buf_pos += snprintf(buf + *buf_pos, size - *buf_pos,
                     "gpl.get_element(%d, %d, %d, %d)", obj_name, header, depth, *element);
                break;
            default:
                *buf_pos += snprintf(buf + *buf_pos, size - *buf_pos, "gpl.read_complex(%d, %d, %d)", obj_name, header, depth);
                break;
        }
        lprintf("--read_complex:reading header (%d) at depth (%d)\n", header, depth);
        lprintf("--gpl.get_element(%d, %d, %d, %d)\n", obj_name, header, depth, *element);
        ret = gpl_lua_get_complex_data(header, depth, element);
        return ret;
    } else {
        lua_exit("--read_complex else not implemented!\n");
    }

    return ret;
}

int gpl_lua_read_simple_num_var(char *buf, const size_t buf_size) {
    int16_t temps16 = gpl_get_byte();

    if ((gpl_global_big_num & EXTENDED_VAR) != 0) {
        temps16 *= 0x100;
        temps16 += gpl_get_byte();
        gpl_global_big_num &= 0x3F;
    }

    switch(gpl_global_big_num) {
        case GPL_GFLAG: {
            return snprintf(buf, buf_size, "gpl.get_gf(%d)", temps16);
            break;
        }
        case GPL_LFLAG: {
            return snprintf(buf, buf_size, "gpl.get_lf(%d)", temps16);
            break;
        }
        case GPL_GNUM: {
            return snprintf(buf, buf_size, "gpl.get_gn(%d)", temps16);
            break;
        }
        case GPL_LNUM: {
            return snprintf(buf, buf_size, "gpl.get_ln(%d)", temps16);
            break;
        }
        case GPL_GBIGNUM: {
            return snprintf(buf, buf_size, "gpl.get_gbn(%d)", temps16);
            break;
        }
        case GPL_LBIGNUM: {
            return snprintf(buf, buf_size, "gpl.get_lbn(%d)", temps16);
            break;
        }
        case GPL_GNAME: {
            if (temps16 >= 0x20 && temps16 < 0x2F) {
                //return snprintf(buf, buf_size, "global_simple_var[%d]", temps16 - 0x20);
                //return snprintf(buf, buf_size, "\"gpl.get_gname(%d)\"", temps16 - 0x20);
                return snprintf(buf, buf_size, "gpl.get_gname(%d)", temps16 - 0x20);
            } else {
                lua_exit("ERROR: No variable GNAME!!!\n");
            }
            //debug("reading gname @ %d is equal to %d\n", temps16, gpl_global_big_num);
            //lua_exit ("GPL_GNAME not implemented.\n");
            break;
        }
        case GPL_GSTRING: {
            //gpl_global_big_numptr = (int32_t*) gpl_global_strings[temps16];
            //debug("reading gstring @ %d is equal to '%s'\n", temps16, (char*)gpl_global_big_numptr);
            //lua_exit ("GPL_GString not implemented.\n");
            /*
            gpl_global_big_numptr = (int32_t*) gpl_global_strings[temps16];
            */
            return snprintf(buf, buf_size, "gpl.get_gstr(%d)", temps16);
            break;
        }
        case GPL_LSTRING: {
            return snprintf(buf, buf_size, "gpl.get_lstr(%d)", temps16);
            //lua_exit ("GPL_LString not implemented.\n");
            /*
            gpl_global_big_numptr = (int32_t*) gpl_local_strings[temps16];
            debug("reading lstring @ %d is equal to '%s'\n", temps16, (char*)gpl_global_big_numptr);
            */
            break;
        }
        default:
            lua_exit("ERROR: Unknown type in read_simple_num_var.\n");
            break;
    }
    return 0;
}
