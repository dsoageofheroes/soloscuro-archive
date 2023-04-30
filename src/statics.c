#include "region-manager.h"
#include "statics.h"

extern sol_status_t sol_static_from_entity(sol_entity_t *entity, sol_static_t *s) {
    s->ds_id = entity->ds_id;
    s->mapx = entity->mapx;
    s->mapy = entity->mapy;
    s->mapz = entity->mapz;
    s->anim = entity->anim;
    s->flags = entity->object_flags;

    return SOL_SUCCESS;
}

extern sol_status_t sol_static_list_init(sol_static_list_t *ssl) {
    ssl->pos = 0;
    ssl->len = 16;
    ssl->statics = malloc(sizeof(sol_static_t) * ssl->len);

    if (!ssl->statics) {
        ssl->len = 0;
        return SOL_MEMORY_ERROR;
    }

    return SOL_SUCCESS;
}

extern sol_status_t sol_static_list_cleanup(sol_static_list_t *ssl) {
    if (ssl->statics && ssl->len > 0) {
        free(ssl->statics);
    }

    ssl->pos = ssl->len = 0;
    ssl->statics = NULL;

    return SOL_SUCCESS;
}

extern sol_status_t sol_static_list_add(sol_static_list_t *ssl, sol_static_t *s) {
    if (ssl->pos >= ssl->len) {
        sol_static_t *new_statics = realloc(ssl->statics, sizeof(sol_static_t) * (1 + ssl->len * 1.5));
        if (!new_statics) {
            return SOL_MEMORY_ERROR;
        }

        ssl->statics = new_statics;
        ssl->len *= 1.5;
        ssl->len++;
    }

    ssl->statics[ssl->pos++] = *s;

    return SOL_SUCCESS;
}

#include "gff.h"
#include "gfftypes.h"
#include "port.h"
#include "settings.h"
extern sol_status_t sol_static_list_display(sol_static_list_t *ssl) {
    const uint32_t xoffset = sol_get_camerax();
    const uint32_t yoffset = sol_get_cameray();

    for (size_t i = 0; i < ssl->pos; i++) {
        //if (ssl->statics[i].flags & 0x10) { continue; } // Don't draw!
        animate_sprite_t *anim = &ssl->statics[i].anim;
        sol_sprite_set_location(anim->spr,
            anim->x - xoffset, // + scmd_xoffset,
            anim->y - yoffset); // + anim->scmd->yoffset);
        sol_sprite_render_flip(anim->spr, 0, 0);
        //printf("[%d]: 0x%x\n", i, anim->flags);
        //printf("need to display %d: %d (%d x %d)\n", i,
                //ssl->statics[i].anim.spr,
                //sol_sprite_getw(anim->spr),
                //sol_sprite_geth(anim->spr)
                //);
    }
    return SOL_SUCCESS;
}
