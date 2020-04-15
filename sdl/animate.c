#include <string.h>
#include <stdlib.h>
#include "animate.h"
#include "../src/gameloop.h"
#include "../src/dsl.h"
#include "../src/dsl-scmd.h"

//#define MAX_DELAY (256)
static animate_t *list;
//static size_t animate_pos = 0;

//typedef struct animation_delay_s {
    //animate_t *anim;
    //struct animation_delay_s *next, *prev;
//} animation_delay_t;

//static animation_delay_t *animations[MAX_DELAY];

void animate_init() {
    //memset(animations, 0x0, sizeof(animation_delay_t*) * MAX_DELAY);
    list = NULL;
}

// Sometimes the animation needs to be updated for the render list
static void shift_anim(animate_t *anim) {
    animate_t *prev;
    animate_t *next;
    while (anim->next && (
            anim->obj->mapz > anim->next->obj->mapz
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
    }
}

animate_t* animate_add(map_t *map, SDL_Renderer *renderer, dsl_object_t *obj) {
    unsigned char *data = NULL;
    int32_t width, height;
    SDL_Surface *surface;
    uint8_t z;
    animate_t *toadd = malloc(sizeof(animate_t));
    scmd_t *cmd = obj->scmd;

    // add to the list.
    //toadd->scmd = cmd;
    toadd->obj = obj;
    toadd->textures = NULL;
    toadd->len = toadd->ticks_left = toadd->pos = 0;
    toadd->next = list;
    toadd->prev = NULL;
    if (list) {
        list->prev = toadd;
    }
    list = toadd;

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
        data = gff_map_get_object_bmp_pal(map->region->gff_file, map->region->map_id, obj->entry_id, &width, &height,
            (obj->scmd + i)->bmp_idx, map->region->palette_id);
        obj->bmp_width = width;
        obj->bmp_height = height;
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

    int i = 0;
    for (animate_t *rover = list; rover; rover = rover->next) {
        printf("rover[%d]\n", i++);
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
