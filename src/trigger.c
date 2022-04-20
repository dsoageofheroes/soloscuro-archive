#include "gpl.h"
#include "gpl-manager.h"
#include "gpl-state.h"
#include "region.h"
#include "region-manager.h"
#include "replay.h"
#include "trigger.h"
#include "player.h"
#include "port.h"
#include <stdlib.h>
#include <string.h>

static int trigger_noorders = 0, lposx, lposy;

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
        los_trigger_t los;
    };
    struct trigger_node_s *next;
} trigger_node_t;

static trigger_node_t *attack_list, *noorders_list, *use_list, *look_list, *talkto_list, *usewith_list, *tile_list, *box_list;
static trigger_node_t *los_list;

extern void sol_trigger_init() {
    attack_list = noorders_list = use_list = look_list = talkto_list = usewith_list = tile_list = box_list = los_list = NULL;
    sol_trigger_noorders_disable();
    lposx = lposy = -9999;
}

static void free_list(trigger_node_t *list) {
    trigger_node_t *temp;
    while (list) {
        temp = list;
        list = list->next;
        free(temp);
    }
}

extern void sol_trigger_cleanup() {
    free_list(attack_list);
    free_list(noorders_list);
    free_list(use_list);
    free_list(look_list);
    free_list(talkto_list);
    free_list(usewith_list);
    free_list(tile_list);
    free_list(box_list);
    free_list(los_list);
    sol_trigger_init();
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

extern int sol_trigger_add_attack_global(uint32_t obj, uint32_t file, uint32_t addr) {
    return _add_attack_trigger(obj, file, addr, 1);
}

extern int sol_trigger_add_attack(uint32_t obj, uint32_t file, uint32_t addr) {
    return _add_attack_trigger(obj, file, addr, 0);
}

static int _add_use_trigger(uint32_t obj, uint32_t file, uint32_t addr, const uint8_t is_door, uint32_t global) {
    trigger_node_t *to_add = malloc(sizeof(trigger_node_t));
    to_add->use.obj = obj;
    to_add->use.file = file;
    to_add->use.addr = addr;
    to_add->use.global = global;
    to_add->use.is_door = is_door;
    to_add->next = use_list;
    use_list = to_add;
    return 1;
}

extern int sol_trigger_add_use(uint32_t obj, uint32_t file, uint32_t addr, const uint8_t is_door) {
    return _add_use_trigger(obj, file, addr, is_door, 0);
}

extern int sol_trigger_add_use_global(uint32_t obj, uint32_t file, uint32_t addr) {
    return _add_use_trigger(obj, file, addr, 0, 1);
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

extern int sol_trigger_add_look(uint32_t obj, uint32_t file, uint32_t addr) {
    return _add_look_trigger(obj, file, addr, 0);
}

extern int sol_trigger_add_look_global(uint32_t obj, uint32_t file, uint32_t addr) {
    return _add_look_trigger(obj, file, addr, 1);
}

extern int sol_trigger_add_noorders(uint32_t obj, uint32_t file, uint32_t addr, int trigger, int run) {
    trigger_node_t *to_add = malloc(sizeof(trigger_node_t));
    to_add->noorders.obj = obj;
    to_add->noorders.file = file;
    to_add->noorders.addr = addr;
    to_add->noorders.trigger_on_tile = trigger;
    to_add->noorders.need_to_run = run;
    to_add->next = noorders_list;
    noorders_list = to_add;
    if (obj == 0) {
        error("adding no order for obj 0, this makes no sense.\n");
    }
    return 1;
}

extern int sol_trigger_add_talkto(uint32_t obj, uint32_t file, uint32_t addr) {
    trigger_node_t *to_add = malloc(sizeof(trigger_node_t));
    to_add->talkto.obj = obj;
    to_add->talkto.file = file;
    to_add->talkto.addr = addr;
    to_add->next = talkto_list;
    talkto_list = to_add;
    return 1;
}

extern int sol_trigger_add_usewith(uint32_t obj1, uint32_t obj2, uint32_t file, uint32_t addr) {
    trigger_node_t *to_add = malloc(sizeof(trigger_node_t));
    to_add->usewith.obj1 = obj1;
    to_add->usewith.obj2 = obj2;
    to_add->usewith.file = file;
    to_add->usewith.addr = addr;
    to_add->next = usewith_list;
    usewith_list = to_add;
    return 1;
}

extern int sol_trigger_add_tile(uint32_t x, uint32_t y, uint32_t file, uint32_t addr, uint32_t trip) {
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

extern int sol_trigger_add_box(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t file, uint32_t addr, uint32_t trip) {
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

extern talkto_trigger_t sol_trigger_get_talkto(uint32_t obj) {
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

extern look_trigger_t sol_trigger_get_look(uint32_t obj) {
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

static void list_object_clear (trigger_node_t *list, const uint32_t obj, int (*cmp)(const trigger_node_t *, 
        const uint32_t)) {
    trigger_node_t *rover = list;
    trigger_node_t *prev = NULL;

    while (rover) {
        while (rover && cmp (rover, obj)) {
            if (!prev) {
                attack_list = rover->next;
            } else {
                prev->next = rover->next;
            }
            free(rover);
            rover = prev->next;
        }
        if (rover) {
            prev = rover;
            rover = rover->next;
        }
    }
}

static int _add_los_trigger(uint32_t obj, uint32_t file, uint32_t addr, uint32_t param) {
    trigger_node_t *to_add = malloc(sizeof(trigger_node_t));
    to_add->los.obj = obj;
    to_add->los.file = file;
    to_add->los.addr = addr;
    to_add->los.param = param;
    to_add->next = los_list;
    los_list = to_add;
    printf("adding LOS trigger!\n");
    return 1;
}

extern int sol_trigger_add_los(uint32_t obj, uint32_t file, uint32_t addr, uint32_t param) {
    return _add_los_trigger(obj, file, addr, param);
}

static int attack_equals(const trigger_node_t *node, const uint32_t obj) { return node->attack.obj == obj; }
static int noorders_equals(const trigger_node_t *node, const uint32_t obj) { return node->noorders.obj == obj; }
static int use_equals(const trigger_node_t *node, const uint32_t obj) { return node->use.obj == obj; }
static int look_equals(const trigger_node_t *node, const uint32_t obj) { return node->look.obj == obj; }
static int talk_equals(const trigger_node_t *node, const uint32_t obj) { return node->talkto.obj == obj; }

extern void sol_trigger_object_clear(const uint32_t obj) {
    list_object_clear(attack_list, obj, attack_equals);
    list_object_clear(noorders_list, obj, noorders_equals);
    list_object_clear(use_list, obj, use_equals);
    list_object_clear(look_list, obj, look_equals);
    list_object_clear(talkto_list, obj, talk_equals);
}

extern void sol_trigger_enable_object(const uint32_t obj) {
    for(trigger_node_t *rover = noorders_list; rover; rover = rover->next) {
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
    static trigger_node_t *last_door_trigger = NULL;
    static int32_t last_door_y = -1;
    uint8_t found_door = 0;
    //trigger_node_t *rover = noorders_list;
    if (trigger_noorders) {
        trigger_noorders = 0;
        sol_trigger_noorders_event();
        //printf("trigger_noorders = %d\n", trigger_noorders);
    }
    for(trigger_node_t *rover = use_list; rover; rover = rover->next) {
        if (rover->use.is_door) {
            entity_t *door = sol_region_find_entity_by_id(sol_region_manager_get_current(), rover->use.obj);
            if (!door) { continue; }
            uint32_t width = sol_sprite_getw(door->anim.spr) / settings_zoom();
            width += 15;
            width /= 16;
            for (uint32_t i = 0; i < width; i++) {
                if ((door->mapx + i) == x && door->mapy == y) {
                    found_door = 1;
                    last_door_trigger = rover;
                    //printf("ON DOOR\n");
                }
            }
            //printf("->door: %d %d, width = %d\n", door->mapx, door->mapy, width);
        }
    }

    if (!found_door && last_door_trigger) {
        if (last_door_y != (int32_t)y) {
            debug("trigger door %d\n", last_door_trigger->use.obj);
            gpl_lua_execute_script(last_door_trigger->use.file, last_door_trigger->use.addr, 0);
        }
        last_door_trigger = NULL;
    }

    if (!found_door) { last_door_y = y; }
    //dude_t *dude = sol_region_find_entity_by_id(sol_region_manager_get_current(), rover->noorders.obj);
    dude_t *dude = sol_region_find_entity_by_id(sol_region_manager_get_current(), gpl_get_gname(GNAME_PASSIVE));
    if (!dude) { return; }

    if (on_object(dude, lposx, lposy) && !on_object(dude, x, y)) {
        printf("NEED TO TRIGGER!\n");
        sol_trigger_noorders_event();
    }

    // This will have to be updated a lot... bleh...
    //printf("%d, %d\n", dude->mapx, dude->mapy);
    /*
    if ((dude->mapx == x || dude->mapx - 1 == x )
            && (dude->mapy + 1) == y) {
        trigger_noorders = 0;
        sol_trigger_noorders_event();
        //printf("trigger\n");
    }
    */
    lposx = x;
    lposy = y;

/*
    while (rover) {
        dude_t *dude = sol_region_find_entity_by_id(sol_region_manager_get_current(), rover->noorders.obj);
        //printf("%d: player (%d, %d) vs (%d, %d) \n", rover->noorders.obj, x, y, robj->mapx, robj->mapy);
        if (dude && rover->noorders.trigger_on_tile && (dude->mapx) == x && (dude->mapy) == y) {
            rover->noorders.trigger_on_tile = 0;
            rover->noorders.need_to_run = 1;
        }
        if (!rover->noorders.need_to_run) {
            rover = rover->next;
            continue;
        }

        rover->noorders.need_to_run = 0; // We are going to execute, so disable bit running again.

        // Now execute.
        debug("Noorders executing %d:%d\n", rover->noorders.file, rover->noorders.addr);
        gpl_lua_execute_script(rover->noorders.file, rover->noorders.addr, 0);

        rover = rover->next;
    }
    */
}

extern int sol_trigger_tile_check(uint32_t x, uint32_t y) {
    trigger_node_t *rover = tile_list, *prev = NULL, *hold = NULL;

    while (rover) {
        tile_trigger_t *tile = &(rover->tile);
        //printf("tile trigger: (%d, %d) ? (%d, %d)\n", tile->x, tile->y, x, y);

        if (tile->x == x && tile->y == y) {
            debug("tile triggered:\n");
            if (prev) {
                prev->next = rover->next;
            }else {
                tile_list = rover->next;
            }

            // Take out of list.
            hold = rover;
            rover = rover->next;

            // execute script, box check is not valid.
            gpl_lua_execute_script(tile->file, tile->addr, 0);

            //exit(1);
            //free(rover);
            // put check back in list.
            hold->next = tile_list;
            tile_list = hold;
            return 1;
        }
        prev = rover;
        rover = rover->next;
    }

    return 0;
}

extern void sol_trigger_box_check(uint32_t x, uint32_t y) {
    trigger_node_t *rover = box_list, *prev = NULL, *hold = NULL;

    while (rover) {
        box_trigger_t *box = &(rover->box);
        //printf("(%d, %d) -> (%d, %d) ? (%d, %d) \n", box->x, box->y, box->x + box->w, box->y + box->h, x, y);

        if (box->x <= x && (box->x + box->w) >= x
                && (box->y <= y && (box->y + box->h) >= y)) {
            if (prev) {
                prev->next = rover->next;
            }else {
                box_list = rover->next;
            }

            // Take out of list.
            hold = rover;
            rover = rover->next;

            // execute script, box check is not valid.
            gpl_lua_execute_script(box->file, box->addr, 0);

            // put check back in list.
            hold->next = box_list;
            box_list = hold;
            return;
        }
        prev = rover;
        rover = rover->next;
    }
}

extern void sol_trigger_talk_click(uint32_t obj) {
    talkto_trigger_t tt = sol_trigger_get_talkto(obj);
    look_trigger_t lt = sol_trigger_get_look(obj);

    replay_print("rep.talk_click(%d)\n", obj);
    gpl_set_gname(GNAME_PASSIVE, obj);
    gpl_local_clear();

    if (tt.obj != 0) {
        gpl_lua_execute_script(tt.file, tt.addr, 0);
    }

    if (lt.obj != 0) {
        gpl_lua_execute_script(lt.file, lt.addr, 0);
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

static uint32_t list_size(trigger_node_t *tl) {
    uint32_t len = 0;
    for (trigger_node_t *rover = tl; rover; rover = rover->next) {
        len++;
    }
    return len;
}

static char* write_trigger_list(trigger_node_t *tl, char *buf, size_t *buf_len, size_t *offset) {
    uint32_t list_len = list_size(tl);

    buf = append(buf, offset, buf_len, &list_len, sizeof(uint32_t));
    for (trigger_node_t *rover = tl; rover; rover = rover->next) {
        buf = append(buf, offset, buf_len, &(rover->noorders), sizeof(trigger_node_t));
    }

    return buf;
}

extern char* sol_trigger_serialize(size_t *len) {
    size_t buf_len = 128, offset = 0;
    char *buf = malloc(buf_len);

    buf = write_trigger_list(attack_list, buf, &buf_len, &offset);
    buf = write_trigger_list(noorders_list, buf, &buf_len, &offset);
    buf = write_trigger_list(use_list, buf, &buf_len, &offset);
    buf = write_trigger_list(look_list, buf, &buf_len, &offset);
    buf = write_trigger_list(talkto_list, buf, &buf_len, &offset);
    buf = write_trigger_list(usewith_list, buf, &buf_len, &offset);
    buf = write_trigger_list(tile_list, buf, &buf_len, &offset);
    buf = write_trigger_list(box_list, buf, &buf_len, &offset);

    *len = offset;

    return buf;
}

static char* read_trigger_list(trigger_node_t **list, char *buf) {
    uint32_t amt = *(uint32_t*)buf;

    buf += sizeof(uint32_t);
    for (uint32_t i = 0; i < amt; i++) {
        trigger_node_t *tn = (trigger_node_t*)buf;
        trigger_node_t *to_add = malloc(sizeof(trigger_node_t));
        memcpy(to_add, tn, sizeof(trigger_node_t));
        to_add->next = *list;
        *list = to_add;
        buf += sizeof(trigger_node_t);
    }

    return buf;
}

extern void sol_trigger_deserialize(char *data) {
    sol_trigger_cleanup();

    char *buf = read_trigger_list(&attack_list, data);
    buf = read_trigger_list(&noorders_list, buf);
    buf = read_trigger_list(&use_list, buf);
    buf = read_trigger_list(&look_list, buf);
    buf = read_trigger_list(&talkto_list, buf);
    buf = read_trigger_list(&usewith_list, buf);
    buf = read_trigger_list(&tile_list, buf);
    buf = read_trigger_list(&box_list, buf);
}

extern void sol_write_triggers(FILE *file) {
    for (trigger_node_t *rover = attack_list; rover; rover = rover->next) {
        fprintf(file, "gpl.attack_trigger(%d, %d, %d)\n", rover->attack.obj, rover->attack.file, rover->attack.addr);
    }
    for (trigger_node_t *rover = use_list; rover; rover = rover->next) {
        fprintf(file, "gpl.use_trigger(%d, %d, %d, %d)\n", rover->use.obj, rover->use.file, rover->use.addr, rover->use.is_door);
    }
    for (trigger_node_t *rover = look_list; rover; rover = rover->next) {
        fprintf(file, "gpl.look_trigger(%d, %d, %d)\n", rover->look.obj, rover->look.file, rover->look.addr);
    }
    for (trigger_node_t *rover = talkto_list; rover; rover = rover->next) {
        fprintf(file, "gpl.talk_to_trigger(%d, %d, %d)\n", rover->look.obj, rover->look.file, rover->look.addr);
    }
    for (trigger_node_t *rover = usewith_list; rover; rover = rover->next) {
        fprintf(file, "gpl.use_with_trigger(%d, %d, %d, %d)\n", rover->usewith.obj1, rover->usewith.obj2,
                      rover->usewith.file, rover->usewith.addr);
    }
    for (trigger_node_t *rover = tile_list; rover; rover = rover->next) {
        fprintf(file, "gpl.tile_trigger(%d, %d, %d, %d, %d)\n", rover->tile.x, rover->tile.y, rover->tile.addr,
                      rover->tile.file, rover->tile.trip);
    }
    for (trigger_node_t *rover = box_list; rover; rover = rover->next) {
        fprintf(file, "gpl.box_trigger(%d, %d, %d, %d, %d, %d, %d)\n", rover->box.x, rover->box.y,
                      rover->box.w, rover->box.h, rover->box.addr, rover->box.file, rover->box.trip);
    }
    for (trigger_node_t *rover = noorders_list; rover; rover = rover->next) {
        fprintf(file, "gpl.noorders_trigger(%d, %d, %d, %d, %d)\n", rover->noorders.obj, rover->noorders.file,
                rover->noorders.addr, rover->noorders.trigger_on_tile, rover->noorders.need_to_run);
    }
}

extern void sol_trigger_noorders_event() {
    for(trigger_node_t *rover = noorders_list; rover; rover = rover->next) {
        //printf("checking noorders %d \n", rover->noorders.obj);
        if (rover->noorders.obj == (uint32_t) gpl_get_gname(GNAME_PASSIVE)) {
            //printf("executing %d, %d\n", rover->noorders.file, rover->noorders.addr);
            gpl_lua_execute_script(rover->noorders.file, rover->noorders.addr, 0);
        }
    }
}

extern void sol_trigger_noorders_entity_check(entity_t *entity) {
    if (!entity) { return; }
    for(trigger_node_t *rover = noorders_list; rover; rover = rover->next) {
        //printf("checking noorders %d \n", rover->noorders.obj);
        if (rover->noorders.obj == (uint32_t)entity->ds_id) {
            printf("noorder_entity_check: executing %d, %d\n", rover->noorders.file, rover->noorders.addr);
            gpl_lua_execute_script(rover->noorders.file, rover->noorders.addr, 0);
        }
    }
}

static int in_los(const uint32_t obj, dude_t *entity) {
    entity_t     *los_obj;
    sol_region_t *reg = sol_region_manager_get_current();

    if (!entity || !reg) { return 0; }
    los_obj = sol_region_find_entity_by_id(reg, obj);
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
        if (sol_region_is_block(reg, y, x)) { return 0; }
        if (x != entity->mapx) {
            int py = (x - los_obj->mapx) * dy / dx;
            x += (los_obj->mapx > entity->mapx) ? -1 : 1;
            int cy = (x - los_obj->mapx) * dy / dx;
            if (sol_region_is_block(reg, y, x)) { return 0; }
            // Need to see if y is in the wrong plane.
            //printf("(%d) py = %d, cy = %d\n", y, py, cy);
            while (py != cy) {
                y += (py < cy) ? 1 : -1;
                py += (py < cy) ? 1 : -1;
                if (sol_region_is_block(reg, y, x)) { return 0; }
            }
            //dx / dy
        } else {
            int px = (y - los_obj->mapy) * dx / dy;
            y += (los_obj->mapy > entity->mapy) ? -1 : 1;
            int cx = (y - los_obj->mapy) * dx / dy;
            if (sol_region_is_block(reg, y, x)) { return 0; }
            // Need to see if y is in the wrong plane.
            //printf("(%d) py = %d, cy = %d\n", y, py, cy);
            while (px != cx) {
                x += (px < cx) ? 1 : -1;
                px += (px < cx) ? 1 : -1;
                if (sol_region_is_block(reg, y, x)) { return 0; }
            }
        }
    }
    
    return 1;
}

extern void sol_trigger_set_door(const uint32_t id, const uint8_t is_door) {
    for(trigger_node_t *rover = use_list; rover; rover = rover->next) {
        //printf("%d %d\n", rover->use.obj, id);
        if (rover->use.obj == id) {
            debug ("Setting obj %d to door status of %d\n", id, is_door);
            rover->use.is_door = is_door;
        }
    }
}

extern void sol_trigger_los_check(uint32_t obj, uint32_t file, uint32_t addr, uint32_t param) {
    //printf("los CHECK: %d\n", in_los(obj, sol_player_get_active()));
    if (in_los(obj, sol_player_get_active())) {
        debug("%d is in los, calling %d:%d param= %d\n", obj, file, addr, param);
        gpl_lua_execute_script(file, addr, 0);
    }
}
