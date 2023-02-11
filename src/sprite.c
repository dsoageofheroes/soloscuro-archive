/*
 * General sprite functions (not to be confused with implementation of sprite.
 */
#include "sprite.h"
#include "animation.h"

extern sol_status_t sol_sprite_load(struct animate_sprite_s *anim, gff_palette_t *pal, const int gff_index,
                const int type, const uint32_t id, const int num_load) {
    sol_status_t status;

    if (!anim) { return SOL_NULL_ARGUMENT; }

    if (anim->scmd) {
        //error("port_load_sprite not implemented for sprites with a scmd!\n");
        return SOL_NOT_IMPLEMENTED;
    }

    if (anim->spr == SPRITE_ERROR) {
        sol_status_check(sol_sprite_new(pal, 0, 0, settings_zoom(), gff_index, type, id, &anim->spr),
                "Unable to load anims sprite.");

        // Now append anything else needed.
        for (int i = 1; i < num_load; i++) {
            sol_status_check(sol_sprite_append(anim->spr, pal, 0, 0, settings_zoom(),
                gff_index, type, id + i),
                    "Unable to append sprite.");
        }
    }
    //printf("valid = %d\n", sprite_valid(asn->anim->spr));
    return SOL_SUCCESS;
}


