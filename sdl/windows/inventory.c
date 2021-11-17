#include "view-character.h"
#include "../main.h"
#include "../../src/gff.h"
#include "../../src/gff-char.h"
#include "gfftypes.h"
#include "../../src/rules.h"
#include "narrate.h"
#include "../player.h"
#include "new-character.h"
#include "popup.h"
#include "../../src/dsl.h"
#include "add-load-save.h"
#include "../../src/player.h"
#include "../../src/ds-load-save.h"
#include "../../src/settings.h"
#include "../font.h"
#include "../sprite.h"
#include "../mouse.h"

// Sprites
static uint16_t ai[4];
static uint16_t leader[4];
static uint16_t ports[4];
static uint16_t flag_spear, parchment, frame_message, panel, slots;
static uint16_t character, inv, magic, status;
static uint16_t game_menu, game_return;
enum {SELECT_NONE, SELECT_POPUP, SELECT_NEW, SELECT_ALS};
static int8_t last_selection = SELECT_NONE;
static uint8_t slot_clicked;
static uint32_t mousex, mousey;
static int char_selected;
static uint32_t xoffset, yoffset;

static SDL_Rect initial_locs[] = {{ 59, 5, 0, 0 }, // name
                                  { 52, 165, 174, 0 }, // description
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

void inventory_window_init(const uint32_t _xoffset, const uint32_t _yoffset) {
    gff_palette_t *pal = open_files[RESOURCE_GFF_INDEX].pals->palettes + 0;
    const float zoom = settings_zoom();
    mousex = mousey = 0;
    char_selected = 0;
    xoffset = _xoffset;
    yoffset = _yoffset;
    uint32_t x = xoffset / zoom;
    uint32_t y = yoffset / zoom;
    SDL_Renderer *renderer = rend = main_get_rend();

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
        sprite_set_location(slots, (x + slot_locs[i].x) * zoom, (y + slot_locs[i].y) * zoom);
    }

    character = view_sprite_create(renderer, pal, 160 + x, 181 + y, zoom, RESOURCE_GFF_INDEX, GFF_ICON, 10100);
    inv = view_sprite_create(renderer, pal, 185 + x, 181 + y, zoom, RESOURCE_GFF_INDEX, GFF_ICON, 11102);
    sprite_set_frame(inv, 3);
    magic = view_sprite_create(renderer, pal, 210 + x, 181 + y, zoom, RESOURCE_GFF_INDEX, GFF_ICON, 11103);
    status = view_sprite_create(renderer, pal, 235 + x, 181 + y, zoom, RESOURCE_GFF_INDEX, GFF_ICON, 11104);
    game_return = view_sprite_create(renderer, pal, 288 + x, 181 + y, zoom, RESOURCE_GFF_INDEX, GFF_ICON, 10108);
    game_menu = view_sprite_create(renderer, pal, 258 + x, 181 + y, zoom, RESOURCE_GFF_INDEX, GFF_ICON, 11101);
    sprite_set_frame(game_menu, 2);

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
    }

    name_loc = apply_params(initial_locs[0], x, y);
    description_loc = apply_params(initial_locs[1], x, y);

    set_zoom(&name_loc, zoom);
    set_zoom(&description_loc, zoom);

    strcpy(name, "NAME");
    strcpy(description, "message");
}

static const char *slot_names[] = {
    "ARM",
    "AMMO",
    "MISSILE",
    "RIGHT HAND",
    "FINGER",
    "WAIST",
    "LEGS",
    "HEAD",
    "NECK",
    "TORSO",
    "LEFT HAND",
    "FINGER",
    "CLOAK",
    "FOOT",
    "BACKPACK"
};

static int do_slot_highlight(SDL_Renderer *renderer, const uint16_t frame, const uint16_t pos) {
    if (sprite_in_rect(slots, mousex, mousey)) {
        uint32_t x = sprite_getx(slots);
        uint32_t y = sprite_gety(slots);

        sprite_set_frame(slots, 7); // highight

        sprite_set_location(slots, x, y);
        sprite_render(renderer, slots);
        sprite_set_frame(slots, frame);
        strcpy(description, slot_names[pos]);
        return 1;
    }

    return 0;
}

#define BUF_MAX (1<<8)

static int display_attack(entity_t *entity, item_t *item, const int xpos, const int ypos) {
    char buf[BUF_MAX];
    int pos = 0;
    int offset = 0;
    const float zoom = settings_zoom();

    if (item && item->type != ITEM_MELEE && item->type != ITEM_MISSILE_THROWN
            && item->type != ITEM_MISSILE_USE_AMMO) { return 0;}

    if (item == NULL) {
        pos += snprintf(buf, BUF_MAX, "<");
    } else {
        print_line_len(rend, FONT_YELLOW, item->name, xoffset + 235 * zoom, yoffset + ypos * zoom, BUF_MAX);
        offset += 7;
    }

    uint16_t num_attacks = dnd2e_get_attack_num(entity, item);
    pos += snprintf(buf + pos, BUF_MAX - pos, "%d%sx",
        num_attacks >> 1, // num_attacks is half-attacks.
        (num_attacks & 0x01) ? ".5" : "");
    pos += snprintf(buf + pos, BUF_MAX - pos, "%dD%d%s%d\n", 
        dnd2e_get_attack_die_pc(entity, item),
        dnd2e_get_attack_sides_pc(entity, item),
        "+",
        dnd2e_get_attack_mod_pc(entity, item));

    if (item == NULL) {
        pos += snprintf(buf + pos, BUF_MAX - pos, ">");
    }

    print_line_len(rend, FONT_YELLOW, buf, xoffset + 235 * zoom, yoffset + (ypos + offset) * zoom, BUF_MAX);
    return offset + 7;
}

static void render_character() {
    char buf[BUF_MAX];
    entity_t *player = player_get(char_selected);
    if (!player) { return; }
    inventory_t *player_items = (inventory_t*)player->inv;
    const float zoom = settings_zoom();

    if (player->name == NULL) { return; } // no character.
    strcpy(name, player->name);

    print_line_len(rend, FONT_YELLOW, "STR:", xoffset + 235 * zoom, yoffset + 53 * zoom, BUF_MAX);
    print_line_len(rend, FONT_YELLOW, "DEX:", xoffset + 235 * zoom, yoffset + 60 * zoom, BUF_MAX);
    print_line_len(rend, FONT_YELLOW, "CON:", xoffset + 235 * zoom, yoffset + 67 * zoom, BUF_MAX);
    print_line_len(rend, FONT_YELLOW, "INT:", xoffset + 235 * zoom, yoffset + 74 * zoom, BUF_MAX);
    print_line_len(rend, FONT_YELLOW, "WIS:", xoffset + 235 * zoom, yoffset + 81 * zoom, BUF_MAX);
    print_line_len(rend, FONT_YELLOW, "CHA:", xoffset + 235 * zoom, yoffset + 88 * zoom, BUF_MAX);
    snprintf(buf, BUF_MAX, "%d\n", player->stats.str);
    print_line_len(rend, FONT_YELLOW, buf, xoffset + 260 * zoom, yoffset + 53 * zoom, BUF_MAX);
    snprintf(buf, BUF_MAX, "%d\n", player->stats.dex);
    print_line_len(rend, FONT_YELLOW, buf, xoffset + 260 * zoom, yoffset + 60 * zoom, BUF_MAX);
    snprintf(buf, BUF_MAX, "%d\n", player->stats.con);
    print_line_len(rend, FONT_YELLOW, buf, xoffset + 260 * zoom, yoffset + 67 * zoom, BUF_MAX);
    snprintf(buf, BUF_MAX, "%d\n", player->stats.intel);
    print_line_len(rend, FONT_YELLOW, buf, xoffset + 260 * zoom, yoffset + 74 * zoom, BUF_MAX);
    snprintf(buf, BUF_MAX, "%d\n", player->stats.wis);
    print_line_len(rend, FONT_YELLOW, buf, xoffset + 260 * zoom, yoffset + 81 * zoom, BUF_MAX);
    snprintf(buf, BUF_MAX, "%d\n", player->stats.cha);
    print_line_len(rend, FONT_YELLOW, buf, xoffset + 260 * zoom, yoffset + 88 * zoom, BUF_MAX);

    snprintf(buf, BUF_MAX, "PSI:  %d/%d\n", player->stats.psp, player->stats.high_psp);
    print_line_len(rend, FONT_YELLOW, buf, xoffset + 235 * zoom, yoffset + 100 * zoom, BUF_MAX);

    snprintf(buf, BUF_MAX, "AC: %d\n", dnd2e_calc_ac(player));
    print_line_len(rend, FONT_YELLOW, buf, xoffset + 235 * zoom, yoffset + 115 * zoom, BUF_MAX);

    int ypos = 125;
    if (player_items->missile.ds_id != 0) {
        ypos += display_attack(player, &(player_items->missile), 235, ypos);
    }
    int cypos = ypos;

    ypos += display_attack(player, &(player_items->hand0), 235, ypos);
    ypos += display_attack(player, &(player_items->hand1), 235, ypos);

    if (ypos == cypos) { // no attack items
        ypos += display_attack(player, NULL, 235, ypos);
    }
}

static void render_backpack_slot(const int slot, const int frame, const int x, const int y, item_t *items) {
    sprite_set_frame(slots, 5);
    sprite_set_location(slots, x, y);
    sprite_render(rend, slots);
    sprite_set_frame(slots, frame);
    sprite_set_location(slots, x, y);
    sprite_render(rend, slots);
    animate_sprite_t *as = items ? &(items[slot].anim) : NULL;

    if (as) {
        sprite_center_spr(as->spr, slots);
        sprite_render(rend, as->spr);
    }

    do_slot_highlight(rend, 5, 14);

    if (item_allowed_in_slot(mouse_get_item(), slot)) {
        sprite_set_frame(slots, sprite_in_rect(slots, mousex, mousey) ? 8 : 4);
        sprite_set_location(slots, x, y);
        sprite_render(rend, slots);
    }
}

void inventory_window_render(void *data) {
    const float zoom = settings_zoom();
    SDL_Renderer *renderer = main_get_rend();

    description[0] = '\0';
    entity_t *player = player_get(char_selected);
    item_t *items = player ? player->inv : NULL;
    animate_sprite_t *as = NULL;

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
        sprite_set_frame(ports[i], player_exists(i) ? 1 : 2);
        sprite_render(renderer, ports[i]);
        if (sprite_in_rect(ports[i], mousex, mousey)) {
            sprite_set_frame(ports[i], 1);
            sprite_render(renderer, ports[i]);
            if (player_exists(i)) {
                snprintf(description, 128, "%s%s",
                        i == char_selected ? "" : "SELECT ",
                        player->name);
            } else {
                strcpy(description, "PRESS RIGHT MOUSE BUTTON");
            }
        }
        if (char_selected == i) {
            sprite_set_frame(ports[i], 3);
            sprite_render(renderer, ports[i]);
        } else {
            sprite_set_frame(ports[i], 0);
            sprite_render(renderer, ports[i]);
        }
    }

    for (int i = 9; i < 23; i++) {
        as = items ? item_icon(items + i - 9) : NULL;
        sprite_set_frame(slots, i);
        int32_t x = sprite_getx(slots);
        int32_t y = sprite_gety(slots);
        if (as && as->spr != SPRITE_ERROR) {
            sprite_set_frame(slots, 2);
            sprite_set_location(slots, x, y);
            sprite_render(renderer, slots);
            sprite_center_spr(as->spr, slots);
            sprite_render(renderer, as->spr);
        } else {
            sprite_render(renderer, slots);
        }
        do_slot_highlight(renderer, i, i - 9);
        if (item_allowed_in_slot(mouse_get_item(), i - 9)) {
            sprite_set_frame(slots, sprite_in_rect(slots, mousex, mousey) ? 8 : 4);
            sprite_set_location(slots, x, y);
            sprite_render(rend, slots);
        }
    }

    sprite_set_frame(slots, 0);
    for (int i = 0; i < 6; i++) {
        render_backpack_slot(14 + 2 * i, 0, xoffset + 186 * zoom, yoffset + (18 + 18 * i) * zoom, items);
        render_backpack_slot(15 + 2 * i, 0, xoffset + 204 * zoom, yoffset + (18 + 18 * i) * zoom, items);
    }

    if (sprite_in_rect(character, mousex, mousey)) { strcpy(description, "VIEW CHARACTER"); }
    if (sprite_in_rect(inv, mousex, mousey)) { strcpy(description, "VIEW INVENTORY"); }
    if (sprite_in_rect(magic, mousex, mousey)) { strcpy(description, "CAST SPELL/USE PSIONIC"); }
    if (sprite_in_rect(status, mousex, mousey)) { strcpy(description, "CURRENT SPELL EFFECTS"); }
    if (sprite_in_rect(game_return, mousex, mousey)) { strcpy(description, "GAME RETURN"); }

    print_line_len(renderer, FONT_YELLOW, name, name_loc.x, name_loc.y, sizeof(name));
    font_render_center(renderer, FONT_GREY, description, description_loc);

    render_character();

    for (int i = 0; i < 4; i++) {
        sol_player_center(i, xoffset + 12 * zoom, yoffset + (4 + 48 * i) * zoom, 34 * zoom, 34 * zoom);
        sol_sprite_t spr = sol_player_get_sprite(i);
        if (sprite_geth(spr) > 30 * zoom) {
            sprite_set_frame(spr, 0);
            sprite_render_box(rend, spr, xoffset + 15 * zoom, yoffset + 11 + (48 * i) * zoom,
                30 * zoom, 30 * zoom);
        } else {
            sol_player_render(i);
        }
    }

    if (player_exists(char_selected)) {
        sol_player_center_portrait(char_selected, xoffset + (75) * zoom, yoffset + (36) * zoom, 90 * zoom, 125 * zoom);
        player_render_portrait(char_selected);
    }
}

static int get_sprite_mouse_is_on(const uint32_t x, const uint32_t y) {
    if (sprite_in_rect(character, x, y)) { return character; }
    if (sprite_in_rect(magic, x, y)) { return magic; }
    if (sprite_in_rect(status, x, y)) { return status; }
    if (sprite_in_rect(game_return, x, y)) { return game_return; }

    for (int i = 0; i < 4; i++) {
        if (sprite_in_rect(ai[i], x, y)) { return ai[i]; }
        if (sprite_in_rect(leader[i], x, y)) { return leader[i]; }
    }

    return SPRITE_ERROR;
}

int inventory_window_handle_mouse_movement(const uint32_t x, const uint32_t y) {
    static uint16_t last_sprite = SPRITE_ERROR;

    uint16_t cur_sprite = get_sprite_mouse_is_on(x, y);

    mousex = x;
    mousey = y;

    if (last_sprite != cur_sprite) {
        sprite_set_frame(cur_sprite, sprite_get_frame(cur_sprite) + 1);
        if (last_sprite != SPRITE_ERROR) {
            sprite_set_frame(last_sprite, sprite_get_frame(last_sprite) - 1);
        }
    }
    
    last_sprite = cur_sprite;
    return 1;// handle
}

int inventory_window_handle_mouse_down(const uint32_t button, const uint32_t x, const uint32_t y) {
    return 1; // means I captured the mouse click
}

static void clicked_slot(const int slot) {
    item_t* mouse_item = mouse_get_item();
    entity_t *player = player_get(char_selected);
    item_t *player_item = player->inv + slot;

    if (mouse_item) { // mouse has an item
        if (!item_allowed_in_slot(mouse_get_item(), slot)) { return; }
        if (player_item->ds_id) { // If we are doing a swap
            // Order matters.
            player_item = item_dup(player_item);
            entity_copy_item(player, mouse_item, slot);
            mouse_set_as_item(player_item);
            item_free_except_graphics(player_item);
        } else {
            entity_copy_item(player, mouse_item, slot);
            mouse_free_item();
        }
    } else if (player_item->ds_id) {
        mouse_set_as_item(player->inv + slot);
        entity_clear_item(player, slot);
    }
}

int inventory_window_handle_mouse_up(const uint32_t button, const uint32_t x, const uint32_t y) {
    const float zoom = settings_zoom();

    for (int i = 0; i < 4; i++) {
        if (sprite_in_rect(ports[i], x, y)) {
            if (button == SDL_BUTTON_RIGHT) {
                slot_clicked = i;
                window_push(&popup_window, 100, 75);
                popup_set_message("INACTIVE CHARACTER");
                popup_set_option(0, "NEW");
                popup_set_option(1, "ADD");
                popup_set_option(2, "CANCEL");
                last_selection = SELECT_POPUP;
            } else if (button == SDL_BUTTON_LEFT) {
                char_selected = i;
            }
        }
    }

    if (sprite_in_rect(game_return, x, y)) {
        window_pop();
        return 1;
    } 

    if (sprite_in_rect(character, x, y)) {
        window_push(&view_character_window, 0, 10);
        return 1;
    } 

    for (int i = 9; i < 23; i++) {
        sprite_set_frame(slots, i);
        if (sprite_in_rect(slots, x, y)) {
            clicked_slot(i-9);
        }
    }

    sprite_set_frame(slots, 5);
    for (int i = 0; i < 6; i++) {
        sprite_set_location(slots, xoffset + 186 * zoom, yoffset + (18 + 18 * i) * zoom);
        if (sprite_in_rect(slots, x, y)) {
            clicked_slot(14 + 2 * i);
        }
        sprite_set_location(slots, xoffset + 204 * zoom, yoffset + (18 + 18 * i) * zoom);
        if (sprite_in_rect(slots, x, y)) {
            clicked_slot(15 + 2 * i);
        }
    }

    return 1; // means I captured the mouse click
}

void inventory_window_free() {
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
    sprite_free(slots);
}

void inventory_window_return_control () {
    if (last_selection == SELECT_POPUP) {
        if (popup_get_selection() == POPUP_0) { // new
            popup_clear_selection();
            window_push(&new_character_window, 0, 0);
            last_selection = SELECT_NEW;
            return;
        }
        if (popup_get_selection() == POPUP_1) { // ADD
            popup_clear_selection();
            window_push(&als_window, 0, 0);
            last_selection = SELECT_ALS;
            return;
        }
    } else if (last_selection == SELECT_NEW) {
        entity_t *pc = new_character_get_pc();
        psin_t* psi = new_character_get_psin();
        ssi_spell_list_t* spells = new_character_get_spell_list();
        psionic_list_t* psionics = new_character_get_psionic_list();
        //char *name = new_character_get_name();
        if (pc && psi && spells && psionics) {
            if (dnd2e_character_is_valid(pc)) {// && dnd2e_psin_is_valid(pc, psi)) {
                warn ("TODO: PUT BACK IN CHARACTER ADDING!\n");
                //gff_char_add_character(pc, psi, spells, psionics, name);
                sol_player_load(slot_clicked, settings_zoom());
            } else {
                window_push(&popup_window, 100, 75);
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
            if (!ds_load_character_charsave(slot_clicked, sel)) {
                printf("Char loading failed.\n");
            } else {
                sol_player_load(slot_clicked, settings_zoom());
                char_selected = slot_clicked;
            }
        }
    }
    last_selection = SELECT_NONE;
}

wops_t inventory_window = {
    .init = inventory_window_init,
    .cleanup = inventory_window_free,
    .render = inventory_window_render,
    .mouse_movement = inventory_window_handle_mouse_movement,
    .mouse_down = inventory_window_handle_mouse_down,
    .mouse_up = inventory_window_handle_mouse_up,
    .return_control = inventory_window_return_control,
    .name = "inventory",
    .grey_out_map = 1,
    .data = NULL
};
