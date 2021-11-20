#include "gpl-string.h"
#include "dsl-var.h"
#include <string.h>

/* externals first */
uint16_t this_gpl_file = 0;
uint16_t this_gpl_type = 0;

/* Now static to file... */
static uint8_t dsl_global_flags[DSL_GFLAGVAR_SIZE];
static uint8_t dsl_local_flags[DSL_LFLAGVAR_SIZE];
static int32_t accum; //, number;
static unsigned char* dsl_data;
static unsigned char* dsl_data_start;
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
    disk_object_t dobj;
    switch(so->type) {
        case SO_DS1_COMBAT:
            return so->data.ds1_combat.name;
            break;
        case SO_DS1_ITEM:
            gff_read_object(so->data.ds1_item.id, &dobj);
            //dobj = gff_get_object(so->data.ds1_item.name_index);
            //sprintf(buf, "item (index = %d, bmp_id = %d)", so->data.ds1_item.name_index, dobj->bmp_id);
            sprintf(buf, "item (id = %d, bmp_id = %d, script_id = %d)", so->data.ds1_item.id, dobj.bmp_id,
            dobj.script_id);
            //return "<ITEM-need to implement>";
            return buf;
            break;
    }
    return "UNKNOWN";
}

static void print_name_check(int check_index) {
    debug("new_name.name = %d\n", new_name.name);
    so_object_t *so = gff_object_inspect(OBJEX_GFF_INDEX, abs(new_name.name));
    debug("When I %s to '%s' (%d) goto file: %d, addr: %d, global = %d\n",
        debug_index_names[check_index], get_so_name(so), new_name.name,
        new_name.file, new_name.addr, new_name.global);
    free(so);
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

void print_all_checks() {
    box_t box;
    tile_t tile;
    name_t name;
    name2_t name2;
    for (int i = 0; i < MAX_CHECK_TYPES; i++) {
        for (int j = 0; j < checks_pos[i]; j++) {
            box   = checks[i][j].data.box_check;
            tile  = checks[i][j].data.tile_check;
            name  = checks[i][j].data.name_check;
            name2 = checks[i][j].data.name2_check;
            switch(i) {
                case UNUSED_CHECK_INDEX:
                    debug("check['%s'] =\n", debug_index_names[i]);
                    break;
                case TALK_TO_CHECK_INDEX:
                    debug("check['%s']: talk to %d call %d:%d (global = %d)\n",
                        debug_index_names[i], name.name, name.file, name.addr, name.global);
                    break;
                case ATTACK_CHECK_INDEX:
                    debug("check['%s']: attacks %d call %d:%d (global = %d)\n",
                        debug_index_names[i], name.name, name.file, name.addr, name.global);
                    break;
                case POV_CHECK_INDEX:
                    debug("check['%s'] =\n", debug_index_names[i]);
                    break;
                case PICKUP_CHECK_INDEX:
                    debug("check['%s'] =\n", debug_index_names[i]);
                    break;
                case OTHER_CHECK_INDEX:
                    debug("check['%s'] =\n", debug_index_names[i]);
                    break;
                case OTHER1_CHECK_INDEX:
                    debug("check['%s'] =\n", debug_index_names[i]);
                    break;
                case MOVE_TILE_CHECK_INDEX:
                    debug("check['%s']: when I move to (%d, %d) call %d:%d (pc-only = %d) \n",
                        debug_index_names[i], tile.x, tile.y, tile.file, tile.addr, tile.trip);
                    break;
                case LOOK_CHECK_INDEX:
                    debug("check['%s']: look at %d call %d:%d (global = %d)\n",
                        debug_index_names[i], name.name, name.file, name.addr, name.global);
                    break;
                case USE_CHECK_INDEX:
                    debug("check['%s']: use %d call %d:%d (global = %d)\n",
                        debug_index_names[i], name.name, name.file, name.addr, name.global);
                    break;
                case USE_WITH_CHECK_INDEX:
                    debug("check['%s']: use %d on %d call %d:%d (global = %d)\n",
                        debug_index_names[i], name2.name2, name2.name1, name2.file, name2.addr, name2.global);
                    break;
                case MOVE_BOX_CHECK_INDEX:
                    debug("check['%s']: when I move to within (%d, %d) -> (%d, %d) call %d:%d (pc-only = %d) \n",
                        debug_index_names[i], box.x, box.y, box.x + box.xd,
                        box.y + box.yd, box.file, box.addr, box.trip);
                    break;
            }
            /*
typedef struct dsl_check_s {
    union {
        box_t box_check;
        tile_t tile_check;
        name_t name_check;
        name2_t name2_check;
    } data;
    uint8_t type;
    uint16_t next;
} dsl_check_t;
*/
        }
    }
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

void dsl_cleanup_vars() {
    free(dsl_global_bnums);
    free(dsl_global_nums);
    free(dsl_local_bnums);
    free(dsl_local_nums);
    free(check);
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

uint8_t preview_byte(uint8_t offset) {
    return *(dsl_data + offset);
}

static uint8_t access_complex(int16_t *header, uint16_t *depth, uint16_t *element) {
    uint16_t i;
    int32_t obj_name;
    
    obj_name = get_word();
    debug("header = %d, depth = %d, element = %d, obj_name = %d\n", *header, *depth, *element, obj_name);
    if (obj_name < 0x8000) {
        debug("access_complex: I need to convert from ID to header!\n");
    } else {
        debug("access_complex: I need to set the *head to the correct view\n");
        switch (obj_name & 0x7FFF) {
            case 0x25: // POV
            case 0x26: // ACTIVE
            case 0x27: // PASSIVE
            case 0x28: // OTHER
            case 0x2C: // OTHER1
            case 0x2B: // THING
                debug("access_complex:valid obj_name(%d), need to set header (but can't yet...)\n", obj_name & 0x7FFF);
                break;
            default:
                return 0;
        }
    }
    *depth = get_byte();
    debug("depth = %d\n", *depth);
    for (i = 1; i <= *depth; i++) {
        element[i-1] = get_byte();
        debug("element[%d] = %d\n", i-1, element[i-1]);
    }

    return 1;
}


/*
static int32_t read_complex(void) {
    int32_t ret = 0;
    uint16_t depth = 0;
    int16_t header = 0;
    uint16_t element[MAX_SEARCH_STACK];
    memset(element, 0x0, sizeof(uint16_t) * MAX_SEARCH_STACK);

    if (access_complex(&header, &depth, element) == 1) {
        debug("reading header (%d) at depth (%d)\n", header, depth);
        ret = get_complex_data(header, depth, element);
        return ret;
    } else {
        printf("read_complex: else not implemented!\n");
        command_implemented = 0;
    }

    return ret;
}
*/

void setrecord() {
    uint16_t depth = 0;
    int16_t header = 0;
    uint16_t element[MAX_SEARCH_STACK];
    uint16_t tmp = peek_half_word();

    if (tmp > 0x8000) {
        access_complex(&header, &depth, element);
        //accum = read_number();
        //smart_write_data(header, depth, element, accum);
        return;
    }
    if (tmp == 0) {
        printf("dsl_setrecord: need to implement party...\n");
        command_implemented = 0;
        return;
    }
    if (tmp < 0x8000) {
        access_complex(&header, &depth, element);
        //set_accumulator(read_number());
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

/*
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
*/

void use_with_check() {
    name2_t name = new_name2;
    int check_index = USE_WITH_CHECK_INDEX;
    int cpos = checks_pos[check_index]; // Where in the list we are.
    if (cpos > MAX_DSL_CHECKS) {
        fprintf(stderr, "FATAL ERROR: Max checks reached! ci = %d\n", check_index);
        exit(1);
    }
    checks[check_index][cpos].data.name2_check = name;
    checks[check_index][cpos].next = cpos + 1;
    checks[check_index][cpos].type = check_index;

    //print_name_check(check_index);

    checks_pos[check_index]++;
    debug("insert with check = {file = %d, addr = %d, name1 = %d, name2 = %d, is_global = %d}\n",
        name.file, name.addr, name.name1,
        name.name2, name.global);
}

void generic_box_check(int check_index, box_t box) {
    int cpos = checks_pos[check_index]; // Where in the list we are.
    if (cpos > MAX_DSL_CHECKS) {
        fprintf(stderr, "FATAL ERROR: Max checks reached! ci = %d\n", check_index);
        exit(1);
    }
    checks[check_index][cpos].data.box_check = box;
    checks[check_index][cpos].next = cpos + 1;
    checks[check_index][cpos].type = check_index;

    //print_name_check(check_index);

    checks_pos[check_index]++;
    debug("tile with check = {file = %d, addr = %d, x = %d, y = %d, xd = %d, yd = %d, trip = %d}\n",
        box.file, box.addr, box.x, box.xd, box.yd,
        box.y, box.trip);
}

void generic_tile_check(int check_index, tile_t tile) {
    int cpos = checks_pos[check_index]; // Where in the list we are.
    if (cpos > MAX_DSL_CHECKS) {
        fprintf(stderr, "FATAL ERROR: Max checks reached! ci = %d\n", check_index);
        exit(1);
    }
    checks[check_index][cpos].data.tile_check = tile;
    checks[check_index][cpos].next = cpos + 1;
    checks[check_index][cpos].type = check_index;

    //print_name_check(check_index);

    checks_pos[check_index]++;
    debug("tile with check = {file = %d, addr = %d, x = %d, y = %d, trip = %d}\n",
        tile.file, tile.addr, tile.x,
        tile.y, tile.trip);
}

static void add_save_orders(int16_t los_order, name_t name, int16_t range, int ordertype) {
    if (name.name < 0) {
        warn("*******************add_save_orders (with name < 0) not implemented****************\n");
        warn("addr = %d, file = %d, name = %d, global = %d\n", name.addr, name.file, name.name, name.global);
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

void print_vars(int what) {
    printf("--------------------------------------print vars------------------\n");
    if (what == 0) {
        for (int i = 0; i < DSL_LFLAGVAR_SIZE; i++) {
            printf("%x, ", dsl_local_flags[i]);
        }
        printf("\n");
    }
}
