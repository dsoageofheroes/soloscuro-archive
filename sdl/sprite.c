/*
 * Actual implementation of a sprite (works with src/sprite.c)
 */
#include "sprite.h"
#include "main.h"
#include "entity.h"
#include "settings.h"
#include "port.h"

#define MAX_SPRITES (1<<10)

static sprite_t sprites[MAX_SPRITES];
static int16_t sprite_pos = -1;

static sol_sprite_t sprite_append_full(uint16_t sprite_id, SDL_Renderer *renderer, SDL_Rect *initial,
        gff_palette_t *pal,
        const int offsetx, const int offsety, const float zoom,
        const int gff_idx, const int type_id, const int res_id);

extern sol_status_t sol_sprite_init() {
    // init is called on load, only init once.
    if (sprite_pos < 0) {
        memset(sprites, 0x0, sizeof(sprite_t) * MAX_SPRITES);
        sprite_pos = 0;
    }

    return SOL_SUCCESS;
}

static void set_zoom(SDL_Rect *loc, const float zoom) {
    loc->x *= zoom;
    loc->y *= zoom;
    loc->w *= zoom;
    loc->h *= zoom;
}

static int valid_id(const uint16_t id) {
    return (id < MAX_SPRITES && sprites[id].in_use);
}

extern sol_sprite_t sol_sprite_print(const sol_sprite_t sprite_id) {
    if (!valid_id(sprite_id)) { return SOL_SPRITE_NOT_FOUND; }
    sprite_t *sprite = sprites + sprite_id;
    printf("%d: frame %d of %d @(%d, %d), %dx%x texture: %p, in_use? %d\n",
        sprite_id, sprite->pos, sprite->len,
        sprite->loc[sprite->pos].x, sprite->loc[sprite->pos].y,
        sprite->loc[sprite->pos].w, sprite->loc[sprite->pos].h,
        sprite->tex[sprite->pos], sprite->in_use);
    return SOL_SUCCESS;
}

static SDL_Rect apply_params(const SDL_Rect rect, const uint32_t x, const uint32_t y) {
    SDL_Rect ret = {rect.x + x, rect.y + y, rect.w, rect.h};
    return ret;
}

static uint16_t get_next_sprite_id() {
    if (sprite_pos >= MAX_SPRITES) {
        sprite_pos = 0;
    }

    if (sprite_pos < 0) { return SPRITE_ERROR; }

    if (!sprites[sprite_pos].in_use) {
        return sprite_pos++;
    }

    for (int i = 0; i < MAX_SPRITES; i++) {
        if (!sprites[i].in_use) {
            sprite_pos = i;
            return sprite_pos++;
        }
    }


    return SPRITE_ERROR;
}

static SDL_Texture* create_texture(SDL_Renderer *renderer, const uint32_t gff_idx,
        const uint32_t type_id, const uint32_t res_id, const uint32_t frame_id,
        const gff_palette_t *pal, SDL_Rect *loc) {
    unsigned char *data;
    SDL_Surface *surface = NULL;
    SDL_Texture *ret = NULL;
    loc->w = gff_get_frame_width(gff_idx, type_id, res_id, frame_id);
    loc->h = gff_get_frame_height(gff_idx, type_id, res_id, frame_id);
    //printf("loc.w/h = (%d, %d), from %d, %d, %d, %d\n", loc->w, loc->h, gff_idx, type_id, res_id, frame_id);
    //data = get_frame_rgba_with_palette(gff_file, type, id, frame_id, palette_id);
    data = gff_get_frame_rgba_palette(gff_idx, type_id, res_id, frame_id, pal);
    surface = SDL_CreateRGBSurfaceFrom(data, loc->w, loc->h, 32, 4*loc->w,
            0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);
    ret = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    free(data);
    return ret;
}

// Create a sprite and return its ID
static sol_status_t sprite_create(SDL_Renderer *renderer, SDL_Rect *initial,
        gff_palette_t *pal,
        const int offsetx, const int offsety, const float zoom,
        const int gff_idx, const int type_id, const int res_id,
        sol_sprite_t *spr) {
    sprite_t *sprite;
    if (!renderer) { return SOL_SPRITE_NOT_INITIALIZED; }

    *spr = get_next_sprite_id();
    if (*spr == SPRITE_ERROR) { return SOL_SPRITE_MAX_LIMIT; }
    sprite = sprites + *spr;
    memset(sprite, 0x0, sizeof(sprite_t));

    sprite_append_full(*spr, renderer, initial, pal,
        offsetx, offsety, zoom, gff_idx, type_id, res_id);

    sprite->pos = 0;
    sprite->in_use = 1;
    //printf("sprite[%d]: %d frames, tex[0] = %p\n", sprite_id, sprite->len, sprite->tex[0]);

    return SOL_SUCCESS;
}

static sol_sprite_t sprite_new(SDL_Renderer *renderer, gff_palette_t *pal,
        const int offsetx, const int offsety, const float zoom,
        const int gff_idx, const int type_id, const int res_id,
        sol_sprite_t *spr) {
    SDL_Rect tmp = {offsetx, offsety, 0, 0};
    return sprite_create(renderer, &tmp, pal, 0, 0, zoom, gff_idx, type_id, res_id, spr);
}

extern sol_status_t sprite_append(uint16_t sprite_id, SDL_Renderer *renderer,
        gff_palette_t *pal,
        const int offsetx, const int offsety, const float zoom,
        const int gff_idx, const int type_id, const int res_id) {
    SDL_Rect tmp = {offsetx, offsety, 0, 0};
    if (!renderer) { return SOL_SPRITE_NOT_INITIALIZED; }
    return sprite_append_full(sprite_id, renderer, &tmp, pal, 0, 0, zoom, gff_idx, type_id, res_id);
}

// Create a sprite and return its ID
static sol_status_t sprite_append_full(uint16_t sprite_id, SDL_Renderer *renderer, SDL_Rect *initial,
        gff_palette_t *pal,
        const int offsetx, const int offsety, const float zoom,
        const int gff_idx, const int type_id, const int res_id) {

    size_t append_start = 0;
    if (sprite_id == SPRITE_ERROR) { return SOL_SPRITE_NOT_FOUND; }
    sprite_t *sprite = sprites + sprite_id;

    append_start = sprite->len;
    sprite->len += gff_get_frame_count(gff_idx, type_id, res_id);

    if (sprite->len == 0) { sprite->len = 1;}

    sprite->loc = (SDL_Rect*) realloc(sprite->loc, sizeof(SDL_Rect) * sprite->len);
    for (int i = append_start; i < sprite->len; i++) {
        *(sprite->loc + i) = apply_params(*initial, offsetx, offsetx);
    }

    // TODO: PERFORMANCE, create 1 large texture of multiple frames, which also eliminates a malloc...
    // TODO: alternatively, create a function to manually load the frames at once,
    //       instead of reading each frame from disk one at a time.
    sprite->tex = (SDL_Texture**)realloc(sprite->tex, sizeof(SDL_Texture*) * sprite->len);
    for (int i = append_start; i < sprite->len; i++) {
        sprite->tex[i] = create_texture(renderer, gff_idx, type_id, res_id, i - append_start, pal, (sprite->loc + i));
    }

    for (int i = append_start; i < sprite->len; i++) {
        set_zoom((sprite->loc + i), zoom);
    }

    return SOL_SUCCESS;
}

uint32_t sprite_valid(const uint16_t id) { return valid_id(id); }

extern sol_status_t sol_sprite_set_frame_keep_loc(const sol_sprite_t id, const uint16_t frame) {
    if (!valid_id(id))            { return SOL_SPRITE_NOT_FOUND; }
    if (frame >= sprites[id].len) { return SOL_OUT_OF_RANGE; }
    int lpos = sprites[id].pos;

    sprites[id].pos = frame;
    sprites[id].loc[sprites[id].pos].x = sprites[id].loc[lpos].x;
    sprites[id].loc[sprites[id].pos].y = sprites[id].loc[lpos].y;

    return SOL_SUCCESS;
}

extern sol_status_t sol_sprite_set_frame(const sol_sprite_t id, const uint16_t frame) {
    if (!valid_id(id))            { return SOL_SPRITE_NOT_FOUND; }
    if (frame >= sprites[id].len) { return SOL_OUT_OF_RANGE; }
    sprites[id].pos = frame;

    return SOL_SUCCESS;
}

extern sol_status_t sol_sprite_increment_frame(const sol_sprite_t id, const int16_t amt) {
    if (!valid_id(id))            { return SOL_SPRITE_NOT_FOUND; }
    if (sprites[id].pos + amt >= sprites[id].len) { return SOL_OUT_OF_RANGE; }
    sprites[id].pos += amt;

    return SOL_SUCCESS;
}

extern sol_status_t sol_sprite_set_alpha(const sol_sprite_t id, const uint8_t alpha) {
    if (!valid_id(id)) { return SOL_SPRITE_NOT_FOUND; }

    for (uint32_t i = 0; i < sprites[id].len; i++) {
        SDL_SetTextureAlphaMod(sprites[id].tex[i], alpha);
    }

    return SOL_SUCCESS;
}

static sol_sprite_t sprite_render_box(SDL_Renderer *renderer, const uint16_t sprite_id,
        const uint16_t x, const uint16_t y, const uint16_t w, const uint16_t h) {
    if (sprite_id == (uint16_t)SPRITE_ERROR) { return SOL_SPRITE_NOT_FOUND; }
    sprite_t *sprite = sprites + sprite_id;
    SDL_Rect dest = *(sprites[sprite_id].loc);
    dest.x = x;
    dest.y = y;
    dest.w = w;
    dest.h = h;

    SDL_Rect src = *(sprites[sprite_id].loc);
    src.x = src.y = 0;
    src.w = w;
    src.h = w / settings_zoom();
    src.h = h / settings_zoom();

    SDL_RenderCopy(renderer, sprite->tex[sprite->pos], &src, &dest); //(sprite->loc + sprite->pos));
    return SOL_SUCCESS;
}

extern sol_status_t sprite_render(SDL_Renderer *renderer, const uint16_t sprite_id) {
    if (sprite_id == (uint16_t)SPRITE_ERROR) { return SOL_SPRITE_NOT_FOUND; }
    sprite_t *sprite = sprites + sprite_id;
    /*
    printf("sprite_render: %d(%p)\n", sprite_id, sprite);
    printf("sprite_Render: text = %p\n", sprite->tex);
    printf("->%p [%d] {%d, %d, %d, %d}\n", sprite->tex[sprite->pos], sprite->pos,
            sprite->loc[sprite->pos].x,
            sprite->loc[sprite->pos].y,
            sprite->loc[sprite->pos].w,
            sprite->loc[sprite->pos].h);
            */
    SDL_RenderCopy(renderer, sprite->tex[sprite->pos], NULL, (sprite->loc + sprite->pos));
    return SOL_SUCCESS;
}

extern sol_status_t sol_sprite_render_flip(const uint16_t sprite_id, const int horizontal_flip, const int vertical_flip) {
    if (sprite_id == (uint16_t)SPRITE_ERROR) { return SOL_SPRITE_NOT_FOUND; }
    SDL_RendererFlip flip = (horizontal_flip) ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;
    flip |= (vertical_flip) ? SDL_FLIP_VERTICAL : SDL_FLIP_NONE;
    sprite_t *sprite = sprites + sprite_id;
    //SDL_SetTextureColorMod(sprite->tex[sprite->pos], 255, 255, 255);
    SDL_RenderCopyEx(main_get_rend(), sprite->tex[sprite->pos], NULL, (sprite->loc + sprite->pos), 0, NULL, flip);
    return SOL_SUCCESS;
}

extern sol_status_t sol_sprite_set_color_mod(const uint16_t sprite_id, const uint8_t r, const uint8_t g, const uint8_t b) {
    if (sprite_id == (uint16_t)SPRITE_ERROR) { return SOL_SPRITE_NOT_FOUND; }
    sprite_t *sprite = sprites + sprite_id;
    SDL_SetTextureColorMod(sprite->tex[sprite->pos], r, g, b);
    return SOL_SUCCESS;
}

void sprite_render_flip(SDL_Renderer *renderer, const uint16_t sprite_id, SDL_RendererFlip flip) {
    if (sprite_id == (uint16_t)SPRITE_ERROR) { return; }
    sprite_t *sprite = sprites + sprite_id;
    /*
    printf("->%p {%d, %d, %d, %d} len = %d\n", sprite->tex[sprite->pos],
            sprite->loc[sprite->pos].x,
            sprite->loc[sprite->pos].y,
            sprite->loc[sprite->pos].w,
            sprite->loc[sprite->pos].h,
            sprite->len);
            */
    SDL_RenderCopyEx(renderer, sprite->tex[sprite->pos], NULL, (sprite->loc + sprite->pos), 0, NULL, flip);
}

extern sol_sprite_t sprite_set_location(const uint16_t sprite_id, const uint32_t x, const uint32_t y) {
    if (sprite_id == (uint16_t)SPRITE_ERROR) { return SOL_SPRITE_NOT_FOUND; }
    int pos = sprites[sprite_id].pos;
    //printf("->%d\n", sprites[sprite_id].in_use);
    sprites[sprite_id].loc[pos].x = x;
    sprites[sprite_id].loc[pos].y = y;
    return SOL_SUCCESS;
}

extern sol_sprite_t sol_sprite_in_rect(const uint16_t id, const uint32_t x, const uint32_t y) {
    if (!valid_id(id)) { return SOL_SPRITE_NOT_FOUND; }

    SDL_Rect *loc = (sprites[id].loc + sprites[id].pos);
    return (x >= loc->x && x < (loc->x + loc->w)
        && y >= loc->y && y < (loc->y + loc->h)) ? SOL_SUCCESS: SOL_SPRITE_NOT_IN_RECT;
}

extern sol_status_t sol_sprite_get_info(const sol_sprite_t id, sol_sprite_info_t *info) {
    if (!info)         { return SOL_NULL_ARGUMENT; }
    if (!valid_id(id)) { return SOL_SPRITE_NOT_FOUND; }

    SDL_Rect *rect = (sprites[id].loc + sprites[id].pos);
    info->x = rect->x;
    info->y = rect->y;
    info->w = rect->w;
    info->h = rect->h;
    info->current_frame = sprites[id].pos;
    info->num_frames = sprites[id].len;

    return SOL_SUCCESS;
}

extern sol_status_t sol_sprite_center(const int id, const int x, const int y, const int w, const int h) {
    if (!valid_id(id)) { return SOL_SPRITE_NOT_FOUND; }
    SDL_Rect *loc = (sprites[id].loc + sprites[id].pos);

    int diffx = (w - loc->w) / 2;
    int diffy = (h - loc->h) / 2;

    loc->x = x + diffx;
    loc->y = y + diffy;

    return SOL_SUCCESS;
}

extern sol_status_t sol_sprite_center_spr(const int dest, const int src) {
    return sol_sprite_center(dest,
            sprites[src].loc[sprites[src].pos].x,
            sprites[src].loc[sprites[src].pos].y,
            sprites[src].loc[sprites[src].pos].w,
            sprites[src].loc[sprites[src].pos].h);
}

// Needed which our scmds are xmirrored. Used to calculate offset.
uint32_t sprite_get_xdiff_from_start(const uint16_t id) {
    if (!valid_id(id)) { return 0; }
    SDL_Rect *oloc = (sprites[id].loc);
    SDL_Rect *cloc = (sprites[id].loc + sprites[id].pos);

    return cloc->w - oloc->w;
}

// Free a sprite at an ID (do not use it again!)
extern sol_status_t sol_sprite_free(const sol_sprite_t id) {
    if (!valid_id(id)) { return SOL_SPRITE_NOT_FOUND; }

    if (sprites[id].in_use) {
        for (int i = 0; i < sprites[id].len; i++) {
            SDL_DestroyTexture(sprites[id].tex[i]);
        }
        sprites[id].len = 0;
        free(sprites[id].loc);
        sprites[id].loc = NULL;
        free(sprites[id].tex);
        sprites[id].tex = NULL;
        sprites[id].in_use = 0;
    }

    return SOL_SUCCESS;
}

extern void port_entity_update_scmd(entity_t *entity) {
    sol_sprite_set_frame(entity->anim.spr, entity->anim.scmd[entity->anim.pos].bmp_idx);
}

extern sol_status_t sol_sprite_create_from_data(unsigned char *data, const uint32_t w, const uint32_t h, sol_sprite_t *spr) {
    int sprite_id = get_next_sprite_id();
    if (sprite_id == SPRITE_ERROR) {
        *spr = SPRITE_ERROR;
        return SOL_SPRITE_MAX_LIMIT;
    }
    sprite_t *sprite = sprites + sprite_id;
    SDL_Surface *surface = NULL;

    surface = SDL_CreateRGBSurfaceFrom(data, w, h, 32,
            4*w, 0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);
    sprite->len = 1;
    sprite->pos = 0;
    sprite->loc = (SDL_Rect*) malloc(sizeof(SDL_Rect));
    sprite->loc->w = w * settings_zoom();
    sprite->loc->h = h * settings_zoom();
    sprite->loc->x = sprite->loc->y = 0;
    sprite->tex = (SDL_Texture**)realloc(sprite->tex, sizeof(SDL_Texture*) * sprite->len);
    if (sprite->tex == NULL) {
        return SOL_MEMORY_ERROR;
    }

    sprite->tex[0] = SDL_CreateTextureFromSurface(main_get_rend(), surface);
    sprite->in_use = 1;
    SDL_FreeSurface(surface);

    *spr = sprite_id;
    return SOL_SUCCESS;
}

extern sol_status_t sol_sprite_set_location(const sol_sprite_t id, const uint32_t x, const uint32_t y) {
    return sprite_set_location(id, x, y);
}

extern sol_status_t sol_sprite_render(const sol_sprite_t id) {
    return sprite_render(main_get_rend(), id);
}

extern sol_status_t sol_sprite_new(gff_palette_t *pal,
        const int offsetx, const int offsety, const float zoom,
        const int gff_idx, const int type_id, const int res_id,
        sol_sprite_t *spr) {
    return sprite_new(main_get_rend(), pal, offsetx, offsety, zoom, gff_idx, type_id, res_id, spr);
}

extern sol_status_t sol_sprite_append(sol_sprite_t sprite_id, gff_palette_t *pal,
        const int offsetx, const int offsety, const float zoom,
        const int gff_idx, const int type_id, const int res_id) {
    return sprite_append(sprite_id, main_get_rend(), pal, offsetx, offsety, zoom, gff_idx, type_id, res_id);
}

extern sol_sprite_t sol_sprite_render_box(const uint16_t sprite_id, const uint16_t x,
    const uint16_t y, const uint16_t w, const uint16_t h) { return sprite_render_box(main_get_rend(), sprite_id, x, y, w, h); }
