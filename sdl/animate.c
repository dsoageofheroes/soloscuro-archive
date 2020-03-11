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

void animate_add(map_t *map, SDL_Renderer *renderer, scmd_t *cmd, int id) {
    unsigned char *data = NULL;
    int32_t width, height;
    SDL_Surface *surface;
    uint16_t x, y;
    uint8_t z;
    uint32_t palette_id = gff_get_palette_id(DSLDATA_GFF_INDEX, map->map_id - 1);
    animate_t *toadd = malloc(sizeof(animate_t));

    memset(&(toadd->loc), 0x0, sizeof(SDL_Rect));
    // add to the list.
    toadd->scmd = cmd;
    toadd->textures = NULL;
    toadd->len = toadd->ticks_left = toadd->pos = 0;
    toadd->next = list;
    list = toadd;

    toadd->ticks_left = toadd->scmd->delay;
    while (!(cmd->flags & SCMD_LAST)) {
        cmd++;
        toadd->len++;
    }

    toadd->flags = gff_map_get_object_location(map->gff_file, map->map_id, id, &x, &y, &z);

    // get all the textures
    toadd->textures = malloc(sizeof(SDL_Surface *) * (toadd->len));
    for (int i = 0; i < toadd->len; i++) {
        data = gff_map_get_object_bmp_pal(map->gff_file, map->map_id, id, &width, &height,
            toadd->scmd->bmp_idx, palette_id);
        surface = SDL_CreateRGBSurfaceFrom(data, width, height, 32, 
                4*width, 0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);
        toadd->textures[i] = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_FreeSurface(surface);
        free(data);
        if (i == 0) { // Grab the stats for the first one.
            toadd->loc.w = width;
            toadd->loc.h = height;
            toadd->loc.x = x;
            toadd->loc.y = y;
        }
    }

    // delay of a zero means no animation.
    /*
    if (delay > 0) {
        // So add it to the correct animation list (for when to change.)
        size_t aloc = (animate_pos + delay) % MAX_DELAY;
        debug("adding animate_pos = %ld\n", aloc);
        animation_delay_t *anim_delay = malloc(sizeof(animation_delay_t));
        anim_delay->anim = toadd;
        anim_delay->prev = NULL;
        anim_delay->next = animations[aloc];
        animations[aloc] = anim_delay;
    }
    */
}

void animate_render(void *data, SDL_Renderer *renderer) {
    const int stretch = 2;
    const uint32_t xoffset = getCameraX();
    const uint32_t yoffset = getCameraY();
    SDL_Rect loc;

    for (animate_t *rover = list; rover; rover = rover->next) {
        if (rover->ticks_left > 0) {
            rover->ticks_left--;
            if (rover->ticks_left == 0) {
                rover->pos++;
                rover->ticks_left = (rover->scmd+rover->pos)->delay;
                debug("rover->pos = %d, rover->ticks_left = %d\n", rover->pos, rover->ticks_left);
            }
        }
        memcpy(&loc, &(rover->loc), sizeof(SDL_Rect));
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
