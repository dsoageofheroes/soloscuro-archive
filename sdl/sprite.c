#include "sprite.h"

#define MAX_SPRITES (1<<10)

static sprite_t sprites[MAX_SPRITES];
static uint16_t sprite_pos;

static uint16_t sprite_append_full(uint16_t sprite_id, SDL_Renderer *renderer, SDL_Rect *initial,
        gff_palette_t *pal,
        const int offsetx, const int offsety, const float zoom,
        const int gff_idx, const int type_id, const int res_id);

void sprite_init() {
    memset(sprites, 0x0, sizeof(sprite_t) * MAX_SPRITES);
    sprite_pos = 0;
}

static void set_zoom(SDL_Rect *loc, const float zoom) {
    loc->x *= zoom;
    loc->y *= zoom;
    loc->w *= zoom;
    loc->h *= zoom;
}

static SDL_Rect apply_params(const SDL_Rect rect, const uint32_t x, const uint32_t y) {
    SDL_Rect ret = {rect.x + x, rect.y + y, rect.w, rect.h};
    return ret;
}

static uint16_t get_next_sprite_id() {
    if (sprite_pos >= MAX_SPRITES) {
        sprite_pos = 0;
    }

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
    loc->w = get_frame_width(gff_idx, type_id, res_id, frame_id);
    loc->h = get_frame_height(gff_idx, type_id, res_id, frame_id);
    //data = get_frame_rgba_with_palette(gff_file, type, id, frame_id, palette_id);
    data = get_frame_rgba_palette(gff_idx, type_id, res_id, frame_id, pal);
    surface = SDL_CreateRGBSurfaceFrom(data, loc->w, loc->h, 32, 4*loc->w,
            0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);
    ret = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    free(data);
    return ret;
}

uint16_t sprite_new(SDL_Renderer *renderer, gff_palette_t *pal,
        const int offsetx, const int offsety, const float zoom,
        const int gff_idx, const int type_id, const int res_id) {
    SDL_Rect tmp = {offsetx, offsety, 0, 0};
    return sprite_create(renderer, &tmp, pal, 0, 0, zoom, gff_idx, type_id, res_id);
}

// Create a sprite and return its ID
uint16_t sprite_create(SDL_Renderer *renderer, SDL_Rect *initial,
        gff_palette_t *pal,
        const int offsetx, const int offsety, const float zoom,
        const int gff_idx, const int type_id, const int res_id) {

    sprite_t *sprite;
    int sprite_id = get_next_sprite_id();
    if (sprite_id == SPRITE_ERROR) { return sprite_id; }

    sprite_append_full(sprite_id, renderer, initial, pal,
        offsetx, offsety, zoom, gff_idx, type_id, res_id);

    sprite = sprites + sprite_id;
    sprite->pos = 0;
    sprite->in_use = 1;

    return sprite_id;
}

uint16_t sprite_append(uint16_t sprite_id, SDL_Renderer *renderer,
        gff_palette_t *pal,
        const int offsetx, const int offsety, const float zoom,
        const int gff_idx, const int type_id, const int res_id) {
    SDL_Rect tmp = {offsetx, offsety, 0, 0};
    return sprite_append_full(sprite_id, renderer, &tmp, pal, 0, 0, zoom, gff_idx, type_id, res_id);
}

// Create a sprite and return its ID
static uint16_t sprite_append_full(uint16_t sprite_id, SDL_Renderer *renderer, SDL_Rect *initial,
        gff_palette_t *pal,
        const int offsetx, const int offsety, const float zoom,
        const int gff_idx, const int type_id, const int res_id) {

    size_t append_start = 0;
    if (sprite_id == SPRITE_ERROR) { return sprite_id; }
    sprite_t *sprite = sprites + sprite_id;

    //printf("%d, %d, %d\n", gff_idx, type_id, res_id);
    append_start = sprite->len;
    sprite->len += get_frame_count(gff_idx, type_id, res_id);
    sprite->loc = realloc(sprite->loc, sizeof(SDL_Rect) * sprite->len);
    for (int i = append_start; i < sprite->len; i++) {
        *(sprite->loc + i) = apply_params(*initial, offsetx, offsetx);
    }
    //printf("frame out: %d\n", sprite->len);
    if (sprite->len == 0) { sprite->len = 1;}
    // TODO: PERFORMANCE, create 1 large texture of multiple frames, which also eliminates a malloc...
    // TODO: alternatively, create a function to manually load the frames at once,
    //       instead of reading each frame from disk one at a time.
    sprite->tex = realloc(sprite->tex, sizeof(SDL_Texture*) * sprite->len);
    for (int i = append_start; i < sprite->len; i++) {
        sprite->tex[i] = create_texture(renderer, gff_idx, type_id, res_id, i - append_start, pal, (sprite->loc + i));
        //printf("->{%d, %d, %d, %d}\n", sprite->loc.x, sprite->loc.y, sprite->loc.w, sprite->loc.h);
    }

    for (int i = append_start; i < sprite->len; i++) {
        set_zoom((sprite->loc + i), zoom);
    }

    return sprite_id;
}

static int valid_id(const uint16_t id) {
    return (id < MAX_SPRITES && sprites[id].in_use);
}

uint16_t sprite_get_frame(const uint16_t id) {
    if (!valid_id(id)) { return SPRITE_ERROR; }
    return sprites[id].pos;
}

void sprite_set_frame(const uint16_t id, const uint16_t frame) {
    if (!valid_id(id) || frame >= sprites[id].len) { return; }
    sprites[id].pos = frame;
}

void sprite_render(SDL_Renderer *renderer, const uint16_t sprite_id) {
    if (sprite_id == (uint16_t)SPRITE_ERROR) { return; }
    sprite_t *sprite = sprites + sprite_id;
    /*
    printf("->%p {%d, %d, %d, %d}\n", sprite->tex[sprite->pos],
            sprite->loc.x,
            sprite->loc.y,
            sprite->loc.w,
            sprite->loc.h);
            */
    SDL_RenderCopy(renderer, sprite->tex[sprite->pos], NULL, (sprite->loc + sprite->pos));
}

void sprite_render_flip(SDL_Renderer *renderer, const uint16_t sprite_id, SDL_RendererFlip flip) {
    if (sprite_id == (uint16_t)SPRITE_ERROR) { return; }
    sprite_t *sprite = sprites + sprite_id;
    /*
    printf("->%p {%d, %d, %d, %d}\n", sprite->tex[sprite->pos],
            sprite->loc.x,
            sprite->loc.y,
            sprite->loc.w,
            sprite->loc.h);
            */
    SDL_RenderCopyEx(renderer, sprite->tex[sprite->pos], NULL, (sprite->loc + sprite->pos), 0, NULL, flip);
}

void sprite_set_location(const uint16_t sprite_id, const uint32_t x, const uint32_t y) {
    if (sprite_id == (uint16_t)SPRITE_ERROR) { return; }
    int pos = sprites[sprite_id].pos;
    sprites[sprite_id].loc[pos].x = x;
    sprites[sprite_id].loc[pos].y = y;
}

int sprite_in_rect(const uint16_t id, const uint32_t x, const uint32_t y) {
    if (!valid_id(id)) { return 0; }
    SDL_Rect *loc = (sprites[id].loc + sprites[id].pos);
    return (x >= loc->x && x < (loc->x + loc->w)
        && y >= loc->y && y < (loc->y + loc->h));
}

uint32_t sprite_getx(const uint16_t id) {
    if (!valid_id(id)) { return 0; }
    SDL_Rect *loc = (sprites[id].loc + sprites[id].pos);
    return loc->x;
}

uint32_t sprite_gety(const uint16_t id) {
    if (!valid_id(id)) { return 0; }
    SDL_Rect *loc = (sprites[id].loc + sprites[id].pos);
    return loc->y;
}

uint32_t sprite_geth(const uint16_t id) {
    if (!valid_id(id)) { return 0; }
    SDL_Rect *loc = (sprites[id].loc + sprites[id].pos);
    return loc->h;
}

uint32_t sprite_getw(const uint16_t id) {
    if (!valid_id(id)) { return 0; }
    SDL_Rect *loc = (sprites[id].loc + sprites[id].pos);
    return loc->w;
}

uint32_t sprite_num_frames(const uint16_t id) {
    if (!valid_id(id)) { return 0; }
    return sprites[id].len;
}

void sprite_center(const int id, const int x, const int y, const int w, const int h) {
    if (!valid_id(id)) { return; }
    SDL_Rect *loc = (sprites[id].loc + sprites[id].pos);

    int diffx = (w - loc->w) / 2;
    int diffy = (h - loc->h) / 2;

    loc->x = x + diffx;
    loc->y = y + diffy;
}

void sprite_center_spr(const int dest, const int src) {
    if (!valid_id(src)) { return; }
    sprite_center(dest,
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
void sprite_free(const uint16_t id) {
    if (!valid_id(id)) { return; }
    if (sprites[id].in_use) {
        for (int i = 0; i < sprites[id].len; i++) {
            SDL_DestroyTexture(sprites[id].tex[i]);
        }
        free(sprites[id].loc);
        free(sprites[id].tex);
        sprites[id].in_use = 0;
    }
}
