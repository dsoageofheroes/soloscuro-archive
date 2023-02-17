#include "gpl.h"
#include "gpl-manager.h"
#include "gpl-state.h"
#include "region.h"
#include "region-manager.h"
#include "trigger.h"
#include "player.h"
#include "port.h"
#include <stdlib.h>
#include <string.h>

// It looks like noorders should be triggered:
// 0) Every 60-300 game ticks (confirmed, but number is unknown.)
// 1) On entry of area (unconfirmed)
// 2) When the object attached to it is moved (animated.) (Unconfirmed.)
// 3) When character moves off of it via y-axis.  (Unconfirmed.)

static int trigger_noorders = 0;
static sol_triggers_t *triggers = NULL;
static sol_triggers_t global_triggers;

extern void sol_trigger_init() {
    memset(&global_triggers, 0x0, sizeof(sol_triggers_t));
    sol_trigger_noorders_disable();
}

static void free_list(sol_trigger_node_t *list) {
    sol_trigger_node_t *temp;
    while (list) {
        temp = list;
        list = list->next;
        free(temp);
    }
}

extern void sol_trigger_cleanup() {
    if (!triggers) { return; }
    free_list(triggers->attack_list);
    free_list(triggers->noorders_list);
    free_list(triggers->use_list);
    free_list(triggers->look_list);
    free_list(triggers->talkto_list);
    free_list(triggers->usewith_list);
    free_list(triggers->tile_list);
    free_list(triggers->box_list);
    free_list(triggers->los_list);
    triggers = NULL;
    sol_trigger_noorders_disable();
}

extern void sol_trigger_set_region(sol_region_t *reg) {
    triggers = &(reg->triggers);
}

static int _add_attack_trigger(uint32_t obj, uint32_t file, uint32_t addr, sol_triggers_t *trig) {
    sol_trigger_node_t *to_add = malloc(sizeof(sol_trigger_node_t));
    to_add->attack.obj = obj;
    to_add->attack.file = file;
    to_add->attack.addr = addr;
    to_add->next = trig->attack_list;
    trig->attack_list = to_add;
    return 1;
}

extern int sol_trigger_add_attack_global(uint32_t obj, uint32_t file, uint32_t addr) {
    return _add_attack_trigger(obj, file, addr, &global_triggers);
}

extern int sol_trigger_add_attack(uint32_t obj, uint32_t file, uint32_t addr) {
    return _add_attack_trigger(obj, file, addr, triggers);
}

static int _add_use_trigger(uint32_t obj, uint32_t file, uint32_t addr, sol_triggers_t *trig) {
    sol_trigger_node_t *to_add = malloc(sizeof(sol_trigger_node_t));
    to_add->use.obj = obj;
    to_add->use.file = file;
    to_add->use.addr = addr;
    to_add->next = trig->use_list;
    trig->use_list = to_add;
    return 1;
}

extern int sol_trigger_add_use(uint32_t obj, uint32_t file, uint32_t addr) {
    return _add_use_trigger(obj, file, addr, triggers);
}

extern int sol_trigger_add_use_global(uint32_t obj, uint32_t file, uint32_t addr) {
    return _add_use_trigger(obj, file, addr, &global_triggers);
}

static int _add_look_trigger(uint32_t obj, uint32_t file, uint32_t addr, sol_triggers_t *trig) {
    sol_trigger_node_t *to_add = malloc(sizeof(sol_trigger_node_t));
    to_add->look.obj = obj;
    to_add->look.file = file;
    to_add->look.addr = addr;
    to_add->next = trig->look_list;
    trig->look_list = to_add;
    return 1;
}

extern int sol_trigger_add_look(uint32_t obj, uint32_t file, uint32_t addr) {
    return _add_look_trigger(obj, file, addr, triggers);
}

extern int sol_trigger_add_look_global(uint32_t obj, uint32_t file, uint32_t addr) {
    return _add_look_trigger(obj, file, addr, &global_triggers);
}

extern int sol_trigger_add_noorders(uint32_t obj, uint32_t file, uint32_t addr, int trigger, int run) {
    sol_trigger_node_t *to_add = malloc(sizeof(sol_trigger_node_t));
    to_add->noorders.obj = obj;
    to_add->noorders.file = file;
    to_add->noorders.addr = addr;
    to_add->noorders.trigger_on_tile = trigger;
    to_add->noorders.need_to_run = run;
    to_add->next = triggers->noorders_list;
    triggers->noorders_list = to_add;
    if (obj == 0) {
        error("adding no order for obj 0, this makes no sense.\n");
    }
    return 1;
}

extern int sol_trigger_add_talkto(uint32_t obj, uint32_t file, uint32_t addr) {
    sol_trigger_node_t *to_add = malloc(sizeof(sol_trigger_node_t));
    to_add->talkto.obj = obj;
    to_add->talkto.file = file;
    to_add->talkto.addr = addr;
    to_add->next = triggers->talkto_list;
    triggers->talkto_list = to_add;
    return 1;
}

extern int sol_trigger_add_usewith(uint32_t obj1, uint32_t obj2, uint32_t file, uint32_t addr) {
    sol_trigger_node_t *to_add = malloc(sizeof(sol_trigger_node_t));
    to_add->usewith.obj1 = obj1;
    to_add->usewith.obj2 = obj2;
    to_add->usewith.file = file;
    to_add->usewith.addr = addr;
    to_add->next = triggers->usewith_list;
    triggers->usewith_list = to_add;
    return 1;
}

extern int sol_trigger_add_tile(uint32_t x, uint32_t y, uint32_t file, uint32_t addr, uint32_t trip) {
    sol_trigger_node_t *to_add = malloc(sizeof(sol_trigger_node_t));
    to_add->tile.x = x;
    to_add->tile.y = y;
    to_add->tile.file = file;
    to_add->tile.addr = addr;
    to_add->tile.trip = trip;
    to_add->next = triggers->tile_list;
    triggers->tile_list = to_add;
    return 1;
}

extern int sol_trigger_add_box(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t file, uint32_t addr, uint32_t trip) {
    sol_trigger_node_t *to_add = malloc(sizeof(sol_trigger_node_t));
    to_add->box.x = x;
    to_add->box.y = y;
    to_add->box.w = w;
    to_add->box.h = h;
    to_add->box.file = file;
    to_add->box.addr = addr;
    to_add->box.trip = trip;
    to_add->next = triggers->box_list;
    triggers->box_list = to_add;
    return 0;
}

extern sol_talkto_trigger_t sol_trigger_get_talkto(uint32_t obj) {
    sol_trigger_node_t *rover = triggers->talkto_list;
    sol_talkto_trigger_t ret;
    ret.obj = 0;

    while (rover) {
        if (rover->talkto.obj == obj) {
            return rover->talkto;
        }
        rover = rover->next;
    }

    return ret;
}

extern sol_look_trigger_t sol_trigger_get_look(uint32_t obj) {
    sol_trigger_node_t *rover = triggers->look_list;
    sol_look_trigger_t ret;
    ret.obj = 0;

    while (rover) {
        if (rover->look.obj == obj) {
            return rover->look;
        }
        rover = rover->next;
    }

    return ret;
}

extern sol_use_trigger_t sol_trigger_get_use(uint32_t obj) {
    sol_trigger_node_t *rover = triggers->use_list;
    sol_use_trigger_t ret;
    ret.obj = 0;

    while (rover) {
        if (rover->use.obj == obj) {
            return rover->use;
        }
        rover = rover->next;
    }

    return ret;
}

static void list_object_clear (sol_trigger_node_t **list, const uint32_t obj, int (*cmp)(const sol_trigger_node_t *, 
        const uint32_t)) {
    sol_trigger_node_t *rover = *list;
    sol_trigger_node_t *prev = NULL, *to_delete = NULL;

    while (rover) {
        while (rover && cmp (rover, obj)) {
            to_delete = rover;
            rover = rover->next;
            if (to_delete == *list) {
                *list = rover;
            } else {
                prev->next = rover;
            }
            free(to_delete);
            to_delete = NULL;
            continue;
        }
        if (rover) {
            prev = rover;
            rover = rover->next;
        }
    }
}

static int _add_los_trigger(uint32_t obj, uint32_t file, uint32_t addr, uint32_t param) {
    sol_trigger_node_t *to_add = malloc(sizeof(sol_trigger_node_t));
    to_add->los.obj = obj;
    to_add->los.file = file;
    to_add->los.addr = addr;
    to_add->los.param = param;
    to_add->next = triggers->los_list;
    triggers->los_list = to_add;
    return 1;
}

extern int sol_trigger_add_los(uint32_t obj, uint32_t file, uint32_t addr, uint32_t param) {
    return _add_los_trigger(obj, file, addr, param);
}

static int attack_equals(const sol_trigger_node_t *node, const uint32_t obj) { return node->attack.obj == obj; }
static int noorders_equals(const sol_trigger_node_t *node, const uint32_t obj) { return node->noorders.obj == obj; }
static int use_equals(const sol_trigger_node_t *node, const uint32_t obj) { return node->use.obj == obj; }
static int look_equals(const sol_trigger_node_t *node, const uint32_t obj) { return node->look.obj == obj; }
static int talk_equals(const sol_trigger_node_t *node, const uint32_t obj) { return node->talkto.obj == obj; }

extern void sol_trigger_object_clear(const uint32_t obj) {
    if (!triggers) { return; }

    list_object_clear(&(triggers->attack_list), obj, attack_equals);
    list_object_clear(&(triggers->noorders_list), obj, noorders_equals);
    list_object_clear(&(triggers->use_list), obj, use_equals);
    list_object_clear(&(triggers->look_list), obj, look_equals);
    list_object_clear(&(triggers->talkto_list), obj, talk_equals);
}

extern void sol_trigger_enable_object(const uint32_t obj) {
    for(sol_trigger_node_t *rover = triggers->noorders_list; rover; rover = rover->next) {
        if (rover->noorders.obj == obj) {
            rover->noorders.need_to_run = 1;
        }
    }
}

extern void sol_trigger_noorders_enable()  { trigger_noorders = 1; }
extern void sol_trigger_noorders_disable() { trigger_noorders = 0; }

static int on_object(dude_t *dude, const uint32_t x, const uint32_t y) {
    if (!dude) { return 0; }

    return ((dude->mapx == x || dude->mapx - 1 == x )
            && (dude->mapy) == y);
}

extern void sol_trigger_noorders(uint32_t x, uint32_t y) {
    static sol_trigger_node_t *last_door_trigger = NULL;
    static int32_t last_door_y = -1;
    uint8_t found_door = 0;
    //sol_trigger_node_t *rover = triggers->noorders_list;
    if (trigger_noorders) {
        trigger_noorders = 0;
        sol_trigger_noorders_event();
        //printf("trigger_noorders = %d\n", trigger_noorders);
    }
}

extern int sol_trigger_tile_check(uint32_t x, uint32_t y) {
    sol_trigger_node_t *rover = triggers->tile_list, *prev = NULL, *hold = NULL;

    while (rover) {
        sol_tile_trigger_t *tile = &(rover->tile);
        //printf("tile trigger: (%d, %d) ? (%d, %d)\n", tile->x, tile->y, x, y);

        if (tile->x == x && tile->y == y) {
            debug("tile triggered:\n");
            if (prev) {
                prev->next = rover->next;
            }else {
                triggers->tile_list = rover->next;
            }

            // Take out of list.
            hold = rover;
            rover = rover->next;

            // execute script, box check is not valid.
            sol_gpl_lua_execute_script(tile->file, tile->addr, 0);

            //exit(1);
            //free(rover);
            // put check back in list.
            hold->next = triggers->tile_list;
            triggers->tile_list = hold;
            return 1;
        }
        prev = rover;
        rover = rover->next;
    }

    return 0;
}

extern void sol_trigger_box_check(uint32_t x, uint32_t y) {
    sol_trigger_node_t *rover = triggers->box_list, *prev = NULL, *hold = NULL;

    while (rover) {
        sol_box_trigger_t *box = &(rover->box);
        //printf("(%d, %d) -> (%d, %d) ? (%d, %d) \n", box->x, box->y, box->x + box->w, box->y + box->h, x, y);

        if (box->x <= x && (box->x + box->w) >= x
                && (box->y <= y && (box->y + box->h) >= y)) {
            if (prev) {
                prev->next = rover->next;
            }else {
                triggers->box_list = rover->next;
            }

            // Take out of list.
            hold = rover;
            rover = rover->next;

            // execute script, box check is not valid.
            sol_gpl_lua_execute_script(box->file, box->addr, 0);

            // put check back in list.
            hold->next = triggers->box_list;
            triggers->box_list = hold;
            return;
        }
        prev = rover;
        rover = rover->next;
    }
}

extern void sol_trigger_use(uint32_t obj) {
    sol_use_trigger_t ut = sol_trigger_get_use(obj);
    if (sol_gpl_lua_execute_script(ut.file, ut.addr, 0) != SOL_SUCCESS) {
        // couldn't execute, lets try annotations.
        //sol_annotations_call_func(ut.file, ut.addr, obj);
    }
}

extern void sol_trigger_talk_click(uint32_t obj) {
    sol_talkto_trigger_t tt = sol_trigger_get_talkto(obj);
    sol_look_trigger_t lt = sol_trigger_get_look(obj);

    sol_gpl_set_gname(GNAME_PASSIVE, obj);
    sol_gpl_local_clear();

    if (tt.obj != 0) {
        sol_gpl_lua_execute_script(tt.file, tt.addr, 0);
    }

    if (lt.obj != 0) {
        sol_gpl_lua_execute_script(lt.file, lt.addr, 0);
    }
}

static char* append(char *buf, size_t *offset, size_t *len, const void *data, const size_t data_len) {
    while ((*offset + data_len) >= *len) {
        *len *= 1.5;
        buf = realloc(buf, *len);
    }

    memcpy(buf + *offset, data, data_len);
    *offset += data_len;

    return buf;
}

static uint32_t list_size(sol_trigger_node_t *tl) {
    uint32_t len = 0;
    for (sol_trigger_node_t *rover = tl; rover; rover = rover->next) {
        len++;
    }
    return len;
}

static char* write_trigger_list(sol_trigger_node_t *tl, char *buf, size_t *buf_len, size_t *offset) {
    uint32_t list_len = list_size(tl);

    buf = append(buf, offset, buf_len, &list_len, sizeof(uint32_t));
    for (sol_trigger_node_t *rover = tl; rover; rover = rover->next) {
        buf = append(buf, offset, buf_len, &(rover->noorders), sizeof(sol_trigger_node_t));
    }

    return buf;
}

extern char* sol_trigger_serialize(size_t *len) {
    size_t buf_len = 128, offset = 0;
    char *buf = malloc(buf_len);

    buf = write_trigger_list(triggers->attack_list, buf, &buf_len, &offset);
    buf = write_trigger_list(triggers->noorders_list, buf, &buf_len, &offset);
    buf = write_trigger_list(triggers->use_list, buf, &buf_len, &offset);
    buf = write_trigger_list(triggers->look_list, buf, &buf_len, &offset);
    buf = write_trigger_list(triggers->talkto_list, buf, &buf_len, &offset);
    buf = write_trigger_list(triggers->usewith_list, buf, &buf_len, &offset);
    buf = write_trigger_list(triggers->tile_list, buf, &buf_len, &offset);
    buf = write_trigger_list(triggers->box_list, buf, &buf_len, &offset);

    *len = offset;

    return buf;
}

static char* read_trigger_list(sol_trigger_node_t **list, char *buf) {
    uint32_t amt = *(uint32_t*)buf;

    buf += sizeof(uint32_t);
    for (uint32_t i = 0; i < amt; i++) {
        sol_trigger_node_t *tn = (sol_trigger_node_t*)buf;
        sol_trigger_node_t *to_add = malloc(sizeof(sol_trigger_node_t));
        memcpy(to_add, tn, sizeof(sol_trigger_node_t));
        to_add->next = *list;
        *list = to_add;
        buf += sizeof(sol_trigger_node_t);
    }

    return buf;
}

extern void sol_trigger_deserialize(char *data) {
    sol_trigger_cleanup();

    char *buf = read_trigger_list(&triggers->attack_list, data);
    buf = read_trigger_list(&triggers->noorders_list, buf);
    buf = read_trigger_list(&triggers->use_list, buf);
    buf = read_trigger_list(&triggers->look_list, buf);
    buf = read_trigger_list(&triggers->talkto_list, buf);
    buf = read_trigger_list(&triggers->usewith_list, buf);
    buf = read_trigger_list(&triggers->tile_list, buf);
    buf = read_trigger_list(&triggers->box_list, buf);
}

extern void sol_write_triggers(FILE *file) {
    for (sol_trigger_node_t *rover = triggers->attack_list; rover; rover = rover->next) {
        fprintf(file, "gpl.attack_trigger(%d, %d, %d)\n", rover->attack.obj, rover->attack.file, rover->attack.addr);
    }
    for (sol_trigger_node_t *rover = triggers->use_list; rover; rover = rover->next) {
        fprintf(file, "gpl.use_trigger(%d, %d, %d)\n", rover->use.obj, rover->use.file, rover->use.addr);
    }
    for (sol_trigger_node_t *rover = triggers->look_list; rover; rover = rover->next) {
        fprintf(file, "gpl.look_trigger(%d, %d, %d)\n", rover->look.obj, rover->look.file, rover->look.addr);
    }
    for (sol_trigger_node_t *rover = triggers->talkto_list; rover; rover = rover->next) {
        fprintf(file, "gpl.talk_to_trigger(%d, %d, %d)\n", rover->look.obj, rover->look.file, rover->look.addr);
    }
    for (sol_trigger_node_t *rover = triggers->usewith_list; rover; rover = rover->next) {
        fprintf(file, "gpl.use_with_trigger(%d, %d, %d, %d)\n", rover->usewith.obj1, rover->usewith.obj2,
                      rover->usewith.file, rover->usewith.addr);
    }
    for (sol_trigger_node_t *rover = triggers->tile_list; rover; rover = rover->next) {
        fprintf(file, "gpl.tile_trigger(%d, %d, %d, %d, %d)\n", rover->tile.x, rover->tile.y, rover->tile.addr,
                      rover->tile.file, rover->tile.trip);
    }
    for (sol_trigger_node_t *rover = triggers->box_list; rover; rover = rover->next) {
        fprintf(file, "gpl.box_trigger(%d, %d, %d, %d, %d, %d, %d)\n", rover->box.x, rover->box.y,
                      rover->box.w, rover->box.h, rover->box.addr, rover->box.file, rover->box.trip);
    }
    for (sol_trigger_node_t *rover = triggers->noorders_list; rover; rover = rover->next) {
        fprintf(file, "gpl.noorders_trigger(%d, %d, %d, %d, %d)\n", rover->noorders.obj, rover->noorders.file,
                rover->noorders.addr, rover->noorders.trigger_on_tile, rover->noorders.need_to_run);
    }
}

extern void sol_trigger_noorders_event() {
    if (!triggers) { return; }

    for(sol_trigger_node_t *rover = triggers->noorders_list; rover; rover = rover->next) {
        sol_gpl_lua_execute_script(rover->noorders.file, rover->noorders.addr, 0);
    }
}

extern void sol_trigger_noorders_entity_check(entity_t *entity) {
    if (!entity) { return; }
    for(sol_trigger_node_t *rover = triggers->noorders_list; rover; rover = rover->next) {
        printf("checking %d vs %d\n", rover->noorders.obj, entity->ds_id);
        if (rover->noorders.obj == (uint32_t)entity->ds_id) {
            printf("noorder_entity_check: executing %d, %d\n", rover->noorders.file, rover->noorders.addr);
            sol_gpl_lua_execute_script(rover->noorders.file, rover->noorders.addr, 0);
        }
    }
}

static int in_los(const uint32_t obj, dude_t *entity) {
    entity_t     *los_obj;
    sol_region_t *reg;

    sol_region_manager_get_current(&reg);
    if (!entity || !reg) { return 0; }
    sol_region_find_entity_by_id(reg, obj, &los_obj);
    if (!los_obj) { return 0; }

    int dx = entity->mapx - los_obj->mapx;
    int dy = entity->mapy - los_obj->mapy;
    int x = los_obj->mapx;
    int y = los_obj->mapy;
// (2,2) -> (9, 5)
//  dx = 7, dy = 3
//  ratio is 9 / 3
// -7 * 3 = -21 / 7 = -3
    //printf("(%d, %d) -> (%d, %d)\n", los_obj->mapx, los_obj->mapy, entity->mapx, entity->mapy);
    // TODO: This needs testing badly...
    while (x != entity->mapx || y != entity->mapy) {
        //printf("(%d, %d)\n", x, y);
        if (sol_region_is_block(reg, y, x) == SOL_SUCCESS) { return 0; }
        if (x != entity->mapx) {
            int py = (x - los_obj->mapx) * dy / dx;
            x += (los_obj->mapx > entity->mapx) ? -1 : 1;
            int cy = (x - los_obj->mapx) * dy / dx;
            if (sol_region_is_block(reg, y, x) == SOL_SUCCESS) { return 0; }
            // Need to see if y is in the wrong plane.
            //printf("(%d) py = %d, cy = %d\n", y, py, cy);
            while (py != cy) {
                y += (py < cy) ? 1 : -1;
                py += (py < cy) ? 1 : -1;
                if (sol_region_is_block(reg, y, x) == SOL_SUCCESS) { return 0; }
            }
            //dx / dy
        } else {
            int px = (y - los_obj->mapy) * dx / dy;
            y += (los_obj->mapy > entity->mapy) ? -1 : 1;
            int cx = (y - los_obj->mapy) * dx / dy;
            if (sol_region_is_block(reg, y, x) == SOL_SUCCESS) { return 0; }
            // Need to see if y is in the wrong plane.
            //printf("(%d) py = %d, cy = %d\n", y, py, cy);
            while (px != cx) {
                x += (px < cx) ? 1 : -1;
                px += (px < cx) ? 1 : -1;
                if (sol_region_is_block(reg, y, x) == SOL_SUCCESS) { return 0; }
            }
        }
    }
    
    return 1;
}

extern void sol_trigger_los_check(uint32_t obj, uint32_t file, uint32_t addr, uint32_t param) {
    //printf("los CHECK: %d\n", in_los(obj, sol_player_get_active()));
    sol_dude_t *player;
    sol_player_get_active(&player);
    if (in_los(obj, player)) {
        debug("%d is in los, calling %d:%d param= %d\n", obj, file, addr, param);
        sol_gpl_lua_execute_script(file, addr, 0);
    }
}
