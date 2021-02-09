#include "view-character.h"
#include "../main.h"
#include "../../src/gff.h"
#include "../../src/gff-char.h"
#include "../../src/gfftypes.h"
#include "../../src/rules.h"
#include "narrate.h"
#include "new-character.h"
#include "popup.h"
#include "add-load-save.h"
#include "../../src/ds-player.h"
#include "../../src/ds-load-save.h"
#include "../font.h"
#include "../sprite.h"

// Sprites
static uint16_t ai[4];
static uint16_t leader[4];
static uint16_t ports[4];
static uint16_t flag_spear, parchment, frame_message, panel, slots;
static uint16_t character, inv, magic, status;
static uint16_t game_menu, game_return;
enum {SELECT_NONE, SELECT_POPUP, SELECT_NEW, SELECT_ALS};
static int8_t last_selection = SELECT_NONE;
static float zoom = 1.0;
static uint8_t slot_clicked;

static SDL_Rect initial_locs[] = {{ 59, 5, 0, 0 }, // name
                                  { 57, 165, 0, 0 }, // description
};
static SDL_Rect slot_locs[] = {
    {0, 0, 0, 0},
    {0, 0, 0, 0},
    {0, 0, 0, 0},
    {0, 0, 0, 0},
    {0, 0, 0, 0},
    {0, 0, 0, 0},
    {0, 0, 0, 0},
    {0, 0, 0, 0},
    {0, 0, 0, 0},
    {57, 62, 0, 0}, // arm
    {64, 18, 0, 0}, // ammo
    {57, 42, 0, 0}, // missle
    {57, 82, 0, 0}, // hand0
    {57, 102, 0, 0}, // finger0
    {57, 122, 0, 0}, // waist
    {165, 122, 0, 0}, // legs
    {111, 18, 0, 0}, // head
    {158, 18, 0, 0}, // neck
    {165, 62, 0, 0}, // torso
    {165, 82, 0, 0}, // hand1
    {165, 102, 0, 0}, // finger1
    {165, 42, 0, 0}, // cloak
    {165, 142, 0, 0}, // foot
};

static SDL_Rect name_loc, description_loc;
static char name[25];
static char description[128];

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

static uint16_t view_sprite_create(SDL_Renderer *renderer, gff_palette_t *pal,
        const int offsetx, const int offsety, const float zoom,
        const int gff_idx, const int type_id, const int res_id) {
    SDL_Rect tmp = {offsetx, offsety, 0, 0};
    return sprite_create(renderer, &tmp, pal, 0, 0, zoom, gff_idx, type_id, res_id);
}

void inventory_screen_init(SDL_Renderer *renderer, const uint32_t x, const uint32_t y, const float _zoom) {
    gff_palette_t *pal = open_files[RESOURCE_GFF_INDEX].pals->palettes + 0;
    rend = renderer;
    zoom = _zoom;

    memset(name, 0x0, sizeof(name));
    memset(description, 0x0, sizeof(description));

    panel = view_sprite_create(renderer, pal, 0 + x, 0 + y, zoom, RESOURCE_GFF_INDEX, GFF_BMP, 13001);
    flag_spear = view_sprite_create(renderer, pal, 232 + x, 5 + y, zoom, RESOURCE_GFF_INDEX, GFF_BMP, 13004);
    parchment = view_sprite_create(renderer, pal, 75 + x, 36 + y, zoom, RESOURCE_GFF_INDEX, GFF_BMP, 13005);
    frame_message = view_sprite_create(renderer, pal, 53 + x, 3 + y, zoom, RESOURCE_GFF_INDEX, GFF_BMP, 13000);
    sprite_set_frame(frame_message, 0);
    sprite_set_location(frame_message, (53 + x) * zoom, (183 + y) * zoom);
    sprite_set_frame(frame_message, 3);
    sprite_set_location(frame_message, (45 + x) * zoom, (161 + y) * zoom);
    slots = view_sprite_create(renderer, pal, 53 + x, 3 + y, zoom, RESOURCE_GFF_INDEX, GFF_BMP, 13007);
    for (int i = 0; i < 23; i++) {
        sprite_set_frame(slots, i);
        sprite_set_location(slots, slot_locs[i].x * zoom, slot_locs[i].y * zoom);
    }

    character = view_sprite_create(renderer, pal, 160 + x, 181 + y, zoom, RESOURCE_GFF_INDEX, GFF_ICON, 10100);
    inv = view_sprite_create(renderer, pal, 185 + x, 181 + y, zoom, RESOURCE_GFF_INDEX, GFF_ICON, 11102);
    magic = view_sprite_create(renderer, pal, 210 + x, 181 + y, zoom, RESOURCE_GFF_INDEX, GFF_ICON, 11103);
    status = view_sprite_create(renderer, pal, 235 + x, 181 + y, zoom, RESOURCE_GFF_INDEX, GFF_ICON, 11104);
    game_return = view_sprite_create(renderer, pal, 288 + x, 181 + y, zoom, RESOURCE_GFF_INDEX, GFF_ICON, 10108);
    game_menu = view_sprite_create(renderer, pal, 258 + x, 181 + y, zoom, RESOURCE_GFF_INDEX, GFF_ICON, 11101);

    ai[0] = view_sprite_create(renderer, pal, 2 + x, 14 + y, zoom, RESOURCE_GFF_INDEX, GFF_ICON, 11111);
    ai[1] = view_sprite_create(renderer, pal, 2 + x, 62 + y, zoom, RESOURCE_GFF_INDEX, GFF_ICON, 11111);
    ai[2] = view_sprite_create(renderer, pal, 2 + x, 110 + y, zoom, RESOURCE_GFF_INDEX, GFF_ICON, 11111);
    ai[3] = view_sprite_create(renderer, pal, 2 + x, 158 + y, zoom, RESOURCE_GFF_INDEX, GFF_ICON, 11111);
    leader[0] = view_sprite_create(renderer, pal, 2 + x, 4 + y, zoom, RESOURCE_GFF_INDEX, GFF_ICON, 11106);
    leader[1] = view_sprite_create(renderer, pal, 2 + x, 52 + y, zoom, RESOURCE_GFF_INDEX, GFF_ICON, 11106);
    leader[2] = view_sprite_create(renderer, pal, 2 + x, 100 + y, zoom, RESOURCE_GFF_INDEX, GFF_ICON, 11106);
    leader[3] = view_sprite_create(renderer, pal, 2 + x, 148 + y, zoom, RESOURCE_GFF_INDEX, GFF_ICON, 11106);
    ports[0] = view_sprite_create(renderer, pal, 12 + x, 4 + y, zoom, RESOURCE_GFF_INDEX, GFF_ICON, 11100);
    ports[1] = view_sprite_create(renderer, pal, 12 + x, 52 + y, zoom, RESOURCE_GFF_INDEX, GFF_ICON, 11100);
    ports[2] = view_sprite_create(renderer, pal, 12 + x, 100 + y, zoom, RESOURCE_GFF_INDEX, GFF_ICON, 11100);
    ports[3] = view_sprite_create(renderer, pal, 12 + x, 148 + y, zoom, RESOURCE_GFF_INDEX, GFF_ICON, 11100);

    for (int i = 0; i < 4; i++) {
        sprite_set_frame(ports[i], 2);
        //sprite_set_frame(port_background[i], 3);
    }

    name_loc = apply_params(initial_locs[0], x, y);
    description_loc = apply_params(initial_locs[1], x, y);

    set_zoom(&name_loc, zoom);
    set_zoom(&description_loc, zoom);

    strcpy(name, "NAME");
    strcpy(description, "message");
}

void inventory_screen_render(void *data, SDL_Renderer *renderer) {
    sprite_render(renderer, panel);
    sprite_render(renderer, parchment);
    sprite_render(renderer, flag_spear);
    sprite_set_frame(frame_message, 0);
    sprite_render(renderer, frame_message);
    sprite_set_frame(frame_message, 1);
    sprite_render(renderer, frame_message);
    sprite_set_frame(frame_message, 3);
    sprite_render(renderer, frame_message);

    sprite_render(renderer, character);
    sprite_render(renderer, inv);
    sprite_render(renderer, magic);
    sprite_render(renderer, status);
    sprite_render(renderer, game_menu);
    sprite_render(renderer, game_return);

    for (int i = 0; i < 4; i++) {
        sprite_render(renderer, ai[i]);
        sprite_render(renderer, leader[i]);
        //sprite_render(renderer, port_background[i]);
        sprite_render(renderer, ports[i]);
    }

    for (int i = 9; i < 23; i++) {
        sprite_set_frame(slots, i);
        sprite_render(renderer, slots);
    }

    sprite_set_frame(slots, 0);
    for (int i = 0; i < 6; i++) {
        sprite_set_frame(slots, 5);
        sprite_set_location(slots, (186 * zoom), (18 + 17 * i) * zoom);
        sprite_render(renderer, slots);
        sprite_set_frame(slots, 0);
        sprite_set_location(slots, (186 * zoom), (18 + 17 * i) * zoom);
        sprite_render(renderer, slots);
        sprite_set_frame(slots, 5);
        sprite_set_location(slots, (204 * zoom), (18 + 17 * i) * zoom);
        sprite_render(renderer, slots);
        sprite_set_frame(slots, 0);
        sprite_set_location(slots, (204 * zoom), (18 + 17 * i) * zoom);
        sprite_render(renderer, slots);
    }

    print_line_len(renderer, FONT_YELLOW, name, name_loc.x, name_loc.y, sizeof(name));
    print_line_len(renderer, FONT_GREY, description, description_loc.x, description_loc.y, sizeof(description));

    sprite_render(renderer, slots);

/*
    for (int i = 0; i < 4; i++) {
        if (ds_player_exists(i)) {
            ds1_combat_t* combat = ds_player_get_combat(i);
            ds_character_t* ch = ds_player_get_char(i);
            int x = 56, y = 74;
            if (i == 1 || i == 3) { y += 60; }
            if (i == 2 || i == 3) { x += 50; }
            snprintf(buf, BUF_MAX, "%d/%d", combat->hp, ch->base_hp);
            print_line_len(renderer, FONT_YELLOW, buf, x * zoom, (y + 0) * zoom, BUF_MAX);
            snprintf(buf, BUF_MAX, "%d/%d", combat->psp, ch->base_psp);
            print_line_len(renderer, FONT_BLUE, buf, x * zoom, (y + 8) * zoom, BUF_MAX);
            if (combat->status) {
                snprintf(buf, BUF_MAX, "%d", combat->status);
            } else {
                snprintf(buf, BUF_MAX, "Okay");
            }
            print_line_len(renderer, FONT_YELLOW, buf, x * zoom, (y + 16) * zoom, BUF_MAX);
        }
    }
    */
}

static int get_sprite_mouse_is_on(const uint32_t x, const uint32_t y) {
    if (sprite_in_rect(character, x, y)) { return character; }
    if (sprite_in_rect(inv, x, y)) { return inv; }
    if (sprite_in_rect(magic, x, y)) { return magic; }
    if (sprite_in_rect(status, x, y)) { return status; }
    if (sprite_in_rect(game_menu, x, y)) { return game_menu; }
    if (sprite_in_rect(game_return, x, y)) { return game_return; }
    //if (sprite_in_rect(flag_spear, x, y)) { return flag_spear; }

    for (int i = 0; i < 4; i++) {
        if (sprite_in_rect(ai[i], x, y)) { return ai[i]; }
        if (sprite_in_rect(leader[i], x, y)) { return leader[i]; }
        //if (sprite_in_rect(port_background[i], x, y)) { return port_background[i]; }
        if (sprite_in_rect(ports[i], x, y)) { return ports[i]; }
    }

    return SPRITE_ERROR;
}

int inventory_screen_handle_mouse_movement(const uint32_t x, const uint32_t y) {
    static uint16_t last_sprite = SPRITE_ERROR;

    uint16_t cur_sprite = get_sprite_mouse_is_on(x, y);

    if (last_sprite != cur_sprite) {
        sprite_set_frame(cur_sprite, sprite_get_frame(cur_sprite) + 1);
        if (last_sprite != SPRITE_ERROR) {
            sprite_set_frame(last_sprite, sprite_get_frame(last_sprite) - 1);
        }
    }
    
    last_sprite = cur_sprite;
    return 1;// handle
}

int inventory_screen_handle_mouse_down(const uint32_t button, const uint32_t x, const uint32_t y) {
    return 1; // means I captured the mouse click
}

int inventory_screen_handle_mouse_up(const uint32_t button, const uint32_t x, const uint32_t y) {
    if (button == SDL_BUTTON_RIGHT) {
        for (int i = 0; i < 4; i++) {
            if (sprite_in_rect(ports[i], x, y)) {
                slot_clicked = i;
                screen_push_screen(rend, &popup_screen, 100, 75);
                popup_set_message("INACTIVE CHARACTER");
                popup_set_option(0, "NEW");
                popup_set_option(1, "ADD");
                popup_set_option(2, "CANCEL");
                last_selection = SELECT_POPUP;
            }
        }
    }
    if (sprite_in_rect(game_return, x, y)) { screen_pop(); } 
    return 1; // means I captured the mouse click
}

void inventory_screen_free() {
    sprite_free(panel);
    sprite_free(frame_message);
    sprite_free(parchment);
    sprite_free(flag_spear);

    for (int i = 0; i < 4; i++) {
        sprite_free(ai[i]);
        sprite_free(leader[i]);
        sprite_free(ports[i]);
    }
    sprite_free(game_return);
    sprite_free(game_menu);
    sprite_free(character);
    sprite_free(inv);
    sprite_free(magic);
    sprite_free(status);
}

void inventory_screen_return_control () {
    if (last_selection == SELECT_POPUP) {
        if (popup_get_selection() == POPUP_0) { // new
            popup_clear_selection();
            screen_push_screen(rend, &new_character_screen, 0, 0);
            last_selection = SELECT_NEW;
            return;
        }
        if (popup_get_selection() == POPUP_1) { // ADD
            popup_clear_selection();
            screen_push_screen(rend, &als_screen, 0, 0);
            last_selection = SELECT_ALS;
            return;
        }
    } else if (last_selection == SELECT_NEW) {
        ds_character_t *pc = new_character_get_pc();
        psin_t* psi = new_character_get_psin();
        spell_list_t* spells = new_character_get_spell_list();
        psionic_list_t* psionics = new_character_get_psionic_list();
        char *name = new_character_get_name();
        if (pc && psi && spells && psionics) {
            if (dnd2e_character_is_valid(pc) && dnd2e_psin_is_valid(pc, psi)) {
                gff_char_add_character(pc, psi, spells, psionics, name);
            } else {
                screen_push_screen(rend, &popup_screen, 100, 75);
                popup_set_message("Character was invalid.");
                popup_set_option(0, "TRY AGAIN");
                popup_set_option(1, "ADD");
                popup_set_option(2, "CANCEL");
                last_selection = SELECT_POPUP;
                return;
            }
        }
    } else if (last_selection == SELECT_ALS) {
        if (add_load_save_get_action() == ACTION_ADD) {
            uint32_t sel = add_load_save_get_selection();
            //if (!ds_player_load_character_charsave(slot_clicked, sel)) {
            if (!ds_load_character_charsave(slot_clicked, sel)) {
                printf("Char loading failed.\n");
            }
        }
    }
    last_selection = SELECT_NONE;
}

sops_t inventory_screen = {
    .init = inventory_screen_init,
    .cleanup = inventory_screen_free,
    .render = inventory_screen_render,
    .mouse_movement = inventory_screen_handle_mouse_movement,
    .mouse_down = inventory_screen_handle_mouse_down,
    .mouse_up = inventory_screen_handle_mouse_up,
    .return_control = inventory_screen_return_control,
    .data = NULL
};
