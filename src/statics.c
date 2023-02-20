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
    gff_palette_t *pal = open_files[RESOURCE_GFF_INDEX].pals->palettes + 0;
    const uint32_t xoffset = sol_get_camerax();
    const uint32_t yoffset = sol_get_cameray();
    sol_sprite_info_t info;

    for (size_t i = 0; i < ssl->pos; i++) {
        if (ssl->statics[i].flags & 0x10) { continue; } // Don't draw!
        animate_sprite_t *anim = &ssl->statics[i].anim;
        if (ssl->statics[i].anim.spr == SPRITE_ERROR) {
        //sol_sprite_new(pal, 0, 0, zoom, OBJEX_GFF_INDEX, GFF_BMP, entity->anim.bmp_id);
            sol_status_check(sol_sprite_new(pal,
                anim->xoffset,
                anim->yoffset,
                settings_zoom(), OBJEX_GFF_INDEX, GFF_BMP,
                anim->bmp_id, &anim->spr),
                    "Unable to create sprite for static.");
            sol_status_check(sol_sprite_get_info(anim->spr, &info), "Unable to get anim sprite info.");
            anim->w = info.w;
            anim->h = info.h;
            anim->x = (ssl->statics[i].mapx * 16 + anim->xoffset) * settings_zoom();
            anim->y = (ssl->statics[i].mapy * 16 + anim->yoffset + ssl->statics[i].mapz) * settings_zoom();
            anim->destx = anim->x;
            //entity->anim.destx -= sol_sprite_getw(entity->anim.spr) / 2;
            anim->desty = anim->y;
            //sol_sprite_set_frame(anim->spr, anim->load_frame);
        }
        //anim->x = xoffset + (i%20) * 8;
        //anim->y = yoffset + (i/20) * 8;
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
