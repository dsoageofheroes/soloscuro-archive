#include <string.h>
#include <stdlib.h>
#include "animate.h"
#include "gameloop.h"
#include "../src/dsl.h"
#include "../src/dsl-scmd.h"

//#define MAX_DELAY (256)
static animate_t *list;
static scmd_t no_animate;
//static size_t animate_pos = 0;

//typedef struct animation_delay_s {
    //animate_t *anim;
    //struct animation_delay_s *next, *prev;
//} animation_delay_t;

//static animation_delay_t *animations[MAX_DELAY];

void animate_init() {
    //memset(animations, 0x0, sizeof(animation_delay_t*) * MAX_DELAY);
    list = NULL;
    no_animate.bmp_idx = 0;
    no_animate.delay = 0;
    no_animate.flags = SCMD_LAST;
    no_animate.xoffset = 0;
    no_animate.yoffset = 0;
    no_animate.xoffsethot = 0;
    no_animate.yoffsethot = 0;
    no_animate.soundidx = 0;
}

animate_t* animate_find(region_object_t *obj) {
    for (animate_t *rover = list; rover; rover = rover->next) {
        if (rover->obj == obj) { return rover; }
    }

    return NULL;
}

// Sometimes the animation needs to be updated for the render list
void shift_anim(animate_t *anim) {
    animate_t *prev;
    animate_t *next;
    const int y = (anim->obj->mapy + anim->obj->bmp_height + 0) / 16;
    const int z = anim->obj->mapz;
    int nz = 0, ny = 0, pz = 0, py = 0;
    if (anim->next) {
        nz = anim->next->obj->mapz;
        ny = anim->next->obj->mapy;
        ny += anim->next->obj->bmp_height;
        ny /= 16;
    }
    while (anim->next && (
            z > nz
            || (z == nz && y >= ny)
            )) {
        prev = anim->prev;
        next = anim->next;
        if (prev) {
            prev->next = next;
        }
        next->prev = prev;
        anim->prev = next;
        anim->next = next->next;
        next->next = anim;
        if (anim->next) {
            anim->next->prev = anim;
        }
        if (list == anim) {
            list = anim->prev;
        }
        if (anim->next) {
            nz = anim->next->obj->mapz;
            ny = anim->next->obj->mapy;
            ny += anim->next->obj->bmp_height;
            ny /= 16;
        }
    }

    if (anim->prev) {
        pz = anim->prev->obj->mapz;
        py = anim->prev->obj->mapy;
        py += anim->prev->obj->bmp_height;
        py /= 16;
    }
    while (anim->prev && (
            z < pz
            || (z == pz && y < py)
            )) {
        prev = anim->prev;
        next = anim->next;
        if (list == prev) {
            list = anim;
        }
        prev->next = next;
        if (next) {
            next->prev = prev;
        }
        anim->prev = prev->prev;
        anim->next = prev;
        prev->prev = anim;
        if (anim->prev) {
            anim->prev->next = anim;
        }
        if (anim->prev) {
            pz = anim->prev->obj->mapz;
            py = anim->prev->obj->mapy;
            py += anim->prev->obj->bmp_height;
            py /= 16;
        }
    }
}

animate_t* animate_add_obj(SDL_Renderer *renderer, region_object_t *obj, const int gff_file, const int palette_id) {
    //unsigned char *data = NULL;
    SDL_Rect loc;
    animate_t *toadd = malloc(sizeof(animate_t));
    scmd_t *cmd = obj->scmd;

    // add to the list.
    toadd->obj = obj;
    toadd->textures = NULL;
    toadd->len = toadd->ticks_left = toadd->pos = 0;
    toadd->next = list;
    toadd->prev = NULL;
    if (list) {
        list->prev = toadd;
    }
    list = toadd;

    if (!obj->scmd) {
        obj->scmd = &no_animate;
        cmd = obj->scmd;
    }
    toadd->ticks_left = obj->scmd->delay;
    while (!(cmd->flags & (SCMD_LAST | SCMD_JUMP))) {
        cmd++;
        toadd->len++;
    }
    cmd++;
    toadd->len++;

    // get all the textures
    toadd->textures = malloc(sizeof(SDL_Surface *) * (toadd->len));
    for (int i = 0; i < toadd->len; i++) {
        toadd->textures[i] = 
            create_texture(renderer, gff_file, GT_BMP, obj->btc_idx, obj->bmp_idx, palette_id, &loc);
        obj->bmp_width = loc.w;
        obj->bmp_height = loc.w;

        //printf("obj->bmp_height = %d\n", obj->bmp_height);
        //printf("obj->bmp_width = %d\n", obj->bmp_width);
        //printf("obj->mapx = %d\n", obj->mapx);
        //printf("obj->mapy = %d\n", obj->mapy);
    }

    shift_anim(toadd);

    return toadd;
}

animate_t* animate_add_objex(map_t *map, SDL_Renderer *renderer, region_object_t *obj) {
    unsigned char *data = NULL;
    SDL_Surface *surface;
    animate_t *toadd = malloc(sizeof(animate_t));
    scmd_t *cmd = obj->scmd;

    // add to the list.
    toadd->obj = obj;
    toadd->textures = NULL;
    toadd->len = toadd->ticks_left = toadd->pos = 0;
    toadd->next = list;
    toadd->prev = NULL;
    if (list) {
        list->prev = toadd;
    }
    list = toadd;

    if (!obj->scmd) {
        obj->scmd = &no_animate;
        cmd = obj->scmd;
    }
    toadd->ticks_left = obj->scmd->delay;
    while (!(cmd->flags & (SCMD_LAST | SCMD_JUMP))) {
        cmd++;
        toadd->len++;
    }
    cmd++;
    toadd->len++;


    // get all the textures
    toadd->textures = malloc(sizeof(SDL_Surface *) * (toadd->len));
    for (int i = 0; i < toadd->len; i++) {
        data = get_frame_rgba_with_palette(OBJEX_GFF_INDEX, GT_BMP, obj->btc_idx, i, -1);
        obj->bmp_width = get_frame_width(OBJEX_GFF_INDEX, GT_BMP, obj->btc_idx, i);
        obj->bmp_height = get_frame_height(OBJEX_GFF_INDEX, GT_BMP, obj->btc_idx, i);

        surface = SDL_CreateRGBSurfaceFrom(data, obj->bmp_width, obj->bmp_height, 32, 
                4*obj->bmp_width, 0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);
        toadd->textures[i] = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_FreeSurface(surface);
        free(data);
    }

    shift_anim(toadd);

    return toadd;
}

animate_t* animate_add(map_t *map, SDL_Renderer *renderer, region_object_t *obj) {
    unsigned char *data = NULL;
    SDL_Surface *surface;
    uint8_t z;
    animate_t *toadd = malloc(sizeof(animate_t));
    scmd_t *cmd = obj->scmd;

    // add to the list.
    toadd->obj = obj;
    toadd->textures = NULL;
    toadd->len = toadd->ticks_left = toadd->pos = 0;
    toadd->next = list;
    toadd->prev = NULL;
    if (list) {
        list->prev = toadd;
    }
    list = toadd;

    if (!obj->scmd) {
        obj->scmd = &no_animate;
        cmd = obj->scmd;
    }
    toadd->ticks_left = obj->scmd->delay;
    while (!(cmd->flags & (SCMD_LAST | SCMD_JUMP))) {
        cmd++;
        toadd->len++;
    }
    cmd++;
    toadd->len++;

    gff_map_get_object_location(map->region->gff_file, map->region->map_id, obj->entry_id,
            &obj->mapx, 
            &obj->mapy, 
            &z);
    obj->mapz = z; // TODO: is z unsigned or signed?

    // get all the textures
    toadd->textures = malloc(sizeof(SDL_Surface *) * (toadd->len));
    for (int i = 0; i < toadd->len; i++) {
        data = dsl_load_object_bmp(map->region, obj->entry_id, (obj->scmd+i)->bmp_idx);

        surface = SDL_CreateRGBSurfaceFrom(data, obj->bmp_width, obj->bmp_height, 32, 
                4*obj->bmp_width, 0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);
        toadd->textures[i] = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_FreeSurface(surface);
        free(data);
    }

    shift_anim(toadd);

    return toadd;
}

void animate_render(void *data, SDL_Renderer *renderer) {
    const int stretch = 2;
    const uint32_t xoffset = getCameraX();
    const uint32_t yoffset = getCameraY();
    SDL_Rect loc;

    for (animate_t *rover = list; rover; rover = rover->next) {
        //printf("rover[%d]\n", i++);
        if (rover->ticks_left > 0) {
            rover->ticks_left--;
            if (rover->ticks_left == 0) {
                rover->pos++;
                scmd_t *scmd = rover->obj->scmd + rover->pos;
                if (scmd->flags & SCMD_JUMP) {
                    scmd = rover->obj->scmd;
                    rover->pos = 0;
                }
                rover->ticks_left = scmd->delay;
                rover->ticks_left = rover->ticks_left == 0 ? 16 : rover->ticks_left;
            }
        }
        loc.w = rover->obj->bmp_width;
        loc.h = rover->obj->bmp_height;
        loc.x = rover->obj->mapx;
        loc.y = rover->obj->mapy;
        loc.x *= 2;
        loc.x -= xoffset;
        loc.y *= 2;
        loc.y -= yoffset;
        loc.w *= stretch;
        loc.h *= stretch;
        SDL_RenderCopy(renderer, rover->textures[rover->pos], NULL, &loc);
    }
}

void animate_clear() {
    animate_t *tmp;

    while (list) {
        tmp = list;
        list = list->next;
        for (int i = 0; i < tmp->len; i++) {
            SDL_DestroyTexture(tmp->textures[i]);
        }
        free(tmp->textures);
        free(tmp);
    }

    //memset(animations, 0x0, sizeof(animate_t*) * MAX_DELAY);

    list = NULL;
}

void animate_close() {
    animate_clear();
}
