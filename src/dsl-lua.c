#include "dsl-lua.h"
#include "ds-state.h"
#include "ds-string.h"
#include "dsl-var.h"
#include "dsl-manager.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>

static int print_cmd();
static int varnum = 0;
static int funcnum = 0;
static int in_func = 0;
static int in_compare = 0;
static size_t cfunc_num = 0;
static char is_master_mas = 0;
static void dsl_lua_load_variable();
static void dsl_lua_load_simple_variable(uint16_t type, uint16_t vnum);
static int dsl_lua_read_simple_num_var(char *buf, const size_t buf_size);
static uint8_t dsl_lua_access_complex(int16_t *header, uint16_t *depth, uint16_t *element, int32_t *obj_name);
static int32_t dsl_lua_read_complex(char *buf, size_t *buf_pos, const size_t size);
static uint16_t dsl_lua_get_word();
static void validate_number(const char *num, const char *message);
static void print_label();

void dsl_lua_byte_dec(void);
void dsl_lua_word_dec(void);
void dsl_lua_long_dec(void);
void dsl_lua_byte_inc(void);
void dsl_lua_word_inc(void);
void dsl_lua_long_inc(void);
void dsl_lua_changemoney(void);
void dsl_lua_setvar(void);
void dsl_lua_toggle_accum(void);
void dsl_lua_getstatus(void);
void dsl_lua_getlos(void);
void dsl_lua_long_times_equal(void);
void dsl_lua_jump(void);
void dsl_lua_local_sub(void);
void dsl_lua_global_sub(void);
void dsl_lua_local_ret(void);
void dsl_lua_compare(void);
void dsl_lua_pdamage(void);
void dsl_lua_cmpend(void);
void dsl_lua_wait(void);
void dsl_lua_while(void);
void dsl_lua_wend(void);
void dsl_lua_attacktrigger(void);
void dsl_lua_looktrigger(void);
void dsl_lua_load_accum(void);
void dsl_lua_global_ret(void);
void dsl_lua_usetrigger(void);
void dsl_lua_ifcompare(void);
void dsl_lua_clearpic(void);
void dsl_lua_orelse(void);
void dsl_lua_exit(void);
void dsl_lua_fetch(void);
void dsl_lua_search(void);
void dsl_lua_getparty(void);
void dsl_lua_fight(void);
void dsl_lua_flee(void);
void dsl_lua_follow(void);
void dsl_lua_getyn(void);
void dsl_lua_give(void);
void dsl_lua_go(void);
void dsl_lua_input_bignum(void);
void dsl_lua_goxy(void);
void dsl_lua_readorders(void);
void dsl_lua_if(void);
void dsl_lua_else(void);
void dsl_lua_setrecord(void);
void dsl_lua_setother(void);
void dsl_lua_menu(void);
void dsl_lua_setthing(void);
void dsl_lua_print_string(void);
void dsl_lua_print_number(void);
void dsl_lua_printnl(void);
void dsl_lua_rand(void);
void dsl_lua_showpic(void);
void dsl_lua_skillroll(void);
void dsl_lua_statroll(void);
void dsl_lua_string_compare(void);
void dsl_lua_match_string(void);
void dsl_lua_take(void);
void dsl_lua_sound(void);
void dsl_lua_tport(void);
void dsl_lua_bitsnoop(void);
void dsl_lua_award(void);
void dsl_lua_request(void);
void dsl_lua_hunt(void);
void dsl_lua_source_trace(void);
void dsl_lua_shop(void);
void dsl_lua_end_control(void);
void dsl_lua_input_string(void);
void dsl_lua_input_number(void);
void dsl_lua_input_money(void);
void dsl_lua_joinparty(void);
void dsl_lua_leaveparty(void);
void dsl_lua_lockdoor(void);
void dsl_lua_nextto(void);
void dsl_lua_inlostrigger(void);
void dsl_lua_notinlostrigger(void);
void dsl_lua_move_tiletrigger(void);
void dsl_lua_continue(void);
void dsl_lua_log(void);
void dsl_lua_damage(void);
void dsl_lua_source_line_num(void);
void dsl_lua_drop(void);
void dsl_lua_passtime(void);
void dsl_lua_door_tiletrigger(void);
void dsl_lua_move_boxtrigger(void);
void dsl_lua_door_boxtrigger(void);
void dsl_lua_pickup_itemtrigger(void);
void dsl_lua_talktotrigger(void);
void dsl_lua_noorderstrigger(void);
void dsl_lua_usewithtrigger(void);
void dsl_lua_clear_los(void);
void dsl_lua_nametonum(void);
void dsl_lua_numtoname(void);
void dsl_lua_getxy(void);
void dsl_lua_clone(void);
void dsl_lua_word_plus_equal(void);
void dsl_lua_word_minus_equal(void);
void dsl_lua_word_times_equal(void);
void dsl_lua_word_divide_equal(void);
void dsl_lua_long_plus_equal(void);
void dsl_lua_long_minus_equal(void);
void dsl_lua_get_range(void);

static size_t dsl_lua_read_number(char *buf, const size_t size);
uint16_t get_half_word();

/********************DSL DEFINES ***************************************/
#define DSL_IN_LOS        (1)
#define DSL_NOT_IN_LOS    (2)

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
static size_t dsl_lua_start_ptr = 0;

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
    return !(strncmp("dsl.get_gstr(", str, 12) == 0);
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

static void dsl_lua_get_parameters(int16_t amt) {
    for (int16_t i = 0; i < amt; i++) {
        dsl_lua_read_number(lparams.params[i], BUF_SIZE);
        //printf("parameter[%d] = %s\n", i, lparams.params[i]);
    }
}

static void print_label() {
    if (collect_labels) { return; }

    uint64_t label = ((size_t)get_data_ptr()) - dsl_lua_start_ptr;
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
} dsl_lua_operation_t;

static void dsl_lua_string_copy();
static void dsl_lua_load_var();

dsl_lua_operation_t dsl_lua_operations[] = {
    { NULL, "dsl zero" }, // 0x0
    { NULL, "dsl long divide equal" }, // 0x1
    { dsl_lua_byte_dec, "dsl byte dec" }, // 0x2
    { dsl_lua_word_dec, "dsl word dec" }, // 0x3
    { dsl_lua_long_dec, "dsl long dec" }, // 0x4
    { dsl_lua_byte_inc, "dsl byte inc" }, // 0x5
    { dsl_lua_word_inc, "dsl word inc" }, // 0x6
    { dsl_lua_long_inc, "dsl long inc" }, // 0x7
    { dsl_lua_hunt, "dsl hunt" }, // 0x8
    { dsl_lua_getxy, "dsl getxy" }, // 0x9
    { dsl_lua_string_copy, "dsl string copy" }, // 0xA
    { dsl_lua_pdamage, "dsl p damage" }, // 0xB
    { dsl_lua_changemoney, "dsl changemoney" }, // 0xC
    { dsl_lua_setvar, "dsl setvar" }, // 0xD
    { dsl_lua_toggle_accum, "dsl toggle accum" }, // 0xE
    { dsl_lua_getstatus, "dsl getstatus" }, // 0xF
    { dsl_lua_getlos, "dsl getlos" }, // 0x10
    { dsl_lua_long_times_equal, "dsl long times equal" }, // 0x11
    { dsl_lua_jump, "dsl jump" }, // 0x12
    { dsl_lua_local_sub, "dsl local sub" }, // 0x13
    { dsl_lua_global_sub, "dsl global sub" }, // 0x14
    { dsl_lua_local_ret, "dsl local ret" }, // 0x15
    { dsl_lua_load_var, "dsl load variable" }, // 0x16
    { dsl_lua_compare, "dsl compare" }, // 0x17
    { dsl_lua_load_accum, "dsl load accum" }, // 0x18
    { dsl_lua_global_ret, "dsl global ret" }, // 0x19
    { dsl_lua_nextto, "dsl nextto" }, // 0x1A
    { dsl_lua_inlostrigger, "dsl inlostrigger" }, // 0x1B
    { dsl_lua_notinlostrigger, "dsl notinlostrigger" }, // 0x1C
    { dsl_lua_clear_los, "dsl clear los" }, // 0x1D
    { dsl_lua_nametonum, "dsl nametonum" }, // 0x1E
    { dsl_lua_numtoname, "dsl numtoname" }, // 0x1F
    { dsl_lua_bitsnoop, "dsl bitsnoop" }, // 0x20
    { dsl_lua_award, "dsl award" }, // 0x21
    { dsl_lua_request, "dsl request" }, // 0x22
    { dsl_lua_source_trace, "dsl source trace" }, // 0x23
    { dsl_lua_shop, "dsl shop" }, // 0x24
    { dsl_lua_clone, "dsl clone" }, // 0x25
    { NULL, "dsl default" }, // 0x26
    { dsl_lua_ifcompare, "dsl ifcompare" }, // 0x27
    { NULL, "dsl trace var" }, // 0x28
    { dsl_lua_orelse, "dsl orelse" }, // 0x29
    { dsl_lua_clearpic, "dsl clearpic" }, // 0x2A
    { dsl_lua_continue, "dsl continue" }, // 0x2B
    { dsl_lua_log, "dsl log" }, // 0x2C
    { dsl_lua_damage, "dsl damage" }, // 0x2D
    { dsl_lua_source_line_num, "dsl source line num" }, // 0x2E
    { dsl_lua_drop, "dsl drop" }, // 0x2F
    { dsl_lua_passtime, "dsl passtime" }, // 0x30
    { dsl_lua_exit, "dsl exit dsl" }, // 0x31
    { dsl_lua_fetch, "dsl fetch" }, // 0x32
    { dsl_lua_search, "dsl search" }, // 0x33
    { dsl_lua_getparty, "dsl getparty" }, // 0x34
    { dsl_lua_fight, "dsl fight" }, // 0x35
    { dsl_lua_flee, "dsl flee" }, // 0x36
    { dsl_lua_follow, "dsl follow" }, // 0x37
    { dsl_lua_getyn, "dsl getyn" }, // 0x38
    { dsl_lua_give, "dsl give" }, // 0x39
    { dsl_lua_go, "dsl go" }, // 0x3A
    { dsl_lua_input_bignum, "dsl input bignum" }, // 0x3B
    { dsl_lua_goxy, "dsl goxy" }, // 0x3C
    { dsl_lua_readorders, "dsl readorders" }, // 0x3D
    { dsl_lua_if, "dsl if" }, // 0x3E
    { dsl_lua_else, "dsl else" }, // 0x3F
    { dsl_lua_setrecord, "dsl setrecord" }, // 0x40
    { dsl_lua_setother, "dsl setother" }, // 0x41
    { dsl_lua_input_string, "dsl input string" }, // 0x42
    { dsl_lua_input_number, "dsl input number" }, // 0x43
    { dsl_lua_input_money, "dsl input money" }, // 0x44
    { dsl_lua_joinparty, "dsl joinparty" }, // 0x45
    { dsl_lua_leaveparty, "dsl leaveparty" }, // 0x46
    { dsl_lua_lockdoor, "dsl lockdoor" }, // 0x47
    { dsl_lua_menu, "dsl menu" }, // 0x48
    { dsl_lua_setthing, "dsl setthing" }, // 0x49
    { NULL, "dsl default" }, // 0x4A
    { NULL, "dsl local sub trace" }, // 0x4B
    { NULL, "dsl default" }, // 0x4C
    { NULL, "dsl default" }, // 0x4D
    { NULL, "dsl default" }, // 0x4E
    { dsl_lua_print_string, "dsl print string" }, // 0x4F
    { dsl_lua_print_number, "dsl print number" }, // 0x50
    { dsl_lua_printnl, "dsl printnl" }, // 0x51
    { dsl_lua_rand, "dsl rand" }, // 0x52
    { NULL, "dsl default" }, // 0x53
    { dsl_lua_showpic, "dsl showpic" }, // 0x54
    { NULL, "dsl default" }, // 0x55
    { NULL, "dsl default" }, // 0x56
    { NULL, "dsl default" }, // 0x57
    { dsl_lua_skillroll, "dsl skillroll" }, // 0x58
    { dsl_lua_statroll, "dsl statroll" }, // 0x59
    { dsl_lua_string_compare, "dsl string compare" }, // 0x5A
    { dsl_lua_match_string, "dsl match string" }, // 0x5B
    { dsl_lua_take, "dsl take" }, // 0x5C
    { dsl_lua_sound, "dsl sound" }, // 0x5D
    { dsl_lua_tport, "dsl tport" }, // 0x5E
    { NULL, "dsl music" }, // 0x5F
    { NULL, "dsl default" }, // 0x60
    { dsl_lua_cmpend, "dsl cmpend" }, // 0x61
    { dsl_lua_wait, "dsl wait" }, // 0x62
    { dsl_lua_while, "dsl while" }, // 0x63
    { dsl_lua_wend, "dsl wend" }, // 0x64
    { dsl_lua_attacktrigger, "dsl attacktrigger" }, // 0x65
    { dsl_lua_looktrigger, "dsl looktrigger" }, // 0x66
    { dsl_lua_end_control, "dsl endif" }, // 0x67
    { dsl_lua_move_tiletrigger, "dsl move tiletrigger" }, // 0x68
    { dsl_lua_door_tiletrigger, "dsl door tiletrigger" }, // 0x69
    { dsl_lua_move_boxtrigger, "dsl move boxtrigger" }, // 0x6A
    { dsl_lua_door_boxtrigger, "dsl door boxtrigger" }, // 0x6B
    { dsl_lua_pickup_itemtrigger, "dsl pickup itemtrigger" }, // 0x6C
    { dsl_lua_usetrigger, "dsl usetrigger" }, // 0x6D
    { dsl_lua_talktotrigger, "dsl talktotrigger" }, // 0x6E
    { dsl_lua_noorderstrigger, "dsl noorderstrigger" }, // 0x6F
    { dsl_lua_usewithtrigger, "dsl usewithtrigger" }, // 0x70
    { NULL, "dsl default" }, // 0x71
    { NULL, "dsl default" }, // 0x72
    { NULL, "dsl default" }, // 0x73
    { NULL, "dsl default" }, // 0x74
    { NULL, "dsl default" }, // 0x75
    { NULL, "dsl byte plus equal" }, // 0x76
    { NULL, "dsl byte minus equal" }, // 0x77
    { NULL, "dsl byte times equal" }, // 0x78
    { NULL, "dsl byte divide equal" }, // 0x79
    { dsl_lua_word_plus_equal, "dsl word plus equal" }, // 0x7A
    { dsl_lua_word_minus_equal, "dsl word minus equal" }, // 0x7B
    { dsl_lua_word_times_equal, "dsl word times equal" }, // 0x7C
    { dsl_lua_word_divide_equal, "dsl word divide equal" }, // 0x7D
    { dsl_lua_long_plus_equal, "dsl long plus equal" }, // 0x7E
    { dsl_lua_long_minus_equal, "dsl long minus equal" }, // 0x7F
    { dsl_lua_get_range, "dsl get range" }, // 0x80
};

static void do_lua_dsl_command(uint8_t cmd) {
    //fflush(stdout);
    //debug("[%p:%d]command byte = 0x%x (%s)\n", (void*)get_data_start_ptr(),
        //(int32_t) (get_data_ptr() - get_data_start_ptr()), cmd, dsl_operations[cmd].name);
    //exit_dsl = 0;
    //printf("cmd = 0x%x (do_lua_dsl_command.)\n", cmd);
    (*dsl_lua_operations[cmd].func)();
    //print_vars(0);
}

/*
 * Warning: This function returns an internal buffer that *does* change
 * with subsequent calls. Make any copies as needed!
 */
static void dsl_lua_pass(unsigned char *dsl, const size_t len, const int pass_num) {
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

    push_data_ptr(dsl);
    set_data_ptr(dsl, dsl);
    dsl_lua_start_ptr = (size_t)dsl;
    const size_t start = (size_t)get_data_ptr();
    size_t diff = (size_t)get_data_ptr() - start;
    is_master_mas = (script_id == 99 && is_mas);
    //printf("dsl = %p, len = %ld\n", dsl, len);
    while (diff < len && print_cmd()) {
        diff = (size_t)get_data_ptr() - start;
    }
    while (lua_depth-- > 0) {
        lprintf("end\n");
    }
    //printf("tranversed = %ld, len = %ld\n", (size_t)get_data_ptr() - (size_t)start, len);
    pop_data_ptr();
}

char* dsl_lua_print(const size_t _script_id, const int _is_mas, size_t *script_len) {
    //size_t len;
    unsigned char *dsl = NULL;
    script_id = _script_id;
    is_mas = _is_mas;

    gff_chunk_header_t chunk = gff_find_chunk_header(DSLDATA_GFF_INDEX,
        is_mas ? GFF_MAS : GFF_GPL,
        script_id);
    dsl = malloc(chunk.length);
    if (!dsl) {
        error ("Unable to alloc for dsl script!\n");
        exit(1);
    }
    gff_read_chunk(DSLDATA_GFF_INDEX, &chunk, dsl, chunk.length);
    
/*
    lua_pos = 0;
    lua_depth = 0; 
    lua_buf[0] = '\0';
    funcnum = 0;
    in_func = 0;
    varnum = 0;

    push_data_ptr(dsl);
    set_data_ptr(dsl, dsl);
    dsl_lua_start_ptr = (size_t)dsl;
    const size_t start = (size_t)get_data_ptr();
    size_t diff = (size_t)get_data_ptr() - start;
    is_master_mas = (script_id == 99 && is_mas);
    //printf("dsl = %p, len = %ld\n", dsl, len);
    while (diff < len && print_cmd()) {
        diff = (size_t)get_data_ptr() - start;
    }
    while (lua_depth-- > 0) {
        lprintf("end\n");
    }
    //printf("tranversed = %ld, len = %ld\n", (size_t)get_data_ptr() - (size_t)start, len);
    pop_data_ptr();
    */
    //dsl_lua_pass(dsl, len, 0);
    //dsl_lua_pass(dsl, len, 1);
    dsl_lua_pass(dsl, chunk.length, 0);
    dsl_lua_pass(dsl, chunk.length, 1);
    *script_len = lua_pos - 1;
    lua_buf[lua_pos - 1] = '\0';
    free(dsl);
    return lua_buf;
}

static int print_cmd() {
    if (!in_func) {
        //lprintf("function func%d (accum) -- address %ld\n", funcnum++, ((size_t)get_data_ptr()) - dsl_lua_start_ptr);
        cfunc_num = ((size_t)get_data_ptr()) - dsl_lua_start_ptr;
        lprintf("function func%ld ()\n", cfunc_num);
        //lprintf("function %c%ldfunc%ld ()\n", 
            //is_mas ? 'm' : 'g',
            //script_id, ((size_t)get_data_ptr()) - dsl_lua_start_ptr);
        lua_depth++;
        in_func = 1;
    } else {
        print_label();
    }
    uint8_t command = get_byte();
    if (!dsl_lua_operations[command].func) {
        printf("Lua code so far:\n%s\n", lua_buf);
        printf("Unimplemented command = 0x%2x\n", command);
        exit(1);
        return 0;
    }
    //printf("print_cmd: command = 0x%x\n", command);
    (*dsl_lua_operations[command].func)();
    return 1;
}

static void dsl_lua_string_copy() {
    char buf[BUF_SIZE];

    dsl_lua_get_parameters(2);
    lprintf("var%d = \"%s\"\n", varnum++, (char*)lparams.params[1]);
    if (strncmp("dsl.get_gstr(", lparams.params[0], 13) == 0) {
        strncpy(buf, lparams.params[0], BUF_SIZE);
        buf[4] = 's';
        buf[strlen(buf) - 1] = '\0';
        lprintf("%s, var%d)\n", buf, varnum - 1);
    } else {
        lprintf("--string_copy, unknown parameter '%s'\n", lparams.params[0]);
    }
}

// Does this variable come from parameter passing?
void dsl_lua_load_accum(void) {
    char buf[BUF_SIZE];
    dsl_lua_read_number(buf, BUF_SIZE);
    lprintf("accum = %s -- load_accum\n", buf);
}

void dsl_lua_global_ret(void) {
    in_func = 0;
    lua_depth--;
    lprintf("end --return\n");
    if (lua_depth < 0) { lua_depth = 0; }
}

void dsl_lua_nextto(void) {
    dsl_lua_get_parameters(2);
    lprintf("accum = dsl.objects_are_adjacent(%s, %s)\n", lparams.params[0], lparams.params[1]);
}

void dsl_lua_inlostrigger(void) {
    dsl_lua_get_parameters(4);
    uint32_t addr = atoi(lparams.params[0]);
    uint32_t file = atoi(lparams.params[1]);
    uint32_t name = atoi(lparams.params[2]);
    //uint32_t global = in_mas ? 1 : 0;
    //global_addr_name(&param);
    //set_los_order(DSL_IN_LOS, param.val[3]);
    //lprintf("-- need to set line of sight order on %d\n", param.val[3]);
    lprintf("-- If in line of sight of %d goto file %d:%d (?: %s)\n", name, file, addr, lparams.params[3]);
}

void dsl_lua_notinlostrigger(void) {
    dsl_lua_get_parameters(4);
    //global_addr_name(&param);
    //set_los_order(DSL_NOT_IN_LOS, param.val[3]);
    lprintf("-- need to set not line of sight order on %d\n", param.val[3]);
}

void dsl_lua_move_tiletrigger(void) {
    dsl_lua_get_parameters(5);
    lprintf("dsl.tile_trigger(%s, %s, %s, %s, %s)\n",
        lparams.params[0], lparams.params[1], lparams.params[3], lparams.params[2], lparams.params[4]);
}

void dsl_lua_continue(void) {
    lprintf("dsl.narrate_open(NAR_ADD_MENU, \"Press Continue\", 0)\n");
    lprintf("dsl.narrate_open(NAR_ADD_MENU, \"Continue\", 0)\n");
    lprintf("dsl.narrate_open(NAR_SHOW_MENU, \"\", 0)\n");
    lprintf("--wait for input from user!\n");
}

void dsl_lua_log(void) {
    lua_exit("dsl_log not implemented.\n");
}

static void do_lua_damage(int is_percent) {
    dsl_lua_get_parameters(2);
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

void dsl_lua_damage(void) {
    do_lua_damage(0);
}

void dsl_lua_source_line_num(void) {
    command_implemented = 0;
    lua_exit("source line num not implemented\n");
}

void dsl_lua_drop(void) {
    dsl_lua_get_parameters(3);
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
    lprintf("accum = dsl.drop(%s, %s, %s) -- %s needs to drop %s of %s\n",
       lparams.params[0], lparams.params[1], lparams.params[2],
       lparams.params[0], lparams.params[1], lparams.params[2]);
    lua_depth--;
    lprintf("end -- drop\n");
}

void dsl_lua_passtime(void) {
    lua_exit("dsl_passtime not implemented\n");
}

void dsl_lua_door_tiletrigger(void) {
    lua_exit("Can't parse door_tiletrigger yet.\n");
    lprintf("--unimplmented move box trigger.\n");
}

void dsl_lua_move_boxtrigger(void) {
    dsl_lua_get_parameters(7);
    lprintf("dsl.box_trigger(%s, %s, %s, %s, %s, %s, %s)\n",
        lparams.params[0], lparams.params[1], lparams.params[2], lparams.params[3], lparams.params[4],
        lparams.params[5], lparams.params[6]);
}

void dsl_lua_door_boxtrigger(void) {
    lua_exit("Can't parse lua_door_boxtrigger yet.\n");
    lprintf("--unimplmented door box trigger.\n");
    lprintf("dsl.box_trigger(%s, %s, %s, %s, %s, %s, %s)\n",
        lparams.params[0], lparams.params[1], lparams.params[2], lparams.params[3], lparams.params[4],
        lparams.params[5], lparams.params[6]);
}

void dsl_lua_pickup_itemtrigger(void) {
    dsl_lua_get_parameters(3);
    //global_addr_name(&param);
    lprintf("--unimplmented pickup item trigger.\n");
}

void dsl_lua_talktotrigger(void) {
    dsl_lua_get_parameters(3);
    char *addr = lparams.params[0];
    char *file = lparams.params[1];
    char *name = lparams.params[2];
    int is_global = (this_gpl_file == GLOBAL_MAS) && (this_gpl_type == MASFILE);

    lprintf("dsl.talk_to_trigger(%s, %s, %s, %d)\n",
        name, file, addr, is_global);
    //lprintf("When I %s to '%s' (%s) goto file: %s, addr: %s, global = %d\n",
        //"talk to", name, name, file, addr, is_global);
    //debug("When I %s to '%s' (%d) goto file: %d, addr: %d, global = %d\n",
        //"talk to", get_so_name(so), new_name.name,
        //new_name.file, new_name.addr, new_name.global);
    //global_addr_name(&param);
}

void dsl_lua_noorderstrigger(void) {
    dsl_lua_get_parameters(3);
    lprintf("dsl.noorders_trigger%s(%s, %s, %s)\n",
        is_master_mas ? "_global" : "",
        lparams.params[2], lparams.params[1], lparams.params[0]);
}

void dsl_lua_usewithtrigger(void) {
    dsl_lua_get_parameters(4);
    lprintf("dsl.use_with_trigger%s(%s, %s, %s, %s)\n",
        is_master_mas ? "_global" : "",
        lparams.params[0], lparams.params[1], lparams.params[3], lparams.params[2]);
}

void dsl_lua_cmpend(void) {
    if (in_compare) {
        lua_depth--;
        lprintf("end\n");
        in_compare = 0;
    } else {
        lprintf("-- warning: encountered cmpend, but not in cmpend!\n");
    }
}

void dsl_lua_wait(void) {
    char buf[BUF_SIZE];
    dsl_lua_read_number(buf, BUF_SIZE);
    lprintf("dsl.set_order(%s, WAIT, 0, 0)\n", buf);
}

void dsl_lua_while(void) {
    dsl_lua_get_parameters(1);
    //lprintf("while accum == true do\n");
    lprintf("if accum == true then -- \"while\" loop\n");
    lua_depth++;
}

void dsl_lua_wend(void) {
    char buf[BUF_SIZE];
    dsl_lua_read_number(buf, BUF_SIZE);
    lua_goto(buf);
    lua_depth--;
    lprintf("end\n");
}

void dsl_lua_attacktrigger(void) {
    dsl_lua_get_parameters(3);
    //global_addr_name(&param);
    //generic_name_trigger(ATTACK_CHECK_INDEX);
    lprintf("dsl.attack_trigger%s(%s, %s, %s)\n",
        is_master_mas ? "_global" : "",
        lparams.params[2], lparams.params[1], lparams.params[0]);
}

void dsl_lua_looktrigger(void) {
    dsl_lua_get_parameters(3);
    //global_addr_name(&param);
    //generic_name_trigger(LOOK_CHECK_INDEX);
    lprintf("dsl.look_trigger%s(%s, %s, %s)\n",
        is_master_mas ? "_global" : "",
        lparams.params[2], lparams.params[1], lparams.params[0]);
}

void dsl_lua_usetrigger(void) {
    dsl_lua_get_parameters(3);
    lprintf("dsl.use_trigger%s(%s, %s, %s) -- When using %s go to file %s address %s\n",
        is_master_mas ? "_global" : "",
        lparams.params[2], lparams.params[1], lparams.params[0],
        lparams.params[2], lparams.params[1], lparams.params[0]);
}

void dsl_lua_fetch(void) {
    dsl_lua_get_parameters(2);
    lprintf("dsl.set_orders(%s, FETCH, %s, 0)\n", lparams.params[1], lparams.params[0]);
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

void dsl_lua_search(void) {
    char object[BUF_SIZE];
    char buf[BUF_SIZE - 128];
    //uint32_t answer = 0L;
    dsl_lua_read_number(object, BUF_SIZE);
    int16_t low_field = get_byte();
    int16_t high_field = get_byte();
    int16_t field_level = -1, depth = 1;
    int32_t search_for = 0, temp_for = 0;
    uint16_t field[16];
    uint8_t type = 0;
    char *i;
    do {
        if (peek_one_byte() == OBJ_QUALIFIER) {
            get_byte();
        }
        field_level++;
        field[field_level] = get_byte();
        type = get_byte();
        if (type >= EQU_SEARCH && type <= GT_SEARCH) {
            dsl_lua_read_number(buf, BUF_SIZE - 128);
            validate_number(buf, "dsl_lua_search");
            //temp_for = read_number();
            temp_for = atoi(buf);
            if (field_level > 0) {
                field[++field_level] = (uint16_t) temp_for;
            } else {
                search_for = temp_for;
            }
        }
        depth--;
    } while ( peek_one_byte() == OBJ_QUALIFIER);

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

void dsl_lua_getparty(void) {
    char buf[BUF_SIZE];
    dsl_lua_read_number(buf, BUF_SIZE);
    lprintf("dsl.get_party(%s)\n", buf);
    //lprintf("gOther = %s\n", buf);
    //lprintf("accum = %s\n", buf);
}

void dsl_lua_fight(void) {
    lprintf("--I need to enter fight mode!\n");
    //dsl_exit_dsl();
}

void dsl_lua_flee(void) {
    char buf[BUF_SIZE];
    dsl_lua_read_number(buf, BUF_SIZE);
    lprintf("--I need to flee!\n");
    //set_orders(read_number(), FLEE, 0, 0);
}

void dsl_lua_follow(void) {
    dsl_lua_get_parameters(2);
    lprintf("--I need to follow\n");
    //set_orders(param.val[1], FOLLOW, param.val[0], 0);
}

void dsl_lua_getyn(void) {
    lprintf("dsl.ask_yes_no()\n");
}

void dsl_lua_give(void) {
    dsl_lua_get_parameters(4);
    //lprintf("--need to give and set accumulator!");
    //set_accumulator(give(param.val[0], param.val[1], param.val[2], param.val[3], DSL_NEW_SLOT));
    lprintf("dsl.give(%s, %s, %s, %s, DSL_NEW_SLOT)\n",
        lparams.params[0], lparams.params[1], lparams.params[2], lparams.params[3]);
}

void dsl_lua_go(void) {
    dsl_lua_get_parameters(2);
    lprintf("--need to give GO order!\n");
    lprintf("print (\"GO ORDER\")\n");
    //set_orders(param.val[1], GO_OBJECT, param.val[0], 0);
}

void dsl_lua_input_bignum(void) {
    lua_exit("input bignum not implemented\n");
}

void dsl_lua_goxy(void) {
    dsl_lua_get_parameters(3);
    lprintf("--I need to goxy1\n");
    //set_orders(param.val[2], GOXY, param.val[0], param.val[1]);
}

void dsl_lua_readorders(void) {
    char buf[BUF_SIZE];
    dsl_lua_read_number(buf, BUF_SIZE);
    lprintf("accum = dsl.read_order(%s)\n", buf);
}

void dsl_lua_if(void) {
    // The paramter is ignored, in the original it probably was the address
    // to jump to if the if was not taken.
    dsl_lua_get_parameters(1);
    lprintf("if dsl.is_true(accum) then \n");
    lua_depth++;
}

void dsl_lua_else(void) {
    // The paramter is ignored, in the original it probably was the address
    // to jump to if the else was not taken.
    dsl_lua_get_parameters(1);
    lua_depth--;
    lprintf("else\n");
    lua_depth++;
}

void dsl_lua_setrecord(void) {
    uint16_t depth = 0;
    int16_t header = 0;
    uint16_t element[MAX_SEARCH_STACK];
    uint16_t tmp = peek_half_word();
    int32_t obj_name;
    char buf[BUF_SIZE];

    if (tmp > 0x8000) {
        dsl_lua_access_complex(&header, &depth, element, &obj_name);
        dsl_lua_read_number(buf, BUF_SIZE);
        lprintf("accum = %s\n", buf);
        lprintf("--setrecordneed to do a smart write for setrecord.\n");
        //smart_write_data(header, depth, element, accum);
        return;
    }
    if (tmp == 0) {
        lua_exit("dsl_setrecord: need to implement party...\n");
        return;
    }
    if (tmp < 0x8000) {
        dsl_lua_access_complex(&header, &depth, element, &obj_name);
        dsl_lua_read_number(buf, BUF_SIZE);
        lprintf("accum = %s\n", buf);
        lprintf("--setrecord:I need to write depth/element/accum to list of headers!\n");
        return;
    }
}

void dsl_lua_setother(void) {
    char buf[BUF_SIZE];
    int32_t header = 0;
    dsl_lua_read_number(buf, BUF_SIZE);
    lprintf("if (%s >= 0 and head ~= NULL_OBJECT) or (%s == dsl.id_to_header(%s)) ~= NULL_OBJECT then\n", buf, buf, buf);
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

void dsl_lua_end_control(void) {
    lua_depth--;
    lprintf("end --end_control\n");
}

void dsl_lua_input_string(void) {
    dsl_lua_get_parameters(1);
    lprintf("dsl.ask_for_string()\n");
}

void dsl_lua_input_number(void) {
    dsl_lua_get_parameters(1);
    lprintf("dsl.ask_for_number()\n");
}

void dsl_lua_input_money(void) {
    dsl_lua_get_parameters(1);
    lprintf("dsl.ask_for_money()\n");
}

void dsl_lua_joinparty(void) {
    lua_exit("join party not implemented\n");
}

void dsl_lua_leaveparty(void) {
    lua_exit("leave party not implemented\n");
}

void dsl_lua_lockdoor(void) {
    lprintf("dsl.lockdoor(1)\n");
}

#define MAXMENU   (24)

void dsl_lua_menu(void) {
    int items = 0;
    char buf[BUF_SIZE];
    char mfunction[BUF_SIZE];
    char mbytes[BUF_SIZE];
    dsl_lua_read_number(buf, BUF_SIZE);
    char *menu = (char*) buf;
    size_t menu_len;

    if (needs_quotes(buf)) {
        lprintf("dsl.narrate_open(NAR_ADD_MENU, \"%s\", 0)\n", menu);
    } else {
        lprintf("dsl.narrate_open(NAR_ADD_MENU, %s, 0)\n", menu);
    }
    while ((peek_one_byte() != 0x4A) && (items <= MAXMENU)) {
        menu_len = dsl_lua_read_number(buf, BUF_SIZE);
        //menu = (char*) gBignumptr;
        menu = (char*) buf;
        while (menu[menu_len - 1] == '\n') {
            menu[--menu_len] = '\0';
        }
        dsl_lua_read_number(mfunction, BUF_SIZE);
        dsl_lua_read_number(mbytes, BUF_SIZE);
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
        lprintf("dsl.narrate_open(NAR_ADD_MENU, \"%s\", %s) -- choice param1 goes to addr param2\n", menu, mfunction);
        lua_depth--;
        lprintf("end\n");
    }
    get_byte();  // get rid of the mend...
    //for (int i = 0; i < items; i++) {
        //lprintf("narrate_choice( %d, %d) -- choice param1 goes to addr param2\n", i, menu_functions[i]);
    //}
    //lprintf("accum = dsl.narrate_show() --narrate_wait for input\n");
    lprintf("dsl.narrate_show() --narrate_wait for input\n");
    //printf("Need to implement display_menu()\n");
    //command_implemented = 0;
}

void dsl_lua_setthing(void) {
    dsl_lua_get_parameters(2);
    lprintf("--move character's %s's itme of type %s\n", lparams.params[0], lparams.params[1]);
}

void dsl_lua_print_string(void) {
    dsl_lua_get_parameters(2);
    char nq = needs_quotes(lparams.params[1]) ? '\"' : ' ';
    lprintf("dsl.narrate_open(NAR_SHOW_TEXT, %c%s%c, %s)\n",
        nq, lparams.params[1], nq, lparams.params[0]);
}

void dsl_lua_print_number(void) {
    dsl_lua_get_parameters(2);
    lprintf("dsl.narrate_open(NAR_SHOW_TEXT, \"%s\", %s)\n",
        lparams.params[1], lparams.params[0]);
}

void dsl_lua_printnl(void) {
    lprintf("dsl.narrate_open(NAR_SHOW_TEXT, \"\\n\", 0)\n");
}

void dsl_lua_rand(void) {
    char buf[BUF_SIZE];
    dsl_lua_read_number(buf, BUF_SIZE);
    lprintf("dsl.rand() %% %d\n", atoi(buf) + 1);
    //set_accumulator((int32_t)rand() % (read_number() + 1));
}

void dsl_lua_showpic(void) {
    char buf[BUF_SIZE];
    dsl_lua_read_number(buf, BUF_SIZE);
    lprintf("dsl.narrate_open(NAR_PORTRAIT, \"\", %s)\n", buf);
}

void dsl_lua_skillroll(void) {
    lua_exit("skillroll not implemented\n");
}

void dsl_lua_statroll(void) {
    dsl_lua_get_parameters(3);
    lprintf("accum = 0\n");
    lprintf("if lparams.params[0] == PARTY then\n");
    lua_depth++;
    lprintf("dsl.dsl_stat_roll(%s)\n", lparams.params[0]);
    lua_depth--;
    lprintf("else\n");
    lua_depth++;
    lprintf("dsl.dsl_stat_roll_party()\n");
    lua_depth--;
    lprintf("end\n");
}

void dsl_lua_string_compare(void) {
    lprintf("accum = 0\n");
    dsl_lua_get_parameters(2);
    lprintf("if %s == %s then\n", lparams.params[0], lparams.params[1]);
    lua_depth++;
    lprintf("accum = 1\n");
    lua_depth--;
    lprintf("end\n");
}

void dsl_lua_match_string(void) {
    lua_exit("match_string not implmeneted\n");
}

void dsl_lua_take(void) {
    dsl_lua_get_parameters(4);
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
        lprintf("accum = 0 -- take out after updating dsl_lua_take\n");
        //if (!grafted(param.val[2], param.val[1])) {
            //set_accumulator(take(param.val[0], param.val[1], param.val[2], param.val[3]));
        //}
    }
}

void dsl_lua_sound(void) {
    char buf[BUF_SIZE];
    dsl_lua_read_number(buf, BUF_SIZE);
    lprintf("dsl.play_sound(%s) -- parameter is bvoc index\n", buf);
}

#define CHOSEN (0x7FFD)
#define PARTY  (0x7FFE)
#define ALL    (0x7FFF)
#define IS_POV   (0)
#define IS_PARTY (1)
#define IS_NPC (1)

void dsl_lua_tport(void) {
    dsl_lua_get_parameters(5);
    if ((param.val[0] != PARTY) && (param.val[0] != IS_POV)) {
        //warn("I need to teleport everything to region %d at (%d, %d) priority: %d, onscreen %d\n",
            //param.val[1], param.val[2], param.val[3], param.val[4], param.val[5]);
        lprintf("dsl.tport_everything( %s, %s, %s, %s, %s)\n",
            lparams.params[1], lparams.params[2], lparams.params[3], lparams.params[4], lparams.params[5]);
    } else {
        //warn("I need to teleport party to region %d at (%d, %d) priority: %d, onscreen %d\n",
            //param.val[1], param.val[2], param.val[3], param.val[4], param.val[5]);
        lprintf("dsl.tport_party( %s, %s, %s, %s, %s)\n",
            lparams.params[1], lparams.params[2], lparams.params[3], lparams.params[4], lparams.params[5]);
    }
}

void dsl_lua_bitsnoop(void) {
    dsl_lua_get_parameters(2);
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

void dsl_lua_award(void) {
    dsl_lua_get_parameters(2);
    lprintf("if %s == %d then\n", lparams.params[0], ALL);
    lua_depth++;
    lprintf("accum = accum\n");
    lua_depth--;
    lprintf("elseif %s == %d then\n", lparams.params[0], PARTY);
    lua_depth++;
    lprintf("dsl.award_party(%s)\n", lparams.params[1]);
    lua_depth--;
    lprintf("else\n");
    lua_depth++;
    lprintf("dsl.award_object(%s, %s)\n", lparams.params[0], lparams.params[1]);
    lua_depth--;
    lprintf("end\n");
}

void dsl_lua_request(void) {
    dsl_lua_get_parameters(4);
    lprintf("accum = dsl.request(%s, %s, %s, %s)\n",
        (lparams.params[0]),
        (lparams.params[1]),
        (lparams.params[2]),
        (lparams.params[3]));
}

void dsl_lua_byte_dec(void) {
    dsl_lua_get_parameters(1);
    //( *((uint8_t *) param.ptr[0]) )--;
    lprintf("--((uint8_t)lparams.params[0])--\n");
}

void dsl_lua_word_dec(void) {
    dsl_lua_get_parameters(1);
    lprintf("--((uint16_t)lparams.params[0])--\n");
    //(*((uint16_t *)param.ptr[0]))--;
}

void dsl_lua_long_dec(void) {
    dsl_lua_get_parameters(1);
    lprintf("--((uint32_t)lparams.params[0])--\n");
    //command_implemented = 0;
}

void dsl_lua_byte_inc(void) {
    dsl_lua_get_parameters(1);
    lprintf("--((uint8_t)lparams.params[0])++\n");
    //( *((uint8_t *) param.ptr[0]) )++;
}

static void print_increment(const char *stmt) {
    char buf[128];
    int pos;
    if (!strncmp(stmt, "dsl.get_", 8)) {
        strcpy(buf, stmt); // create something to edit
        for (pos = 0; pos < strlen(stmt) && buf[pos] != 'g'; pos++) { ; }
        buf[pos] = 's';
        buf[strlen(buf) - 1] = '\0'; // chop off the final ')'
        lprintf("%s, %s + 1)\n", buf, stmt);
        return;
    }
    lua_exit("Unable to convert stmt\n");
}

void dsl_lua_word_inc(void) {
    dsl_lua_get_parameters(1);
    lprintf("--WORD INC\n");
    //lprintf("--((uint16_t)lparams.params[0])++\n");
    lprintf("--%s\n", lparams.params[0]);
    print_increment(lparams.params[0]);
    //(*((uint16_t*)param.ptr[0]))++;
}

void dsl_lua_long_inc(void) {
    dsl_lua_get_parameters(1);
    lprintf("--((uint32_t)lparams.params[0])++\n");
}

void dsl_lua_hunt(void) {
    char buf[BUF_SIZE];
    dsl_lua_read_number(buf, BUF_SIZE);
    lprintf("--Need to set %s to HUNT!\n", buf);
    //set_orders(read_number(), HUNT, gPov, 0);
}

void dsl_lua_source_trace(void) {
    lua_exit("lua_source not implemented\n");;
}

void dsl_lua_shop(void) {
    char buf[BUF_SIZE];
    dsl_lua_read_number(buf, BUF_SIZE);
    lprintf("dsl.shop(%s)\n", buf);
}

void dsl_lua_clone(void) {
    dsl_lua_get_parameters(6);
    //int16_t header_num = atoi(lparams.params[0]);
    //int16_t qty = atoi(lparams.params[1]);
    //int16_t x = atoi(lparams.params[2]);
    //int16_t y = atoi(lparams.params[3]);
    //uint8_t priority = atoi(lparams.params[4]);
    //uint8_t placement = atoi(lparams.params[5]);
    lprintf("dsl.clone(%s, %s, %s, %s, %s, %s)\n",
        lparams.params[0],
        lparams.params[1],
        lparams.params[2],
        lparams.params[3],
        lparams.params[4],
        lparams.params[5]);
    //warn("dsl_clone: must create %d of %d objects at (%d, %d) with priority = %d, placement = %d\n",
        //header_num, qty, x, y, priority, placement);
}

void dsl_lua_changemoney(void) {
    char buf[BUF_SIZE];
    dsl_lua_read_number(buf, BUF_SIZE);
    lprintf("dsl.give_money(%s)\n", buf);
}

void dsl_lua_setvar(void) {
    lua_exit("setvar not implemented");
}

void dsl_lua_toggle_accum(void) {
    lprintf("accum = !accum\n");
}

void dsl_lua_getstatus(void) {
    char buf[BUF_SIZE];
    dsl_lua_read_number(buf, BUF_SIZE);
    lprintf("accum = dsl.get_character_status(%s);\n", buf);
}

void dsl_lua_getlos(void) {
    dsl_lua_get_parameters(3);
    lprintf("accum = dsl.los(%s, %s, %s) -- los from %s to %s < %s\n",
        lparams.params[0], lparams.params[1], lparams.params[2],
        lparams.params[0], lparams.params[1], lparams.params[2]);
}

void dsl_lua_long_times_equal(void) {
    dsl_lua_get_parameters(2);
    lprintf("--*lparams.params[0] *= lparams.params[1]\n");
}

void dsl_lua_jump(void) {
    lua_exit("jump not implemented!\n");
}

void dsl_lua_local_sub(void) {
    char buf[BUF_SIZE];
    dsl_lua_read_number(buf, BUF_SIZE);
    lprintf("func%s()\n", buf);
}

void dsl_lua_global_sub(void) {
    dsl_lua_get_parameters(2);
    lprintf("dsl.call_function(%s, %s) -- Jump to addr %s in file %s (GPL.)\n",
        lparams.params[1], lparams.params[0],
        lparams.params[0], lparams.params[1]);
}

void dsl_lua_local_ret(void) {
    in_func = 0;
    lua_depth--;
    lprintf("end --return\n");
}

static void do_damage(int is_percent) {
    dsl_lua_get_parameters(2);
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

void dsl_lua_pdamage(void) {
    do_damage(1);
}

void dsl_lua_word_plus_equal(void) {
    dsl_lua_get_parameters(2);
    lprintf("--*((uint16_t *)lparams.params[0]) += lparam.params[1];\n");
    //*((uint16_t *)param.ptr[0]) += param.val[1];
}

void dsl_lua_word_minus_equal(void) {
    dsl_lua_get_parameters(2);
    lprintf("--*((uint16_t *)lparams.params[0]) -= lparam.params[1];\n");
    //*((uint16_t *)param.ptr[0]) -= param.val[1];
}

void dsl_lua_word_times_equal(void) {
    dsl_lua_get_parameters(2);
    lprintf("--*((uint16_t *)lparams.params[0]) *= lparam.params[1];\n");
    //*((uint16_t *)param.ptr[0]) *= param.val[1];
}

void dsl_lua_word_divide_equal(void) {
    dsl_lua_get_parameters(2);
    lprintf("--*((uint16_t *)lparams.params[0]) /= lparam.params[1];\n");
    //*((uint16_t *)param.ptr[0]) /= param.val[1];
}

void dsl_lua_long_plus_equal(void) {
    dsl_lua_get_parameters(2);
    lprintf("--*((uint32_t *)lparams.params[0]) += lparam.params[1];\n");
    //*((uint32_t *)param.ptr[0]) += param.val[1];
}

void dsl_lua_long_minus_equal(void) {
    dsl_lua_get_parameters(2);
    lprintf("--*((uint32_t *)lparams.params[0]) -= lparam.params[1];\n");
    //*((uint32_t *)param.ptr[0]) -= param.val[1];
}

uint16_t dsl_range(int16_t obj0, int16_t obj1) {
    lprintf("--Must compute range from %d to %d\n", obj0, obj1);
    return 10;//Totally bogus
}

void dsl_lua_get_range(void) {
    dsl_lua_get_parameters(2);
    lprintf("dsl.range(%s, %s)\n",
        lparams.params[0], lparams.params[1]);
}

void dsl_lua_compare(void) {
    dsl_lua_load_accum();
    lprintf("compare = accum\n");
}

static void dsl_lua_load_var() {
    dsl_lua_load_accum();
    dsl_lua_load_variable();
}

/********* EXTRA variable functions ********/
static void dsl_lua_write_complex_var() {
    uint16_t depth = 0;
    int16_t header = 0;
    uint16_t element[MAX_SEARCH_STACK];
    int32_t obj_name;
    memset(element, 0x0, sizeof(uint16_t) * MAX_SEARCH_STACK);
    if (dsl_lua_access_complex(&header, &depth, element, &obj_name) == 1) {
        lprintf("--complex_write: I need to write 'accum' to header (%d) at depth (%d)\n", header, depth);
        //smart_write_data(header, depth, element, data);
    }
}

static void dsl_lua_load_variable() {
    int8_t extended = 0;
    uint8_t datatype;
    uint16_t varnum;
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
        //load_simple_variable(datatype, varnum, 0);
        dsl_lua_load_simple_variable(datatype, varnum);
    } else {
        dsl_lua_write_complex_var();
        //lua_exit("write complex_var not implemented!\n");
    }
}

void dsl_lua_ifcompare(void) {
    dsl_lua_get_parameters(2);
    if (!in_compare) { lprintf("--cmp start\n"); }
    if (in_compare) { lua_depth--;}
    lprintf("%sif compare == %s then\n", 
        in_compare ? "else" : "",
        lparams.params[0]);
    in_compare = 1;
    lua_depth++;
    //lprintf("--go to %s (lparams.params[1])\n", lparams.params[1]);
}

void dsl_lua_clearpic(void) {
    lprintf("dsl.narrate_open(NAR_PORTRAIT, \"\", 0) -- clearpic\n");
}

void dsl_lua_orelse(void) {
    char buf[BUF_SIZE];
    dsl_lua_read_number(buf, BUF_SIZE);
    lprintf("--or else (%s)\n", buf);
    //lua_exit("OR ELSE?");
    //if (compared[comparePtr] == YES) {
        //move_dsl_ptr(address);
    //}
}

void dsl_lua_exit(void) {
    lprintf("dsl.exit()\n");
    if (lua_depth == 1) {
        in_func = 0;
        lua_depth--;
        lprintf("end\n");
    }
}

void dsl_lua_clear_los(void) {
    char buf[BUF_SIZE];
    dsl_lua_read_number(buf, BUF_SIZE);
    lprintf("--clear_los_order: need to clear #%s\n", buf);
}

void dsl_lua_nametonum(void) {
    char buf[BUF_SIZE];
    dsl_lua_read_number(buf, BUF_SIZE);
    if (buf[0] != '-') {
        lprintf("-%s\n", buf);
    } else {
        lprintf("%s\n", buf + 1);
    }
    //set_accumulator(dsl_lua_read_number() * -1);
}

void dsl_lua_numtoname(void) {
    char buf[BUF_SIZE];
    dsl_lua_read_number(buf, BUF_SIZE);
    if (buf[0] != '-') {
        lprintf("-%s\n", buf);
    } else {
        lprintf("%s\n", buf + 1);
    }
    //set_accumulator(dsl_lua_read_number() * -1);
}

void dsl_lua_getxy(void) {
    char buf[BUF_SIZE];
    dsl_lua_read_number(buf, BUF_SIZE);
    lprintf("-- getxyz\n");
    lprintf("dsl.set_gname(1, dsl.getX(%s))\n", buf);
    lprintf("dsl.set_gname(2, dsl.getY(%s))\n", buf);
    lprintf("dsl.set_gname(3, 0)\n");
    //get_xyz(read_number());
}

static void dsl_lua_load_simple_variable(uint16_t type, uint16_t vnum) {
    switch (type) {
        case DSL_GBIGNUM:
            //dsl_global_bnums[vnum] = (int32_t) val;
            lprintf("dsl.set_gbn(%d, accum)\n", vnum);
            break;
        case DSL_LBIGNUM:
            //dsl_local_bnums[vnum] = (int32_t) val;
            lprintf("dsl.set_lbn(%d, accum)\n", vnum);
            break;
        case DSL_GNUM:
            //dsl_global_nums[vnum] = (int16_t) val;
            lprintf("dsl.set_gn(%d,  accum)\n", vnum);
            break;
        case DSL_LNUM:
            //dsl_local_nums[vnum] = (int16_t) val;
            lprintf("dsl.set_ln(%d, accum)\n", vnum);
            break;
        case DSL_GFLAG:
            lprintf("dsl.set_gf(%d, accum)\n", vnum);
            break;
        case DSL_LFLAG:
            lprintf("dsl.set_lf(%d, accum)\n", vnum);
            break;
        default:
            lua_exit("ERROR: Unknown simple variable type! 0x%x!\n");
            break;
    }
}

static size_t dsl_lua_read_number(char *buf, const size_t size) {
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
        cop = get_byte(); // current operation
        //printf("current operation = 0x%x\n", cop);
        if (cop < 0x80) {
            cval = cop * 0x100 + get_byte();
            //debug("immediate = %d\n", cval);
            buf_pos += snprintf(buf + buf_pos, size - buf_pos, "%d", cval);
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
                    //cval = accum;
                    lua_exit("accumulator not implemented.");
                    lprintf("accum");
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
                    //read_simple_num_var();
                    cval = gBignum;
                    buf_pos += dsl_lua_read_simple_num_var(buf + buf_pos, size - buf_pos);
                    //printf("updating buf_pos = %d\n", buf_pos);
                    break;
                }
                case DSL_RETVAL|0x80: {
                    //debug("DSL_RETVAL begin:\n");
                    /*
                    push_params();
                    do_dsl_command(get_byte());
                    pop_params();
                    */
                    int tpos = lua_pos;
                    char *tptr = lua_buf + lua_pos;

                    lprintf("((");
                    int cmd = get_byte();
                    do_lua_dsl_command(cmd);
                    lua_pos--;
                    lprintf("))");
                    //printf("------------------------>'%s'\n", tptr);
                    buf_pos += strlen(strncpy(buf + buf_pos, tptr, size - buf_pos));
                    //strncpy(taccum, BUF_SIZE, tptr);
                    lua_pos = tpos;
                    lua_buf[lua_pos] = '\0';
                    //debug("DSL_RETVAL end, cval = %d\n", cval);
                    //printf("updating buf_pos = %d\n", buf_pos);
                    break;
                }
                case DSL_IMMED_BIGNUM|0x80: {
                    //cval = (int32_t)((int16_t)get_word()) * 655356L 
                         //+ (int32_t)((uint16_t)get_word());
                    cval = (int32_t)dsl_lua_get_word() * (int32_t)655356L;
                    cval += (int32_t)dsl_lua_get_word();
                    buf_pos += snprintf(buf + buf_pos, size - buf_pos, "%d", cval);
                    break;
                }
                case DSL_IMMED_BYTE|0x80: {
                    cval = (int32_t)((int8_t)get_byte());
                    //debug("DSL_IMMED_BYTE, cval = %d\n", cval);
                    buf_pos += snprintf(buf + buf_pos, size - buf_pos, "%d", cval);
                    //printf("updating buf_pos = %d\n", buf_pos);
                    break;
                }
                case DSL_IMMED_WORD|0x80: {
                    lua_exit("DSL_IMMED_WORD not implemented!\n");
                    break;
                }
                case DSL_IMMED_NAME|0x80: {
                    cval = (int32_t)((int16_t)get_half_word() * -1);
                    buf_pos += snprintf(buf + buf_pos, size - buf_pos, "%d", cval);
                    //printf("updating buf_pos = %d\n", buf_pos);
                    //lua_exit("DSL_IMMED_NAME not implemented!\n");
                    break;
                }
                case DSL_COMPLEX_VAL|0x80: {
                    cval = dsl_lua_read_complex(buf, &buf_pos, size);
                    //lua_exit("DSL_COMPLEX_VAL not implemented!\n");
                    break;
                }
                case DSL_IMMED_STRING|0x80: {
                    //cval = 0;
                    //gBignumptr = (int32_t*) read_text();
                    //debug("DSL_IMMED_STRING, cval = %d, '%s'\n", cval, (char*) gBignumptr);
                    //TODO FIXME: This should just copy directly in!
                    char *tmp = read_text();
                    buf_pos += strlen(strncpy(buf + buf_pos, tmp, size - buf_pos));
                    //printf("updating buf_pos = %d\n", buf_pos);
                    break;
                }
                case DSL_OP_ADD:
                    buf_pos += snprintf(buf + buf_pos, size - buf_pos, " + ");
                    do_next = 1;
                    break;
                case DSL_OP_MINUS:
                    buf_pos += snprintf(buf + buf_pos, size - buf_pos, " - ");
                    do_next = 1;
                    break;
                case DSL_OP_TIMES:
                    buf_pos += snprintf(buf + buf_pos, size - buf_pos, " * ");
                    do_next = 1;
                    break;
                case DSL_OP_DIVIDE:
                    buf_pos += snprintf(buf + buf_pos, size - buf_pos, " / ");
                    do_next = 1;
                    break;
                case DSL_OP_AND:
                    buf_pos += snprintf(buf + buf_pos, size - buf_pos, " and ");
                    do_next = 1;
                    break;
                case DSL_OP_OR:
                    buf_pos += snprintf(buf + buf_pos, size - buf_pos, " or ");
                    do_next = 1;
                    break;
                case DSL_OP_EQUAL:
                    buf_pos += snprintf(buf + buf_pos, size - buf_pos, " == ");
                    do_next = 1;
                    break;
                case DSL_OP_NEQUAL:
                    buf_pos += snprintf(buf + buf_pos, size - buf_pos, " ~= ");
                    do_next = 1;
                    break;
                case DSL_OP_GTR:
                    buf_pos += snprintf(buf + buf_pos, size - buf_pos, " > ");
                    do_next = 1;
                    break;
                case DSL_OP_LESS:
                    buf_pos += snprintf(buf + buf_pos, size - buf_pos, " < ");
                    do_next = 1;
                    break;
                case DSL_OP_GTE:
                    buf_pos += snprintf(buf + buf_pos, size - buf_pos, " >= ");
                    do_next = 1;
                    break;
                case DSL_OP_LTE:
                    buf_pos += snprintf(buf + buf_pos, size - buf_pos, " <= ");
                    do_next = 1;
                    break;
                case DSL_OP_BAND:
                    buf_pos += snprintf(buf + buf_pos, size - buf_pos, " & ");
                    do_next = 1;
                    break;
                case DSL_OP_BOR:
                    buf_pos += snprintf(buf + buf_pos, size - buf_pos, " | ");
                    do_next = 1;
                    break;
                case DSL_OP_BCLR: {
                    do_next = 1;
                    buf_pos += snprintf(buf + buf_pos, size - buf_pos, " | ");
                    //lua_exit ("BCLR not implemented\n");
                    //opstack[paren_level] = cop;
                    //found_operator = 1;
                    break;
                }
                case DSL_HI_OPEN_PAREN: {
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
                case DSL_HI_CLOSE_PAREN: {
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
        (((next_op = preview_byte(0)) > OPERATOR_OFFSET && next_op <= OPERATOR_LAST)
            || (paren_level > 0 && next_op == DSL_HI_CLOSE_PAREN)));
    // We need to look if that above else was executed!
    //printf("accums[0] = %d\n", accums[0]);
    //return accums[0];
    //printf("exiting with buf_pos = %d, buf = '%s'\n", buf_pos, buf);
    //lprintf("---- read_number_end, buf  '%s'\n", buf);
    return buf_pos;
}

static uint16_t dsl_lua_get_word() {
    uint16_t ret;
    ret = get_byte() * 0x100;
    ret += get_byte();
    return ret;
}

static uint8_t dsl_lua_access_complex(int16_t *header, uint16_t *depth, uint16_t *element, int32_t *obj_name) {
    uint16_t i;
    
    *obj_name = dsl_lua_get_word();
    //debug("header = %d, depth = %d, element = %d, obj_name = %d\n", *header, *depth, *element, obj_name);
    if (*obj_name < 0x8000) {
        lprintf("--access_complex: I need to convert from ID to header!\n");
    } else {
        lprintf("--access_complex: I need to set the *head to the correct view\n");
        switch (*obj_name & 0x7FFF) {
            case 0x25: // POV
                lprintf("--access_complex: get POV, valid obj_name(%d), need to set header\n", *obj_name & 0x7FFF);
                break;
            case 0x26: // ACTIVE
                lprintf("--access_complex: get ACTIVE, valid obj_name(%d), need to set header\n", *obj_name & 0x7FFF);
                break;
            case 0x27: // PASSIVE
                lprintf("obj = dsl.get_gname(%d) -- passive\n", GNAME_PASSIVE);
                lprintf("--access_complex: get PASSIVE, valid obj_name(%d), need to set header\n", *obj_name & 0x7FFF);
                break;
            case 0x28: // OTHER
                lprintf("--access_complex: get OTHER, valid obj_name(%d), need to set header\n", *obj_name & 0x7FFF);
                break;
            case 0x2C: // OTHER1
                lprintf("--access_complex: get OTHER1, valid obj_name(%d), need to set header\n", *obj_name & 0x7FFF);
                break;
            case 0x2B: // THING
                lprintf("--access_complex: get THING, valid obj_name(%d), need to set header\n", *obj_name & 0x7FFF);
                break;
            default:
                return 0;
        }
    }
    *depth = get_byte();
    //debug("--access_compledx:depth = %d\n", *depth);
    for (i = 1; i <= *depth; i++) {
        element[i-1] = get_byte();
        //debug("--access_complex:element[%d] = %d\n", i-1, element[i-1]);
    }

    return 1;
}

int32_t dsl_lua_get_complex_data(int16_t header, uint16_t depth, uint16_t *element) {
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

static int32_t dsl_lua_read_complex(char *buf, size_t *buf_pos, const size_t size) {
    int32_t ret = 0;
    uint16_t depth = 0;
    int16_t header = 0;
    uint16_t element[MAX_SEARCH_STACK];
    int32_t obj_name;
    memset(element, 0x0, sizeof(uint16_t) * MAX_SEARCH_STACK);

    if (dsl_lua_access_complex(&header, &depth, element, &obj_name) == 1) {
        // So if depth == 0 then get type, is the type, 1 is the id (like -319)
        switch (depth) {
            case 0:
                //lprintf("dsl.get_type(obj)\n"); 
                *buf_pos += snprintf(buf + *buf_pos, size - *buf_pos, "dsl.get_type(obj)");
                break;
            case 1:
                //lprintf("dsl.get_id(obj)\n");
                *buf_pos += snprintf(buf + *buf_pos, size - *buf_pos, "dsl.get_id(obj)");
                break;
            default:
                *buf_pos += snprintf(buf + *buf_pos, size - *buf_pos, "dsl.read_complex(%d, %d, %d)", obj_name, header, depth);
                break;
        }
        lprintf("--read_complex:reading header (%d) at depth (%d)\n", header, depth);
        ret = dsl_lua_get_complex_data(header, depth, element);
        return ret;
    } else {
        lua_exit("--read_complex else not implemented!\n");
    }

    return ret;
}

int dsl_lua_read_simple_num_var(char *buf, const size_t buf_size) {
    int16_t temps16 = get_byte();

    if ((gBignum & EXTENDED_VAR) != 0) {
        temps16 *= 0x100;
        temps16 += get_byte();
        gBignum &= 0x3F;
    }

    switch(gBignum) {
        case DSL_GFLAG: {
            return snprintf(buf, buf_size, "dsl.get_gf(%d)", temps16);
            break;
        }
        case DSL_LFLAG: {
            return snprintf(buf, buf_size, "dsl.get_lf(%d)", temps16);
            break;
        }
        case DSL_GNUM: {
            return snprintf(buf, buf_size, "dsl.get_gn(%d)", temps16);
            break;
        }
        case DSL_LNUM: {
            return snprintf(buf, buf_size, "dsl.get_ln(%d)", temps16);
            break;
        }
        case DSL_GBIGNUM: {
            return snprintf(buf, buf_size, "dsl.get_gbn(%d)", temps16);
            break;
        }
        case DSL_LBIGNUM: {
            return snprintf(buf, buf_size, "dsl.get_lbn(%d)", temps16);
            break;
        }
        case DSL_GNAME: {
            if (temps16 >= 0x20 && temps16 < 0x2F) {
                //return snprintf(buf, buf_size, "global_simple_var[%d]", temps16 - 0x20);
                //return snprintf(buf, buf_size, "\"dsl.get_gname(%d)\"", temps16 - 0x20);
                return snprintf(buf, buf_size, "dsl.get_gname(%d)", temps16 - 0x20);
            } else {
                lua_exit("ERROR: No variable GNAME!!!\n");
            }
            //debug("reading gname @ %d is equal to %d\n", temps16, gBignum);
            //lua_exit ("DSL_GNAME not implemented.\n");
            break;
        }
        case DSL_GSTRING: {
            //gBignumptr = (int32_t*) dsl_global_strings[temps16];
            //debug("reading gstring @ %d is equal to '%s'\n", temps16, (char*)gBignumptr);
            //lua_exit ("DSL_GString not implemented.\n");
            /*
            gBignumptr = (int32_t*) dsl_global_strings[temps16];
            */
            return snprintf(buf, buf_size, "dsl.get_gstr(%d)", temps16);
            break;
        }
        case DSL_LSTRING: {
            return snprintf(buf, buf_size, "dsl.get_lstr(%d)", temps16);
            //lua_exit ("DSL_LString not implemented.\n");
            /*
            gBignumptr = (int32_t*) dsl_local_strings[temps16];
            debug("reading lstring @ %d is equal to '%s'\n", temps16, (char*)gBignumptr);
            */
            break;
        }
        default:
            lua_exit("ERROR: Unknown type in read_simple_num_var.\n");
            break;
    }
    return 0;
}
