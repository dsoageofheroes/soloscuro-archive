#include "view-character.h"
#include "../main.h"
#include "../../src/gff.h"
#include "../../src/gfftypes.h"
#include "narrate.h"
#include "popup.h"
#include "../font.h"
#include "../sprite.h"

// Sprites
static uint16_t ai[4];
static uint16_t leader[4];
static uint16_t ports[4];
static uint16_t effects, view_char, use, panel;
static uint16_t character, inv, magic, status;
static uint16_t game_menu, game_return;

static SDL_Rect initial_locs[] = {{ 155, 28, 0, 0 }, // description
                                  { 75, 175, 0, 0 }, // message
};

static SDL_Rect description_loc, message_loc;
static char description[25];
static char message[128];

static SDL_Renderer *rend;

static void set_zoom(SDL_Rect *loc, float zoom) {
    loc->x *= zoom;
    loc->y *= zoom;
    loc->w *= zoom;
    loc->h *= zoom;
}

static SDL_Rect apply_params(const SDL_Rect rect, const uint32_t x, const uint32_t y) {
    SDL_Rect ret = {rect.x + x, rect.y + y, rect.w, rect.h};
    return ret;
}

uint16_t view_sprite_create(SDL_Renderer *renderer, gff_palette_t *pal,
        const int offsetx, const int offsety, const float zoom,
        const int gff_idx, const int type_id, const int res_id) {
    SDL_Rect tmp = {offsetx, offsety, 0, 0};
    return sprite_create(renderer, &tmp, pal, 0, 0, zoom, gff_idx, type_id, res_id);
}

void view_character_init(SDL_Renderer *renderer, const uint32_t x, const uint32_t y, const float zoom) {
    gff_palette_t *pal = open_files[RESOURCE_GFF_INDEX].pals->palettes + 0;
    rend = renderer;

    memset(description, 0x0, sizeof(description));
    memset(message, 0x0, sizeof(message));

    panel = view_sprite_create(renderer, pal, 0 + x, 0 + y, zoom, RESOURCE_GFF_INDEX, GFF_BMP, 11000);
    effects = view_sprite_create(renderer, pal, 80 + x, 0 + y, zoom, RESOURCE_GFF_INDEX, GFF_BMP, 20075);
    view_char = view_sprite_create(renderer, pal, 60 + x, 0 + y, zoom, RESOURCE_GFF_INDEX, GFF_BMP, 20079);
    use = view_sprite_create(renderer, pal, 115 + x, 0 + y, zoom, RESOURCE_GFF_INDEX, GFF_BMP, 20080);

    character = view_sprite_create(renderer, pal, 45 + x, 155 + y, zoom, RESOURCE_GFF_INDEX, GFF_ICON, 10100);
    inv = view_sprite_create(renderer, pal, 70 + x, 155 + y, zoom, RESOURCE_GFF_INDEX, GFF_ICON, 11102);
    magic = view_sprite_create(renderer, pal, 95 + x, 155 + y, zoom, RESOURCE_GFF_INDEX, GFF_ICON, 11103);
    status = view_sprite_create(renderer, pal, 120 + x, 155 + y, zoom, RESOURCE_GFF_INDEX, GFF_ICON, 11104);
    game_return = view_sprite_create(renderer, pal, 252 + x, 155 + y, zoom, RESOURCE_GFF_INDEX, GFF_ICON, 10108);
    game_menu = view_sprite_create(renderer, pal, 222 + x, 155 + y, zoom, RESOURCE_GFF_INDEX, GFF_ICON, 11101);

    ai[0] = view_sprite_create(renderer, pal, 45 + x, 40 + y, zoom, RESOURCE_GFF_INDEX, GFF_ICON, 11111);
    ai[1] = view_sprite_create(renderer, pal, 45 + x, 100 + y, zoom, RESOURCE_GFF_INDEX, GFF_ICON, 11111);
    ai[2] = view_sprite_create(renderer, pal, 95 + x, 40 + y, zoom, RESOURCE_GFF_INDEX, GFF_ICON, 11111);
    ai[3] = view_sprite_create(renderer, pal, 95 + x, 100 + y, zoom, RESOURCE_GFF_INDEX, GFF_ICON, 11111);
    leader[0] = view_sprite_create(renderer, pal, 45 + x, 30 + y, zoom, RESOURCE_GFF_INDEX, GFF_ICON, 11106);
    leader[1] = view_sprite_create(renderer, pal, 45 + x, 90 + y, zoom, RESOURCE_GFF_INDEX, GFF_ICON, 11106);
    leader[2] = view_sprite_create(renderer, pal, 95 + x, 30 + y, zoom, RESOURCE_GFF_INDEX, GFF_ICON, 11106);
    leader[3] = view_sprite_create(renderer, pal, 95 + x, 90 + y, zoom, RESOURCE_GFF_INDEX, GFF_ICON, 11106);
    ports[0] = view_sprite_create(renderer, pal, 55 + x, 30 + y, zoom, RESOURCE_GFF_INDEX, GFF_ICON, 11100);
    ports[1] = view_sprite_create(renderer, pal, 55 + x, 90 + y, zoom, RESOURCE_GFF_INDEX, GFF_ICON, 11100);
    ports[2] = view_sprite_create(renderer, pal, 105 + x, 30 + y, zoom, RESOURCE_GFF_INDEX, GFF_ICON, 11100);
    ports[3] = view_sprite_create(renderer, pal, 105 + x, 90 + y, zoom, RESOURCE_GFF_INDEX, GFF_ICON, 11100);

    for (int i = 0; i < 4; i++) {
        sprite_set_frame(ports[i], 2);
    }

    description_loc = apply_params(initial_locs[0], x, y);
    message_loc = apply_params(initial_locs[1], x, y);

    set_zoom(&description_loc, zoom);
    set_zoom(&message_loc, zoom);

    strcpy(description, "description");
    strcpy(message, "message");
}

void view_character_render(void *data, SDL_Renderer *renderer) {
    sprite_render(renderer, panel);
    sprite_render(renderer, view_char);

    sprite_render(renderer, character);
    sprite_render(renderer, inv);
    sprite_render(renderer, magic);
    sprite_render(renderer, status);
    sprite_render(renderer, game_menu);
    sprite_render(renderer, game_return);

    for (int i = 0; i < 4; i++) {
        sprite_render(renderer, ai[i]);
        sprite_render(renderer, leader[i]);
        sprite_render(renderer, ports[i]);
    }

    print_line_len(renderer, FONT_YELLOW, description, description_loc.x, description_loc.y, sizeof(description));
    print_line_len(renderer, FONT_GREY, message, message_loc.x, message_loc.y, sizeof(message));
}

static int get_sprite_mouse_is_on(const uint32_t x, const uint32_t y) {
    if (sprite_in_rect(character, x, y)) { return character; }
    if (sprite_in_rect(inv, x, y)) { return inv; }
    if (sprite_in_rect(magic, x, y)) { return magic; }
    if (sprite_in_rect(status, x, y)) { return status; }
    if (sprite_in_rect(game_menu, x, y)) { return game_menu; }
    if (sprite_in_rect(game_return, x, y)) { return game_return; }
    //if (sprite_in_rect(effects, x, y)) { return effects; }

    for (int i = 0; i < 4; i++) {
        if (sprite_in_rect(ai[i], x, y)) { return ai[i]; }
        if (sprite_in_rect(leader[i], x, y)) { return leader[i]; }
        if (sprite_in_rect(ports[i], x, y)) { return ports[i]; }
    }

    return SPRITE_ERROR;
}

int view_character_handle_mouse_movement(const uint32_t x, const uint32_t y) {
    static uint16_t last_sprite = SPRITE_ERROR;

    uint16_t cur_sprite = get_sprite_mouse_is_on(x, y);

    if (last_sprite != cur_sprite) {
        sprite_set_frame(cur_sprite, sprite_get_frame(cur_sprite) + 1);
        if (last_sprite != SPRITE_ERROR) {
            sprite_set_frame(last_sprite, sprite_get_frame(last_sprite) - 1);
        }
    }
    
    last_sprite = cur_sprite;
    //character = view_sprite_create(renderer, pal, 45 + x, 155 + y, zoom, RESOURCE_GFF_INDEX, GFF_ICON, 10100);
    //inv = view_sprite_create(renderer, pal, 70 + x, 155 + y, zoom, RESOURCE_GFF_INDEX, GFF_ICON, 11102);
    //magic = view_sprite_create(renderer, pal, 95 + x, 155 + y, zoom, RESOURCE_GFF_INDEX, GFF_ICON, 11103);
    //status = view_sprite_create(renderer, pal, 120 + x, 155 + y, zoom, RESOURCE_GFF_INDEX, GFF_ICON, 11104);
    //game_return = view_sprite_create(renderer, pal, 252 + x, 155 + y, zoom, RESOURCE_GFF_INDEX, GFF_ICON, 10108);
    //game_menu = view_sprite_create(renderer, pal, 222 + x, 155 + y, zoom, RESOURCE_GFF_INDEX, GFF_ICON, 11101);
    return 1;// did not handle
}

int view_character_handle_mouse_down(const uint32_t button, const uint32_t x, const uint32_t y) {
    //return 1; // means I captured the mouse click
    return 0; // zero means I did not handle the mouse click, so another screen may.
}

int view_character_handle_mouse_up(const uint32_t button, const uint32_t x, const uint32_t y) {
    //return 1; // means I captured the mouse click
    if (button == SDL_BUTTON_RIGHT) {
        for (int i = 0; i < 4; i++) {
            if (sprite_in_rect(ports[i], x, y)) {
                screen_push_screen(rend, &popup_screen, 100, 75);
                popup_set_message("INACTIVE CHARACTER");
                popup_set_option(0, "NEW");
                popup_set_option(1, "ADD");
                popup_set_option(2, "CANCEL");
            }
        }
    }
    return 0; // zero means I did not handle the mouse click, so another screen may.
}

void view_character_free() {
    sprite_free(panel);
    sprite_free(use);
    sprite_free(view_char);
    sprite_free(effects);

    for (int i = 0; i < 4; i++) {
        sprite_free(ai[i]);
        sprite_free(leader[i]);
        sprite_free(ports[i]);
    }
}

void view_character_return_control () {
    printf("RETURN!\n");
}

sops_t view_character_screen = {
    .init = view_character_init,
    .cleanup = view_character_free,
    .render = view_character_render,
    .mouse_movement = view_character_handle_mouse_movement,
    .mouse_down = view_character_handle_mouse_down,
    .mouse_up = view_character_handle_mouse_up,
    .return_control = view_character_return_control,
    .data = NULL
};
