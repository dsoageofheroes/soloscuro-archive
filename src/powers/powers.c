#include <stdlib.h>
#include "powers.h"
#include "wizard.h"
#include "gff.h"
#include "gfftypes.h"
#include "port.h"
#include "gpl.h"
#include "settings.h"
#include "sprite.h"

extern sol_status_t sol_power_create(sol_power_t **p) {
    if (!p) { return SOL_NULL_ARGUMENT; }
    sol_power_t *ret = calloc(1, sizeof(sol_power_t));
    if (!ret) { return SOL_MEMORY_ERROR; }
    ret->hit.spr = ret->thrown.spr = ret->cast.spr = ret->icon.spr = SPRITE_ERROR;
    *p = ret;
    return SOL_SUCCESS;
}

extern sol_status_t sol_power_list_create(sol_power_list_t **p) {
    if (!p) { return SOL_NULL_ARGUMENT; }
    *p = calloc(1, sizeof(sol_power_list_t*));
    if (!*p) { return SOL_MEMORY_ERROR; }
    return SOL_SUCCESS;
}

extern sol_status_t sol_power_list_add(sol_power_list_t *pl, sol_power_t *pw) {
    if (!pl || !pw) { return SOL_NULL_ARGUMENT; }
    sol_power_instance_t *to_add = calloc(1, sizeof(sol_power_instance_t));

    to_add->stats = pw;
    to_add->next = pl->head;
    if (pl->head) {
        pl->head->prev = to_add;
    }
    pl->head = to_add;

    return SOL_SUCCESS;
}

extern sol_status_t sol_power_get_icon(sol_power_t *pw, animate_sprite_t **a) {
    if (!pw) { return SOL_NULL_ARGUMENT; }
    gff_palette_t *pal = open_files[RESOURCE_GFF_INDEX].pals->palettes + 0;

    if (pw->icon.spr == SPRITE_ERROR) {
        sol_sprite_load(&(pw->icon), pal, RESOURCE_GFF_INDEX, GFF_ICON, pw->icon_id, 1);
    }

    *a = &(pw->icon);
    return SOL_SUCCESS;
}

extern sol_status_t sol_power_free(sol_power_t *pw) {
    if (!pw) { return SOL_NULL_ARGUMENT; }
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

    return SOL_SUCCESS;
}

extern sol_status_t sol_power_list_free_instance(sol_power_list_t *pl, sol_power_instance_t *pi) {
    if (!pl || !pi) { return SOL_NULL_ARGUMENT; }

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

    sol_power_free(pi->stats);

    free(pi);

    return SOL_SUCCESS;
}

extern sol_status_t sol_power_list_free(sol_power_list_t *pl) {
    if (!pl) { return SOL_NULL_ARGUMENT; }

    while (pl->head) {
        sol_power_list_free_instance(pl, pl->head);
    }

    free(pl);

    return SOL_SUCCESS;
}

static void get_powers_sprite_info(animate_sprite_t *anim, const uint32_t id) {
    disk_object_t dobj;
    gff_chunk_header_t chunk = gff_find_chunk_header(OBJEX_GFF_INDEX, GFF_OJFF, id);

    if (!gff_read_chunk(OBJEX_GFF_INDEX, &chunk, &dobj, sizeof(disk_object_t))) {
        error("Unable to read disk_obect for OBJEX, OJFF, id = %d!\n", id);
    }

    anim->bmp_id = dobj.bmp_id;
    anim->flags = dobj.flags;
    //anim->xoffset = dobj.xoffset;
    //anim->yoffset = dobj.yoffset;
}

extern sol_status_t sol_powers_set_cast(sol_power_t *power, const uint32_t id) {
    if (!power) { return SOL_NULL_ARGUMENT; }
    get_powers_sprite_info(&(power->cast), id);
    return SOL_SUCCESS;
}

extern sol_status_t sol_powers_set_icon(sol_power_t *power, const uint32_t id) {
    gff_palette_t *pal = open_files[RESOURCE_GFF_INDEX].pals->palettes + 0;
    if (!power) { return SOL_NULL_ARGUMENT; }

    power->icon_id = id;
    sol_sprite_load(&(power->icon), pal, RESOURCE_GFF_INDEX, GFF_ICON, id, 1);
    return SOL_SUCCESS;
}

extern sol_status_t sol_powers_set_thrown(sol_power_t *power, const uint32_t id) {
    if (!power) { return SOL_SUCCESS; }
    get_powers_sprite_info(&(power->thrown), id);
    return SOL_SUCCESS;
}

extern sol_status_t sol_powers_set_hit(sol_power_t *power, const uint32_t id) {
    if (!power) { return SOL_NULL_ARGUMENT; }

    get_powers_sprite_info(&(power->hit), id);
    return SOL_SUCCESS;
}

extern sol_status_t sol_powers_init() {
    wizard_init();
    wizard_setup_powers();
    return SOL_SUCCESS;
}

extern sol_status_t powers_cleanup() {
    wizard_cleanup();
    return SOL_SUCCESS;
}

extern sol_status_t sol_power_select_by_game(const size_t ds1, const size_t ds2, const size_t dso, size_t *what) {
    switch(gff_get_game_type()) {
        case DARKSUN_1: *what = ds1; break;
        case DARKSUN_2: *what = ds2; break;
        case DARKSUN_ONLINE: *what = dso; break;
        default: *what = dso; break;
    }
    return SOL_SUCCESS;
}

extern sol_status_t sol_power_spin_read_description(const uint16_t id, char **msg) {
    if (!msg) { return SOL_NULL_ARGUMENT; }
    char *description = NULL;

    gff_chunk_header_t chunk = gff_find_chunk_header(RESOURCE_GFF_INDEX, GFF_SPIN, id);
    if (chunk.length <= 0) { return SOL_OUT_OF_RANGE; }
    description = calloc(1, chunk.length + 1);
    if (!description) { return SOL_MEMORY_ERROR; }
    gff_read_chunk(RESOURCE_GFF_INDEX, &chunk, description, chunk.length);

    *msg = description;
    return SOL_SUCCESS;
}

extern sol_status_t sol_power_get_target_type(sol_power_t *power, enum sol_target_shape_e *e) {
    if (!power || !e) { return SOL_NULL_ARGUMENT; }
    *e = power->shape;
    return SOL_SUCCESS;
}

static void load_power_sprite(animate_sprite_t *spr, combat_scmd_t type) {
    gff_palette_t *pal = open_files[RESOURCE_GFF_INDEX].pals->palettes + 0;

    sol_status_check(
        sol_sprite_new(pal, 0, 0, settings_zoom(),
            OBJEX_GFF_INDEX, GFF_BMP, spr->bmp_id, &(spr->spr)),
        "Unable to create sprite for power.");

    if (spr->spr && spr->spr != SPRITE_ERROR) {
        spr->scmd = sol_combat_get_scmd(type);
    }
}

extern sol_status_t powers_load(sol_power_t *power) {
    if (!power) { return SOL_NULL_ARGUMENT; }
    if (power->cast.spr == 0 || power->cast.spr == SPRITE_ERROR) {
        load_power_sprite(&(power->cast), COMBAT_POWER_CAST);
    }
    if (power->thrown.spr == 0 || power->thrown.spr == SPRITE_ERROR) {
        load_power_sprite(&(power->thrown), COMBAT_POWER_THROW);
    }
    if (power->hit.spr == 0 || power->hit.spr == SPRITE_ERROR) {
        load_power_sprite(&(power->hit), COMBAT_POWER_HIT);
    }
    return SOL_SUCCESS;
}
