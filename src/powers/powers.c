#include <stdlib.h>
#include "powers.h"
#include "wizard.h"
#include "gff.h"
#include "gfftypes.h"
#include "port.h"

extern power_t* power_create() {
    power_t *ret = calloc(1, sizeof(power_t));
    ret->hit.spr = ret->thrown.spr = ret->cast.spr = ret->icon.spr = SPRITE_ERROR;
    return ret;
}

extern power_list_t* power_list_create() {
    return calloc(1, sizeof(power_list_t*));
}

extern void power_list_add(power_list_t *pl, power_t *pw) {
    if (!pl || !pw) { return; }
    power_instance_t *to_add = calloc(1, sizeof(power_instance_t));

    to_add->stats = pw;
    to_add->next = pl->head;
    if (pl->head) {
        pl->head->prev = to_add;
    }
    pl->head = to_add;
}

extern animate_sprite_t* power_get_icon(power_t *pw) {
    gff_palette_t *pal = open_files[RESOURCE_GFF_INDEX].pals->palettes + 0;

    if (!pw) { return NULL; }

    if (pw->icon.spr == SPRITE_ERROR) {
        port_load_sprite(&(pw->icon), pal, RESOURCE_GFF_INDEX, GFF_ICON, pw->icon_id, 1);
    }

    return &(pw->icon);
}

extern void power_free(power_t *pw) {
    /*if (pw->icon.data) {
        port_free_sprite(&(pw->icon));
        pw->icon.data = NULL;
    }
    if (pw->thrown.data) {
        port_free_sprite(&(pw->thrown));
        pw->thrown.data = NULL;
    }
    if (pw->hit.data) {
        port_free_sprite(&(pw->hit));
        pw->hit.data = NULL;
    }
    if (pw->cast.data) {
        port_free_sprite(&(pw->cast));
        pw->cast.data = NULL;
    }
    */
    if (pw->description) {
        free(pw->description);
        pw->description = NULL;
    }
    free(pw);
}

extern void power_list_free_instance(power_list_t *pl, power_instance_t *pi) {
    if (!pl || !pi) { return; }

    if (pl->head == pi) {
        pl->head = pi->next;
        if (pl->head) {
            pl->head->prev = NULL;
        }
    } else {
        if (pi->prev) {
            pi->prev->next = pi->next;
        }
        if (pi->next) {
            pi->next->prev = pi->prev;
        }
    }

    power_free(pi->stats);

    free(pi);
}

/*static void set_sprite_from_ojff(sprite_info_t *spr, const uint32_t id) {
    disk_object_t dobj;
    gff_chunk_header_t chunk = gff_find_chunk_header(OBJEX_GFF_INDEX, GFF_OJFF, id);
    if (chunk.length <= 0) { return; }

    gff_read_chunk(OBJEX_GFF_INDEX, &chunk, &dobj, sizeof(dobj));

    spr->bmp_id = dobj.bmp_id;
    spr->flags = dobj.flags;
    spr->xoffset = dobj.xoffset;
    spr->yoffset = dobj.yoffset;
    //printf("id = %d, length = %d, bmp_id = %d", id, chunk.length, spr->bmp_id);
    //spr->anim.scmd = ssi_scmd_get(OBJEX_GFF_INDEX, dobj.script_id, 0);
}
*/

extern void power_list_free(power_list_t *pl) {
    if (!pl) { return; }

    while (pl->head) {
        power_list_free_instance(pl, pl->head);
    }

    free(pl);
}

extern void powers_set_cast(power_t *power, const uint32_t id) {
    if (!power) { return; }
    //set_sprite_from_ojff(&(power->cast), id);
}

extern void powers_set_icon(power_t *power, const uint32_t id) {
    gff_palette_t *pal = open_files[RESOURCE_GFF_INDEX].pals->palettes + 0;
    if (!power) { return; }

    power->icon_id = id;
    port_load_sprite(&(power->icon), pal, RESOURCE_GFF_INDEX, GFF_ICON, id, 1);
}

extern void powers_set_thrown(power_t *power, const uint32_t id) {
    if (!power) { return; }
    //set_sprite_from_ojff(&(power->thrown), id);
}

extern void powers_set_hit(power_t *power, const uint32_t id) {
    if (!power) { return; }
    //set_sprite_from_ojff(&(power->hit), id);
}

extern void powers_init() {
    wizard_init();
    wizard_setup_powers();
}

extern void powers_cleanup() {
    wizard_cleanup();
}

extern size_t power_select_by_game(const size_t ds1, const size_t ds2, const size_t dso) {
    switch(gff_get_game_type()) {
        case DARKSUN_1: return ds1;
        case DARKSUN_2: return ds2;
        case DARKSUN_ONLINE: return dso;
        default: break;
    }
    return dso; // By default, dso is always right.
}

extern char* power_spin_read_description(const uint16_t id) {
    char *description = NULL;

    gff_chunk_header_t chunk = gff_find_chunk_header(RESOURCE_GFF_INDEX, GFF_SPIN, id);
    if (chunk.length <= 0) { return NULL; }
    description = calloc(1, chunk.length + 1);
    gff_read_chunk(RESOURCE_GFF_INDEX, &chunk, description, chunk.length);

    return description;
}

extern enum target_shape power_get_target_type(power_t *power) {
    if (!power) { return TARGET_NONE; }
    return power->shape;
}

//static void load_power_sprite(sprite_info_t *spr) {
    //gff_palette_t *pal = open_files[RESOURCE_GFF_INDEX].pals->palettes + 0;
    //port_load_sprite(spr, pal, OBJEX_GFF_INDEX, GFF_BMP, spr->bmp_id);
//}

extern void power_load(power_t *power) {
    if (!power) { return; }
    /*
    if (!power->cast.data) {
        load_power_sprite(&(power->cast));
    }
    if (!power->thrown.data) {
        load_power_sprite(&(power->thrown));
    }
    if (!power->hit.data) {
        load_power_sprite(&(power->hit));
    }
    */
}
