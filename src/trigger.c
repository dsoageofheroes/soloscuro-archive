#include "dsl-manager.h"
#include "dsl-state.h"
#include "replay.h"
#include "trigger.h"
#include <stdlib.h>

typedef struct trigger_node_s {
    union {
        attack_trigger_t attack;
        noorders_trigger_t noorders;
        use_trigger_t use;
        look_trigger_t look;
        talkto_trigger_t talkto;
        usewith_trigger_t usewith;
        tile_trigger_t tile;
        box_trigger_t box;
    };
    struct trigger_node_s *next;
} trigger_node_t;

static trigger_node_t *attack_list, *noorders_list, *use_list, *look_list, *talkto_list, *usewith_list, *tile_list, *box_list;

void trigger_init() {
    attack_list = noorders_list = use_list = look_list = talkto_list = usewith_list = tile_list = box_list = NULL;
}

static void free_list(trigger_node_t *list) {
    trigger_node_t *temp;
    while (list) {
        temp = list;
        list = list->next;
        free(temp);
    }
}

void trigger_cleanup() {
    free_list(attack_list);
    free_list(noorders_list);
    free_list(use_list);
    free_list(look_list);
    free_list(talkto_list);
    free_list(usewith_list);
    free_list(tile_list);
    free_list(box_list);
}

static int _add_attack_trigger(uint32_t obj, uint32_t file, uint32_t addr, uint32_t global) {
    trigger_node_t *to_add = malloc(sizeof(trigger_node_t));
    to_add->attack.obj = obj;
    to_add->attack.file = file;
    to_add->attack.addr = addr;
    to_add->attack.global = global;
    to_add->next = attack_list;
    attack_list = to_add;
    return 1;
}

int add_attack_trigger_global(uint32_t obj, uint32_t file, uint32_t addr) {
    return _add_attack_trigger(obj, file, addr, 1);
}

int add_attack_trigger(uint32_t obj, uint32_t file, uint32_t addr) {
    return _add_attack_trigger(obj, file, addr, 0);
}

static int _add_use_trigger(uint32_t obj, uint32_t file, uint32_t addr, uint32_t global) {
    trigger_node_t *to_add = malloc(sizeof(trigger_node_t));
    to_add->use.obj = obj;
    to_add->use.file = file;
    to_add->use.addr = addr;
    to_add->use.global = global;
    to_add->next = use_list;
    use_list = to_add;
    return 1;
}

int add_use_trigger(uint32_t obj, uint32_t file, uint32_t addr) {
    return _add_use_trigger(obj, file, addr, 0);
}

int add_use_trigger_global(uint32_t obj, uint32_t file, uint32_t addr) {
    return _add_use_trigger(obj, file, addr, 1);
}

static int _add_look_trigger(uint32_t obj, uint32_t file, uint32_t addr, uint32_t global) {
    trigger_node_t *to_add = malloc(sizeof(trigger_node_t));
    to_add->look.obj = obj;
    to_add->look.file = file;
    to_add->look.addr = addr;
    to_add->look.global = global;
    to_add->next = look_list;
    look_list = to_add;
    return 1;
}

int add_look_trigger(uint32_t obj, uint32_t file, uint32_t addr) {
    return _add_look_trigger(obj, file, addr, 0);
}

int add_look_trigger_global(uint32_t obj, uint32_t file, uint32_t addr) {
    return _add_look_trigger(obj, file, addr, 1);
}

int add_noorders_trigger(uint32_t obj, uint32_t file, uint32_t addr) {
    trigger_node_t *to_add = malloc(sizeof(trigger_node_t));
    to_add->noorders.obj = obj;
    to_add->noorders.file = file;
    to_add->noorders.addr = addr;
    to_add->next = noorders_list;
    noorders_list = to_add;
    return 1;
}

int add_talkto_trigger(uint32_t obj, uint32_t file, uint32_t addr) {
    trigger_node_t *to_add = malloc(sizeof(trigger_node_t));
    to_add->talkto.obj = obj;
    to_add->talkto.file = file;
    to_add->talkto.addr = addr;
    to_add->next = talkto_list;
    talkto_list = to_add;
    return 1;
}

int add_usewith_trigger(uint32_t obj1, uint32_t obj2, uint32_t file, uint32_t addr) {
    trigger_node_t *to_add = malloc(sizeof(trigger_node_t));
    to_add->usewith.obj1 = obj1;
    to_add->usewith.obj2 = obj2;
    to_add->usewith.file = file;
    to_add->usewith.addr = addr;
    to_add->next = usewith_list;
    usewith_list = to_add;
    return 1;
}

int add_tile_trigger(uint32_t x, uint32_t y, uint32_t file, uint32_t addr, uint32_t trip) {
    trigger_node_t *to_add = malloc(sizeof(trigger_node_t));
    to_add->tile.x = x;
    to_add->tile.y = y;
    to_add->tile.file = file;
    to_add->tile.addr = addr;
    to_add->tile.trip = trip;
    to_add->next = tile_list;
    tile_list = to_add;
    return 1;
}

int add_box_trigger(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t file, uint32_t addr, uint32_t trip) {
    trigger_node_t *to_add = malloc(sizeof(trigger_node_t));
    to_add->box.x = x;
    to_add->box.y = y;
    to_add->box.w = w;
    to_add->box.h = h;
    to_add->box.file = file;
    to_add->box.addr = addr;
    to_add->box.trip = trip;
    to_add->next = box_list;
    box_list = to_add;
    return 0;
}

talkto_trigger_t get_talkto_trigger(uint32_t obj) {
    trigger_node_t *rover = talkto_list;
    talkto_trigger_t ret;
    ret.obj = 0;

    while (rover) {
        if (rover->talkto.obj == obj) {
            return rover->talkto;
        }
        rover = rover->next;
    }

    return ret;
}

look_trigger_t get_look_trigger(uint32_t obj) {
    trigger_node_t *rover = look_list;
    look_trigger_t ret;
    ret.obj = 0;

    while (rover) {
        if (rover->look.obj == obj) {
            return rover->look;
        }
        rover = rover->next;
    }

    return ret;
}

void talk_click(uint32_t obj) {
    talkto_trigger_t tt = get_talkto_trigger(obj);
    look_trigger_t lt = get_look_trigger(obj);

    replay_print("rep.talk_click(%d)\n", obj);
    dsl_set_gname(GNAME_PASSIVE, obj);

    if (tt.obj != 0) {
        dsl_lua_execute_script(tt.file, tt.addr, 0);
    }

    if (lt.obj != 0) {
        dsl_lua_execute_script(lt.file, lt.addr, 0);
    }
}
