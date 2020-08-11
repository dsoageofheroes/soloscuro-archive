#include "dsl-lua.h"
#include "dsl-string.h"
#include "dsl-var.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>

static int print_cmd();
static int varnum = 0;
static int funcnum = 0;
static int in_func = 0;
static void dsl_lua_load_variable();
static void dsl_lua_load_simple_variable(uint16_t type, uint16_t vnum);
static int dsl_lua_read_simple_num_var(char *buf, const size_t buf_size);
static int32_t dsl_lua_read_complex(void);

void dsl_lua_byte_dec(void);
void dsl_lua_word_dec(void);
void dsl_lua_long_dec(void);
void dsl_lua_byte_inc(void);
void dsl_lua_word_inc(void);
void dsl_lua_long_inc(void);
void dsl_lua_local_sub(void);
void dsl_lua_global_sub(void);
void dsl_lua_local_ret(void);
void dsl_lua_compare(void);
void dsl_lua_pdamage(void);
void dsl_lua_cmpend(void);
void dsl_lua_while(void);
void dsl_lua_wend(void);
void dsl_lua_attackcheck(void);
void dsl_lua_lookcheck(void);
void dsl_lua_load_accum(void);
void dsl_lua_global_ret(void);
void dsl_lua_usecheck(void);
void dsl_lua_ifis(void);
void dsl_lua_orelse(void);
void dsl_lua_exit(void);
void dsl_lua_fight(void);
void dsl_lua_flee(void);
void dsl_lua_follow(void);
void dsl_lua_give(void);
void dsl_lua_go(void);
void dsl_lua_input_bignum(void);
void dsl_lua_goxy(void);
void dsl_lua_if(void);
void dsl_lua_else(void);
void dsl_lua_setother(void);
void dsl_lua_menu(void);
void dsl_lua_print_string(void);
void dsl_lua_print_number(void);
void dsl_lua_printnl(void);
void dsl_lua_rand(void);
void dsl_lua_showpic(void);
void dsl_lua_take(void);
void dsl_lua_sound(void);
void dsl_lua_tport(void);
void dsl_lua_request(void);
void dsl_lua_hunt(void);
void dsl_lua_end_control(void);
void dsl_lua_inloscheck(void);
void dsl_lua_notinloscheck(void);
void dsl_lua_move_tilecheck(void);
void dsl_lua_door_tilecheck(void);
void dsl_lua_move_boxcheck(void);
void dsl_lua_door_boxcheck(void);
void dsl_lua_pickup_itemcheck(void);
void dsl_lua_talktocheck(void);
void dsl_lua_noorderscheck(void);
void dsl_lua_usewithcheck(void);
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

void dsl_lua_read_number(char *buf, const size_t size);
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
#define BUF_SIZE (1<<12)
#define NUM_PARAMS (8)
typedef struct params_s {
    char params[NUM_PARAMS][BUF_SIZE];
} params_t;
static params_t lparams;
#define LUA_MAX_SIZE (1<<16)
static char master_mas[LUA_MAX_SIZE];
static char master_gpl[LUA_MAX_SIZE];
static char lua_buf[LUA_MAX_SIZE];
static size_t lua_pos = 0;
static int32_t lua_depth = 0; 
#define lprintf(...) lua_tab(lua_depth); lua_pos += snprintf(lua_buf + lua_pos, LUA_MAX_SIZE - lua_pos, __VA_ARGS__)
#define LUA_TAB_AMT (4)
void lua_tab(const int amt) {
    for (int i = 0; i < amt && lua_pos < LUA_MAX_SIZE; i++) {
        for (int j = 0; j < LUA_TAB_AMT && lua_pos < LUA_MAX_SIZE; j++) {
            lua_buf[lua_pos++] = ' ';
        }
    }
    lua_buf[lua_pos >= LUA_MAX_SIZE ? LUA_MAX_SIZE - 1 : lua_pos] = '\0';
}
void lua_exit(const char *msg) {
    printf("lua code at error:\n%s\n", lua_buf);
    printf(msg);
    exit(1);
}
void validate_number(const char *num, const char *message) {
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
    { NULL, "dsl changemoney" }, // 0xC
    { NULL, "dsl setvar" }, // 0xD
    { NULL, "dsl toggle accum" }, // 0xE
    { NULL, "dsl getstatus" }, // 0xF
    { NULL, "dsl getlos" }, // 0x10
    { NULL, "dsl long times equal" }, // 0x11
    { NULL, "dsl jump" }, // 0x12
    { dsl_lua_local_sub, "dsl local sub" }, // 0x13
    { dsl_lua_global_sub, "dsl global sub" }, // 0x14
    { dsl_lua_local_ret, "dsl local ret" }, // 0x15
    { dsl_lua_load_var, "dsl load variable" }, // 0x16
    { dsl_lua_compare, "dsl compare" }, // 0x17
    { dsl_lua_load_accum, "dsl load accum" }, // 0x18
    { dsl_lua_global_ret, "dsl global ret" }, // 0x19
    { NULL, "dsl nextto" }, // 0x1A
    { dsl_lua_inloscheck, "dsl inloscheck" }, // 0x1B
    { dsl_lua_notinloscheck, "dsl notinloscheck" }, // 0x1C
    { dsl_lua_clear_los, "dsl clear los" }, // 0x1D
    { dsl_lua_nametonum, "dsl nametonum" }, // 0x1E
    { dsl_lua_numtoname, "dsl numtoname" }, // 0x1F
    { NULL, "dsl bitsnoop" }, // 0x20
    { NULL, "dsl award" }, // 0x21
    { dsl_lua_request, "dsl request" }, // 0x22
    { NULL, "dsl source trace" }, // 0x23
    { NULL, "dsl shop" }, // 0x24
    { dsl_lua_clone, "dsl clone" }, // 0x25
    { NULL, "dsl default" }, // 0x26
    { dsl_lua_ifis, "dsl ifis" }, // 0x27
    { NULL, "dsl trace var" }, // 0x28
    { dsl_lua_orelse, "dsl orelse" }, // 0x29
    { NULL, "dsl clearpic" }, // 0x2A
    { NULL, "dsl continue" }, // 0x2B
    { NULL, "dsl log" }, // 0x2C
    { NULL, "dsl damage" }, // 0x2D
    { NULL, "dsl source line num" }, // 0x2E
    { NULL, "dsl drop" }, // 0x2F
    { NULL, "dsl passtime" }, // 0x30
    { dsl_lua_exit, "dsl exit dsl" }, // 0x31
    { NULL, "dsl fetch" }, // 0x32
    { NULL, "dsl search" }, // 0x33
    { NULL, "dsl getparty" }, // 0x34
    { dsl_lua_fight, "dsl fight" }, // 0x35
    { dsl_lua_flee, "dsl flee" }, // 0x36
    { dsl_lua_follow, "dsl follow" }, // 0x37
    { NULL, "dsl getyn" }, // 0x38
    { dsl_lua_give, "dsl give" }, // 0x39
    { dsl_lua_go, "dsl go" }, // 0x3A
    { dsl_lua_input_bignum, "dsl input bignum" }, // 0x3B
    { dsl_lua_goxy, "dsl goxy" }, // 0x3C
    { NULL, "dsl readorders" }, // 0x3D
    { dsl_lua_if, "dsl if" }, // 0x3E
    { dsl_lua_else, "dsl else" }, // 0x3F
    { NULL, "dsl setrecord" }, // 0x40
    { dsl_lua_setother, "dsl setother" }, // 0x41
    { NULL, "dsl input string" }, // 0x42
    { NULL, "dsl input number" }, // 0x43
    { NULL, "dsl input money" }, // 0x44
    { NULL, "dsl joinparty" }, // 0x45
    { NULL, "dsl leaveparty" }, // 0x46
    { NULL, "dsl lockdoor" }, // 0x47
    { dsl_lua_menu, "dsl menu" }, // 0x48
    { NULL, "dsl setthing" }, // 0x49
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
    { NULL, "dsl skillroll" }, // 0x58
    { NULL, "dsl statroll" }, // 0x59
    { NULL, "dsl string compare" }, // 0x5A
    { NULL, "dsl match string" }, // 0x5B
    { dsl_lua_take, "dsl take" }, // 0x5C
    { dsl_lua_sound, "dsl sound" }, // 0x5D
    { dsl_lua_tport, "dsl tport" }, // 0x5E
    { NULL, "dsl music" }, // 0x5F
    { NULL, "dsl default" }, // 0x60
    { dsl_lua_cmpend, "dsl cmpend" }, // 0x61
    { NULL, "dsl wait" }, // 0x62
    { dsl_lua_while, "dsl while" }, // 0x63
    { dsl_lua_wend, "dsl wend" }, // 0x64
    { dsl_lua_attackcheck, "dsl attackcheck" }, // 0x65
    { dsl_lua_attackcheck, "dsl lookcheck" }, // 0x66
    { dsl_lua_end_control, "dsl endif" }, // 0x67
    { dsl_lua_move_tilecheck, "dsl move tilecheck" }, // 0x68
    { dsl_lua_door_tilecheck, "dsl door tilecheck" }, // 0x69
    { dsl_lua_move_boxcheck, "dsl move boxcheck" }, // 0x6A
    { dsl_lua_door_boxcheck, "dsl door boxcheck" }, // 0x6B
    { dsl_lua_pickup_itemcheck, "dsl pickup itemcheck" }, // 0x6C
    { dsl_lua_usecheck, "dsl usecheck" }, // 0x6D
    { dsl_lua_talktocheck, "dsl talktocheck" }, // 0x6E
    { dsl_lua_noorderscheck, "dsl noorderscheck" }, // 0x6F
    { dsl_lua_usewithcheck, "dsl usewithcheck" }, // 0x70
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
    printf("cmd = 0x%x (do_lua_dsl_command.)\n", cmd);
    (*dsl_lua_operations[cmd].func)();
    //print_vars(0);
}

static size_t dsl_lua_start_ptr = 0;

char* dsl_lua_print(unsigned char *dsl, const size_t len) {
    lua_pos = 0;
    lua_depth = 0; 
    lua_buf[0] = '\0';
    push_data_ptr(dsl);
    set_data_ptr(dsl, dsl);
    dsl_lua_start_ptr = (size_t)dsl;
    const size_t start = (size_t)get_data_ptr();
    size_t diff = (size_t)get_data_ptr() - start;
    //printf("dsl = %p, len = %ld\n", dsl, len);
    while (diff < len && print_cmd()) {
        ;
        diff = (size_t)get_data_ptr() - start;
    }
    printf("tranversed = %ld, len = %ld\n", (size_t)get_data_ptr() - (size_t)start, len);
    pop_data_ptr();
    return lua_buf;
}

static int print_cmd() {
    uint8_t command = get_byte();
    if (!dsl_lua_operations[command].func) {
        printf("Lua code so far:\n%s\n", lua_buf);
        printf("Unimplemented command = 0x%2x\n", command);
        exit(1);
        return 0;
    }
    if (!in_func) {
        lprintf("function func%d (accum) -- address %ld\n", funcnum++, ((size_t)get_data_ptr()) - dsl_lua_start_ptr);
        lua_depth++;
        in_func = 1;
    }
    //printf("print_cmd: command = 0x%x\n", command);
    (*dsl_lua_operations[command].func)();
    return 1;
}

static void write_lua(const char *path, const char *lua) {
    FILE *file = fopen(path, "w+");
    if (file) {
        fwrite(lua, 1, strlen(lua), file);
        fclose(file);
    } else {
        fprintf(stderr, "Warning, unable to write to %s\n", path);
    }
}

void dsl_lua_load_master_gff() {
    size_t len;
    unsigned char *dsl;
    
    dsl = (unsigned char*)gff_get_raw_bytes(DSLDATA_GFF_INDEX, GT_MAS, 99, &len);
    strcpy(master_mas, dsl_lua_print(dsl, len));
    dsl = (unsigned char*)gff_get_raw_bytes(DSLDATA_GFF_INDEX, GT_GPL, 99, &len);
    strcpy(master_gpl, dsl_lua_print(dsl, len));
    write_lua("master-mas.lua", master_mas);
    write_lua("master-gpl.lua", master_gpl);
}

static void dsl_lua_string_copy() {
    dsl_lua_get_parameters(2);
    lprintf("var%d = \"%s\"\n", varnum++, (char*)lparams.params[1]);
}

// Does this variable come from parameter passing?
void dsl_lua_load_accum(void) {
    char buf[BUF_SIZE];
    dsl_lua_read_number(buf, BUF_SIZE);
    lprintf("accum = %s\n", buf);
}

void dsl_lua_global_ret(void) {
    lprintf("--return\n");
    in_func = 0;
    lua_depth--;
}

void dsl_lua_inloscheck(void) {
    dsl_lua_get_parameters(4);
    //global_addr_name(&param);
    //set_los_order(DSL_IN_LOS, param.val[3]);
    lprintf("-- need to set line of sight order on %d\n", param.val[3]);
}

void dsl_lua_notinloscheck(void) {
    dsl_lua_get_parameters(4);
    //global_addr_name(&param);
    //set_los_order(DSL_NOT_IN_LOS, param.val[3]);
    lprintf("-- need to set not line of sight order on %d\n", param.val[3]);
}

void dsl_lua_move_tilecheck(void) {
    dsl_lua_get_parameters(5);
    lprintf("--unimplmented move tile check.\n");
}

void dsl_lua_door_tilecheck(void) {
    lua_exit("Can't parse door_tilecheck yet.\n");
    lprintf("--unimplmented move box check.\n");
}

void dsl_lua_move_boxcheck(void) {
    dsl_lua_get_parameters(7);
    //generic_box_check(MOVE_BOX_CHECK_INDEX, get_box(&param));
    lprintf("--unimplmented move box check.\n");
}

void dsl_lua_door_boxcheck(void) {
    lua_exit("Can't parse lua_door_boxcheck yet.\n");
    lprintf("--unimplmented door box check.\n");
}

void dsl_lua_pickup_itemcheck(void) {
    dsl_lua_get_parameters(3);
    //global_addr_name(&param);
    lprintf("--unimplmented pickup item check.\n");
}

void dsl_lua_talktocheck(void) {
    dsl_lua_get_parameters(3);
    //global_addr_name(&param);
    lprintf("--unimplmented talk to check.\n");
}

void dsl_lua_noorderscheck(void) {
    dsl_lua_get_parameters(3);
    //global_addr_name(&param);
    lprintf("--unimplmented no orders check.\n");
}

void dsl_lua_usewithcheck(void) {
    dsl_lua_get_parameters(4);
    //name_name_global_addr(&param);
    lprintf("--unimplmented use with check.\n");
}

void dsl_lua_cmpend(void) {
    lprintf("--compare end\n");
    lprintf("--compare = NO\n");
}

void dsl_lua_while(void) {
    dsl_lua_get_parameters(1);
    lprintf("while accum == YES do\n");
    lua_depth++;
}

void dsl_lua_wend(void) {
    char buf[BUF_SIZE];
    dsl_lua_read_number(buf, BUF_SIZE);
    lua_depth--;
    lprintf("end -- need to move to %s\n", buf);
}

void dsl_lua_attackcheck(void) {
    dsl_lua_get_parameters(3);
    //global_addr_name(&param);
    //generic_name_check(ATTACK_CHECK_INDEX);
    lprintf("--*******************attack check not finished*************\n");
}

void dsl_lua_lookcheck(void) {
    dsl_lua_get_parameters(3);
    //global_addr_name(&param);
    //generic_name_check(LOOK_CHECK_INDEX);
    lprintf("--*******************loock check not implemented*************\n");
}

void dsl_lua_usecheck(void) {
    dsl_lua_get_parameters(3);
    //global_addr_name(&param);
    //generic_name_check(USE_CHECK_INDEX);
    lprintf("--*******************use check not finished*************\n");
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

void dsl_lua_give(void) {
    dsl_lua_get_parameters(4);
    lprintf("--need to give and set accumulator!");
    //set_accumulator(give(param.val[0], param.val[1], param.val[2], param.val[3], DSL_NEW_SLOT));
}

void dsl_lua_go(void) {
    dsl_lua_get_parameters(2);
    lprintf("--need to give GO order!");
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

void dsl_lua_if(void) {
    // The paramter is ignored, in the original it probably was the address
    // to jump to if the if was not taken.
    dsl_lua_get_parameters(1);
    lprintf("if accum then \n");
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

static int32_t dsl_lua_id_to_header(int32_t header) {
    // guess: is this to find the actual object header?  I'm just return 1234 for now...
    warn("id_to_header: guessing 1234\n");
    return 1234;
}

void dsl_lua_setother(void) {
    char buf[BUF_SIZE];
    int32_t header = 0;
    debug ("set other\n");
    dsl_lua_read_number(buf, BUF_SIZE);
    validate_number(buf, "dsl_lua_setother: ");
    if ((( header = atoi(buf) >= 0 && header != NULL_OBJECT)
        || (header = dsl_lua_id_to_header(header)) != NULL_OBJECT)) {
        lprintf("other1 = %d\n", header);
        lprintf("accum = 1\n");
    } else {
        lprintf("accum = 0\n");
    }
}

void dsl_lua_end_control(void) {
    lua_depth--;
    lprintf("end\n");
}

static uint16_t menu_functions[12];
static uint8_t menu_bytes[12];
#define MAXMENU   (24)
static uint8_t menu_count;

void dsl_lua_menu(void) {
    int items = 0;
    char buf[BUF_SIZE];
    dsl_lua_read_number(buf, BUF_SIZE);
    //char *menu = (char*) gBignumptr;
    char *menu = (char*) buf;
    lprintf("narrate_open(NAR_ADD_MENU, \"%s\", 0)\n", menu);
    menu_count = 0;
    while ((peek_one_byte() != 0x4A) && (items <= MAXMENU)) {
        dsl_lua_read_number(buf, BUF_SIZE);
        //menu = (char*) gBignumptr;
        menu = (char*) buf;
        menu_functions[items] = read_number();
        menu_bytes[items] = (uint8_t) read_number();
        menu_count++;
        if (menu_bytes[items] == 1) {
            lprintf("narrate_open(NAR_ADD_MENU, \"%s\", %d)\n", menu, items - 1);
            items++;
        } else {
            warn("Not available at this time: '%s'\n", menu);
        }
    }
    get_byte();  // get rid of the mend...
    for (int i = 0; i < items; i++) {
        lprintf("narrate_choice( %d, %d) -- choice param1 goes to addr param2\n", i, menu_functions[i]);
    }
    lprintf("--narrate_wait for input\n");
    //printf("Need to implement display_menu()\n");
    //command_implemented = 0;
}

void dsl_lua_print_string(void) {
    dsl_lua_get_parameters(2);
    lprintf("narrate_open(NAR_SHOW_TEXT, \"%s\", %s);\n",
        lparams.params[1], lparams.params[0]);
}

void dsl_lua_print_number(void) {
    dsl_lua_get_parameters(2);
    lprintf("narrate_open(NAR_SHOW_TEXT, \"%s\", %s);\n",
        lparams.params[1], lparams.params[0]);
}

void dsl_lua_printnl(void) {
    lprintf("narrate_open(NAR_SHOW_TEXT, \"\\n\", 0);\n");
}

void dsl_lua_rand(void) {
    char buf[BUF_SIZE];
    dsl_lua_read_number(buf, BUF_SIZE);
    lprintf("accum = rand() MOD %s\n", buf);
    //set_accumulator((int32_t)rand() % (read_number() + 1));
}

void dsl_lua_showpic(void) {
    char buf[BUF_SIZE];
    dsl_lua_read_number(buf, BUF_SIZE);
    lprintf("narrate_open(NAR_PORTRAIT, NULL, %s)\n", buf);
}

void dsl_lua_take(void) {
    dsl_lua_get_parameters(4);
    printf("HERE\n");
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

void dsl_lua_request(void) {
    dsl_lua_get_parameters(4);
    dsl_request_impl(
        atoi(lparams.params[0]),
        atoi(lparams.params[1]),
        atoi(lparams.params[2]),
        atoi(lparams.params[3])
    );
    //set_accumulator(dsl_request_impl(param.val[0], param.val[1], param.val[2], param.val[3]));
    lprintf("--dsl_lua_request not implemented....\n");
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

void dsl_lua_word_inc(void) {
    dsl_lua_get_parameters(1);
    lprintf("--((uint16_t)lparams.params[0])++\n");
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

void dsl_lua_clone(void) {
    dsl_lua_get_parameters(6);
    //for (int i = 0; i < 6; i++) {
        //warn("param.val[%d] = %d\n", i, param.val[i]);
    //}
    int16_t header_num = atoi(lparams.params[0]);
    int16_t qty = atoi(lparams.params[1]);
    int16_t x = atoi(lparams.params[2]);
    int16_t y = atoi(lparams.params[3]);
    uint8_t priority = atoi(lparams.params[4]);
    uint8_t placement = atoi(lparams.params[5]);
    lprintf("-- clone: must create %d of %d objects at (%d, %d) with priority = %d, placement = %d\n",
        header_num, qty, x, y, priority, placement);
    //warn("dsl_clone: must create %d of %d objects at (%d, %d) with priority = %d, placement = %d\n",
        //header_num, qty, x, y, priority, placement);
}

void dsl_lua_local_sub(void) {
    char buf[BUF_SIZE];
    dsl_lua_read_number(buf, BUF_SIZE);
    lprintf("-- I need to call local function at address %s\n", buf);
}

void dsl_lua_global_sub(void) {
    dsl_lua_get_parameters(2);
    int32_t addr = atoi(lparams.params[0]);
    int32_t file = atoi(lparams.params[1]);
    lprintf("--In file %d, I need to execute routine at address %d (NO MAS.)\n", file, addr);
    //dsl_execute_subroutine(param.val[1], param.val[0], 0);
}

void dsl_lua_local_ret(void) {
    lprintf("--return\n");
    in_func = 0;
    lua_depth--;
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
    lua_exit("word -= not implemented\n");
    //*((uint16_t *)param.ptr[0]) -= param.val[1];
}

void dsl_lua_word_times_equal(void) {
    dsl_lua_get_parameters(2);
    lua_exit("word *= not implemented\n");
    //*((uint16_t *)param.ptr[0]) *= param.val[1];
}

void dsl_lua_word_divide_equal(void) {
    dsl_lua_get_parameters(2);
    lua_exit("word /= not implemented\n");
    //*((uint16_t *)param.ptr[0]) /= param.val[1];
}

void dsl_lua_long_plus_equal(void) {
    dsl_lua_get_parameters(2);
    lua_exit("long += not implemented\n");
    //*((uint32_t *)param.ptr[0]) += param.val[1];
}

void dsl_lua_long_minus_equal(void) {
    dsl_lua_get_parameters(2);
    lua_exit("long -= not implemented\n");
    //*((uint32_t *)param.ptr[0]) -= param.val[1];
}

uint16_t dsl_range(int16_t obj0, int16_t obj1) {
    lprintf("--Must compute range from %d to %d\n", obj0, obj1);
    return 10;//Totally bogus
}

void dsl_lua_get_range(void) {
    dsl_lua_get_parameters(2);
    lprintf("--accum = range from %s to %s\n", lparams.params[0], lparams.params[1]);
    //dsl_range(lparams.params[0], lparams.params[1]);
}

void dsl_lua_compare(void) {
    dsl_lua_load_accum();
    lprintf("--compare = accum\n");
}

static void dsl_lua_load_var() {
    dsl_lua_load_accum();
    dsl_lua_load_variable();
}

/********* EXTRA variable functions ********/
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
        warn("------------NEED TO DEBUG WRITE COMPLEX VAR------------\n");
        warn("write complex_var not implemented!\n");
        exit(1);
        //write_complex_var(accum);
    }
}

void dsl_lua_ifis(void) {
    dsl_lua_get_parameters(2);
    lprintf("if accum != %s then\n", lparams.params[0]);
    lua_depth++;
    lprintf("--go to %s (lparams.params[1])\n", lparams.params[1]);
    lua_depth--;
    lprintf("end\n");
    lprintf("--compare = YES\n");
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
    lua_depth--;
    lprintf("end\n");
    in_func = 0;
}

void dsl_lua_clear_los(void) {
    char buf[BUF_SIZE];
    dsl_lua_read_number(buf, BUF_SIZE);
    lprintf("--clear_los_order: need to clear #%s\n", buf);
}

void dsl_lua_nametonum(void) {
    char buf[BUF_SIZE];
    dsl_lua_read_number(buf, BUF_SIZE);
    lprintf("accum = -%s\n", buf);
    //set_accumulator(dsl_lua_read_number() * -1);
}

void dsl_lua_numtoname(void) {
    char buf[BUF_SIZE];
    dsl_lua_read_number(buf, BUF_SIZE);
    lprintf("accum = -%s\n", buf);
    //set_accumulator(dsl_lua_read_number() * -1);
}

void dsl_lua_getxy(void) {
    char buf[BUF_SIZE];
    dsl_lua_read_number(buf, BUF_SIZE);
    lprintf("-- I need to get the xyz of item %s\n", buf);
    //get_xyz(read_number());
}

static void dsl_lua_load_simple_variable(uint16_t type, uint16_t vnum) {
    switch (type) {
        case DSL_GBIGNUM:
            //dsl_global_bnums[vnum] = (int32_t) val;
            lprintf("dsl_global_bignums[%d] = accum\n", vnum);
            break;
        case DSL_LBIGNUM:
            //dsl_local_bnums[vnum] = (int32_t) val;
            lprintf("dsl_local_bignums[%d] = accum\n", vnum);
            break;
        case DSL_GNUM:
            //dsl_global_nums[vnum] = (int16_t) val;
            lprintf("dsl_global_nums[%d] = accum\n", vnum);
            break;
        case DSL_LNUM:
            //dsl_local_nums[vnum] = (int16_t) val;
            lprintf("dsl_local_nums[%d] = accum\n", vnum);
            break;
        case DSL_GFLAG:
            lprintf("dsl_global_flags[%d] = accum\n", vnum);
            break;
        case DSL_LFLAG:
            lprintf("dsl_local_flags[%d] = accum\n", vnum);
            break;
        default:
            lua_exit("ERROR: Unknown simple variable type! 0x%x!\n");
            break;
    }
}

void dsl_lua_read_number(char *buf, const size_t size) {
    int32_t paren_level = 0;
    int8_t found_operator = 0; // did we find an operation?
    int8_t do_next;
    int16_t opstack[MAX_PARENS];
    int32_t accums[MAX_PARENS];
    //char taccum[BUF_SIZE];
    int16_t cop, next_op; // current operation
    int32_t cval = 0, tval = 0; // current value, temporary value
    int buf_pos = 0;
    memset(opstack, 0, sizeof(opstack));
    memset(accums, 0, sizeof(accums));
    do {
     //   taccum[0] = '\0';
        found_operator = 0;
        do_next = 0;
        //printf("buf_pos = %d\n", buf_pos);
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
                    lprintf("(");
                    do_lua_dsl_command(get_byte());
                    lprintf(")");
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
                    lua_exit("DSL_IMMED_BIGNUM not implemented!\n");
                    //debug("DSL_IMMED_BIGNUM, cval = %d\n", cval);
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
                    cval = dsl_lua_read_complex();
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
                    buf_pos += snprintf(buf + buf_pos, size - buf_pos, " && ");
                    do_next = 1;
                    break;
                case DSL_OP_OR:
                    buf_pos += snprintf(buf + buf_pos, size - buf_pos, " || ");
                    do_next = 1;
                    break;
                case DSL_OP_EQUAL:
                    buf_pos += snprintf(buf + buf_pos, size - buf_pos, " == ");
                    do_next = 1;
                    break;
                case DSL_OP_NEQUAL:
                    buf_pos += snprintf(buf + buf_pos, size - buf_pos, " != ");
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
                    printf("CLOSEbuf_pos = %d\n", buf_pos);
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
        
        if (!found_operator) {
            if (opstack[paren_level]) {
                lua_exit("Operators not implemented!\n");
            }
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
    //return accums[0];
    printf("exiting with buf_pos = %d, buf = '%s'\n", buf_pos, buf);
    return;
}

static uint16_t dsl_lua_get_word() {
    uint16_t ret;
    ret = get_byte() * 0x100;
    ret += get_byte();
    return ret;
}

static uint8_t dsl_lua_access_complex(int16_t *header, uint16_t *depth, uint16_t *element) {
    uint16_t i;
    int32_t obj_name;
    
    obj_name = dsl_lua_get_word();
    //debug("header = %d, depth = %d, element = %d, obj_name = %d\n", *header, *depth, *element, obj_name);
    if (obj_name < 0x8000) {
        lprintf("--access_complex: I need to convert from ID to header!\n");
    } else {
        lprintf("--access_complex: I need to set the *head to the correct view\n");
        switch (obj_name & 0x7FFF) {
            case 0x25: // POV
            case 0x26: // ACTIVE
            case 0x27: // PASSIVE
            case 0x28: // OTHER
            case 0x2C: // OTHER1
            case 0x2B: // THING
                lprintf("--access_complex:valid obj_name(%d), need to set header (but can't yet...)\n", obj_name & 0x7FFF);
                break;
            default:
                return 0;
        }
    }
    *depth = get_byte();
    debug("--access_compledx:depth = %d\n", *depth);
    for (i = 1; i <= *depth; i++) {
        element[i-1] = get_byte();
        debug("--access_complex:element[%d] = %d\n", i-1, element[i-1]);
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

static int32_t dsl_lua_read_complex(void) {
    int32_t ret = 0;
    uint16_t depth = 0;
    int16_t header = 0;
    uint16_t element[MAX_SEARCH_STACK];
    memset(element, 0x0, sizeof(uint16_t) * MAX_SEARCH_STACK);

    if (dsl_lua_access_complex(&header, &depth, element) == 1) {
        printf("--read_complex:reading header (%d) at depth (%d)\n", header, depth);
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
            return snprintf(buf, buf_size, "dsl_global_flags[%d]", temps16);
            break;
        }
        case DSL_LFLAG: {
            return snprintf(buf, buf_size, "dsl_local_flags[%d]", temps16);
            break;
        }
        case DSL_GNUM: {
            return snprintf(buf, buf_size, "dsl_global_num_flags[%d]", temps16);
            break;
        }
        case DSL_LNUM: {
            return snprintf(buf, buf_size, "dsl_local_num_flags[%d]", temps16);
            break;
        }
        case DSL_GBIGNUM: {
            return snprintf(buf, buf_size, "dsl_global_bnum_flags[%d]", temps16);
            break;
        }
        case DSL_LBIGNUM: {
            return snprintf(buf, buf_size, "dsl_local_bnum_flags[%d]", temps16);
            break;
        }
        case DSL_GNAME: {
            if (temps16 >= 0x20 && temps16 < 0x2F) {
                //lprintf("gSimpleVar[%d]", temps16 - 0x20);
                return snprintf(buf, buf_size, "global_simple_var[%d]", temps16 - 0x20);
                //gBignumptr = (int32_t*)gSimpleVar[temps16 - 0x20];
                //if (temps16 >= 0x29 && temps16 <= 0x2A) {
                    //gBignum = *((int32_t*)gBignumptr);
                //} else {
                    //gBignum = *((int16_t*)gBignumptr);
                //}
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
            return snprintf(buf, buf_size, "dsl_global_strings[%d]", temps16);
            break;
        }
        case DSL_LSTRING: {
            lua_exit ("DSL_LString not implemented.\n");
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
