#include "view-character.h"
#include "gfftypes.h"
#include "narrate.h"
#include "description.h"
#include "inventory.h"
#include "new-character.h"
#include "label.h"
#include "popup.h"
#include "add-load-save.h"
#include "port.h"
#include "gff.h"
#include "gff-char.h"
#include "rules.h"
#include "settings.h"
#include "gpl.h"
#include "player.h"
#include "ds-load-save.h"


#include <string.h>

#define MAX_POWERS (50)
// Sprites
static sol_sprite_t ai[4], leader[4], ports[4], buttons[3];
static sol_label_t power_name, power_level;
static sol_sprite_t effects, view_char, use, panel, sun;
static sol_sprite_t character, inv, powers, status;
static sol_sprite_t game_menu, game_return;
static sol_sprite_t slots;
static sol_sprite_t power_highlight, power_background;
enum {SELECT_NONE, SELECT_POPUP, SELECT_NEW, SELECT_ALS};
static int8_t last_selection = SELECT_NONE;
static uint8_t slot_clicked;
static int32_t player_selected = 0;
static uint32_t xoffset, yoffset;
static uint32_t mousex = 0, mousey = 0;
static int mode = 2; // 0 = char, 2 = powers, 3 = status
static power_t *power_list[MAX_POWERS];
static power_t *power_to_display = NULL;
static int level = 1;

static sol_dim_t initial_locs[] = {{ 155, 28, 0, 0 }, // description
                                  { 60, 155, 0, 0 }, // message
};

static sol_dim_t description_loc, message_loc;
static char description[25];
static char message[128];

static void set_zoom(sol_dim_t *loc, float zoom) {
    loc->x *= zoom;
    loc->y *= zoom;
    loc->w *= zoom;
    loc->h *= zoom;
}

static sol_dim_t apply_params(const sol_dim_t rect, const uint32_t x, const uint32_t y) {
    sol_dim_t ret = {rect.x + x, rect.y + y, rect.w, rect.h};
    return ret;
}

static void set_power(const int type, const int level) {
    char buf[64];
    power_list_t *powers = wizard_get_spells(level);
    power_instance_t *rover = powers ? powers->head : NULL;
    size_t power_pos = 0;
    if (!rover) { return; }
    while (rover) {
        power_list[power_pos++] = rover->stats;
        rover = rover->next;
    }
    while (power_pos < MAX_POWERS) {
        power_list[power_pos++] = NULL;
    }
    snprintf(buf, 63, "LEVEL %d", level);
    sol_label_set_text(&power_level, buf);
}

void view_character_init(const uint32_t _x, const uint32_t _y) {
    gff_palette_t *pal = open_files[RESOURCE_GFF_INDEX].pals->palettes + 0;
    const float zoom = settings_zoom();
    uint32_t x = _x / settings_zoom(), y = _y / settings_zoom();
    sol_dude_t *player;
    xoffset = _x;
    yoffset = _y;
    sol_player_get_active(&player);
    sol_player_get_slot(player, &player_selected);

    memset(description, 0x0, sizeof(description));
    memset(message, 0x0, sizeof(message));
    memset(power_list, 0x0, sizeof(power_t*) * MAX_POWERS);

    sol_status_check(sol_sprite_new(pal, 0 + x, 0 + y, zoom, RESOURCE_GFF_INDEX, GFF_BMP, 11000, &panel),
            "Unable load vc");
    sol_status_check(sol_sprite_new(pal, 80 + x, 0 + y, zoom, RESOURCE_GFF_INDEX, GFF_BMP, 20075, &effects),
            "Unable load vc");
    sol_status_check(sol_sprite_new(pal, 60 + x, 0 + y, zoom, RESOURCE_GFF_INDEX, GFF_BMP, 20079, &view_char),
            "Unable load vc");
    sol_status_check(sol_sprite_new(pal, 115 + x, 0 + y, zoom, RESOURCE_GFF_INDEX, GFF_BMP, 20080, &use),
            "Unable load vc");
    sol_status_check(sol_sprite_new(pal, 50 + x, 0 + y - 11, zoom, RESOURCE_GFF_INDEX, GFF_BMP, 20028, &sun),
            "Unable load vc");

    sol_status_check(sol_sprite_new(pal, 45 + x, 155 + y, zoom, RESOURCE_GFF_INDEX, GFF_ICON, 10100, &character),
            "Unable load vc");
    sol_status_check(sol_sprite_new(pal, 68 + x, 155 + y, zoom, RESOURCE_GFF_INDEX, GFF_ICON, 11102, &inv),
            "Unable load vc");
    sol_status_check(sol_sprite_new(pal, 91 + x, 155 + y, zoom, RESOURCE_GFF_INDEX, GFF_ICON, 11103, &powers),
            "Unable load vc");
    sol_status_check(sol_sprite_new(pal, 112 + x, 155 + y, zoom, RESOURCE_GFF_INDEX, GFF_ICON, 11104, &status),
            "Unable load vc");
    sol_status_check(sol_sprite_new(pal, 252 + x, 155 + y, zoom, RESOURCE_GFF_INDEX, GFF_ICON, 10108, &game_return),
            "Unable load vc");
    sol_status_check(sol_sprite_new(pal, 222 + x, 155 + y, zoom, RESOURCE_GFF_INDEX, GFF_ICON, 11101, &game_menu),
            "Unable load vc");
    sol_status_check(sol_sprite_new(pal, 53 + x, 3 + y, zoom, RESOURCE_GFF_INDEX, GFF_BMP, 13007, &slots),
            "Unable load vc");
    power_highlight = sol_sprite_new(pal, 53 + x, 3 + y, zoom, RESOURCE_GFF_INDEX, GFF_BMP, 20088, &power_highlight),
    power_background = sol_sprite_new(pal, 53 + x, 3 + y, zoom, RESOURCE_GFF_INDEX, GFF_BMP, 15002, &power_background),

    sol_status_check(sol_sprite_new(pal, 45 + x, 40 + y, zoom, RESOURCE_GFF_INDEX, GFF_ICON, 11111, &ai[0]),
            "Unable load vc");
    sol_status_check(sol_sprite_new(pal, 45 + x, 100 + y, zoom, RESOURCE_GFF_INDEX, GFF_ICON, 11111, &ai[1]),
            "Unable load vc");
    sol_status_check(sol_sprite_new(pal, 95 + x, 40 + y, zoom, RESOURCE_GFF_INDEX, GFF_ICON, 11111, &ai[2]),
            "Unable load vc");
    sol_status_check(sol_sprite_new(pal, 95 + x, 100 + y, zoom, RESOURCE_GFF_INDEX, GFF_ICON, 11111, &ai[3]),
            "Unable load vc");
    sol_status_check(sol_sprite_new(pal, 45 + x, 30 + y, zoom, RESOURCE_GFF_INDEX, GFF_ICON, 11106, &leader[0]),
            "Unable load vc");
    sol_status_check(sol_sprite_new(pal, 45 + x, 90 + y, zoom, RESOURCE_GFF_INDEX, GFF_ICON, 11106, &leader[1]),
            "Unable load vc");
    sol_status_check(sol_sprite_new(pal, 95 + x, 30 + y, zoom, RESOURCE_GFF_INDEX, GFF_ICON, 11106, &leader[2]),
            "Unable load vc");
    sol_status_check(sol_sprite_new(pal, 95 + x, 90 + y, zoom, RESOURCE_GFF_INDEX, GFF_ICON, 11106, &leader[3]),
            "Unable load vc");
    sol_status_check(sol_sprite_new(pal, 55 + x, 30 + y, zoom, RESOURCE_GFF_INDEX, GFF_ICON, 11100, &ports[0]),
            "Unable load vc");
    sol_status_check(sol_sprite_new(pal, 55 + x, 90 + y, zoom, RESOURCE_GFF_INDEX, GFF_ICON, 11100, &ports[1]),
            "Unable load vc");
    sol_status_check(sol_sprite_new(pal, 105 + x, 30 + y, zoom, RESOURCE_GFF_INDEX, GFF_ICON, 11100, &ports[2]),
            "Unable load vc");
    sol_status_check(sol_sprite_new(pal, 105 + x, 90 + y, zoom, RESOURCE_GFF_INDEX, GFF_ICON, 11100, &ports[3]),
            "Unable load vc");
    sol_status_check(sol_sprite_new(pal, 131 + x, 157 + y, zoom, RESOURCE_GFF_INDEX, GFF_ICON, 11109, &buttons[0]),
            "Unable load vc");
    sol_status_check(sol_sprite_new(pal, 172 + x, 157 + y, zoom, RESOURCE_GFF_INDEX, GFF_ICON, 11110, &buttons[1]),
            "Unable load vc");
    sol_status_check(sol_sprite_new(pal, 172 + x, 157 + y, zoom, RESOURCE_GFF_INDEX, GFF_ICON, 13006, &buttons[2]),
            "Unable load vc");
    sol_label_create_at_pos(0, -1, "MAGE", FONT_GREY, (135 + x) * zoom, (158 + y) * zoom, &power_name);
    sol_label_create_at_pos(0, -1, "LEVEL 1", FONT_GREY, (175 + x) * zoom, (158 + y) * zoom, &power_level);

    for (int i = 0; i < 4; i++) {
        sol_sprite_set_frame(ports[i], 2);
    }

    description_loc = apply_params(initial_locs[0], x, y);
    message_loc.x = (75 + x) * zoom;
    message_loc.y = (175 + y) * zoom;
    message_loc.h = 8 * zoom;
    message_loc.w = 180 * zoom;

    set_zoom(&description_loc, zoom);

    strcpy(description, "description");
    sol_player_get_active(&player);
    if (player && player->name) {
        strcpy(description, player->name);
    }
    //strcpy(message, "message");
    sol_label_create_group();
    sol_label_group_set_font(FONT_YELLOW);
    sol_label_set_positions(143 * zoom, 30 * zoom, SCREEN_VIEW_CHARACTER);

    set_power(0, level);
}

static void render_character() {
    const float zoom = settings_zoom();
    sol_dude_t *player;

    sol_player_get(player_selected, &player);
    if (!player) { return; }

    sol_label_set_group(player, SCREEN_VIEW_CHARACTER);
    sol_label_set_positions(143 * zoom, 30 * zoom, SCREEN_VIEW_CHARACTER);
    sol_label_render_stats(xoffset, yoffset);
    sol_label_render_gra(xoffset + (64 * zoom), yoffset - (49 * zoom));
    sol_label_render_class_and_combat(xoffset + (64 * zoom), yoffset - (42 * zoom));

    sol_sprite_set_frame(slots, 0);
    sol_item_t *items = player->inv;
    animate_sprite_t *as = NULL;
    sol_sprite_set_location(slots, xoffset + (205 * zoom), yoffset + (41 * zoom));
    sol_sprite_render(slots);
    if (items) {
        as = &(items[2].anim);
        if (as && items[2].name[0]) {
            sol_sprite_center_spr(as->spr, slots);
            sol_sprite_render(as->spr);
        }
    }

    sol_sprite_set_location(slots, xoffset + (223 * zoom), yoffset + (41 * zoom));
    sol_sprite_render(slots);
    if (items) {
        as = &(items[3].anim);
        if (as && items[3].name[0]) {
            sol_sprite_center_spr(as->spr, slots);
            sol_sprite_render(as->spr);
        }
    }

    sol_sprite_set_location(slots, xoffset + (241 * zoom), yoffset + (41 * zoom));
    sol_sprite_render(slots);
    if (items) {
        as = &(items[10].anim);
        if (as && items[10].name[0]) {
            sol_sprite_center_spr(as->spr, slots);
            sol_sprite_render(as->spr);
        }
    }
}

static power_t* find_power(const uint32_t x, const uint32_t y) {
    size_t power_pos = 0;
    while (power_list[power_pos]) {
        animate_sprite_t *as = &(power_list[power_pos]->icon);
        if (as && sol_sprite_in_rect(as->spr, x, y) == SOL_SUCCESS) {
            return power_list[power_pos];
        }
        power_pos++;
    }

    return NULL;
}

static void sprite_highlight(const uint32_t x, const uint32_t y) {
    const float zoom = settings_zoom();
    power_t *pw = find_power(x, y);
    sol_sprite_info_t info;

    if (!pw) { return; }
    animate_sprite_t *as = &(pw->icon);

    sol_status_check(sol_sprite_get_info(as->spr, &info), "Unable to get sprite info.");
    sol_sprite_set_frame(power_highlight, 4);
    sol_sprite_set_location(power_highlight, info.x - 1 * zoom,
            info.y - 1 * zoom);
    sol_sprite_render(power_highlight);
    strcpy(message, pw->name);
}

static void render_powers() {
    const float zoom = settings_zoom();
    sol_sprite_render(buttons[0]);
    sol_sprite_render(buttons[1]);

    sol_label_render(&power_name);
    sol_label_render(&power_level);

    for (int i = 0; i < MAX_POWERS && power_list[i]; i++) {
        animate_sprite_t *as = power_get_icon(power_list[i]);
        sol_sprite_set_location(as->spr, xoffset + (170 + (i % 5) * 20) * zoom,
            yoffset + (42 + (i / 5) * 20) * zoom);
        sol_sprite_render(as->spr);
    }

    sprite_highlight(mousex, mousey);
}

static int get_next_len(const char *str, const int max) {
    int ret = 0;
    for (int i = 0; i < max; i++) {
        if (str[i] == ' ') { ret = i + 1; }
        if (str[i] == '\0') {
            return i;
        }
    }
    return ret;
}

#define BUF_MAX (1<<12)

void view_character_render(void *data) {
    char buf[BUF_MAX];
    const float zoom = settings_zoom();
    sol_sprite_info_t info;
    sol_dude_t *player, *active;

    message[0] = '\0';
    sol_sprite_render(sun);
    sol_sprite_render(panel);
    sol_sprite_render(view_char);

    sol_sprite_set_frame((mode == 0) ? character
            : (mode == 2) ? powers
            : status, 3);

    sol_sprite_render(character);
    sol_sprite_render(inv);
    sol_sprite_render(powers);
    sol_sprite_render(status);
    sol_sprite_render(game_menu);
    sol_sprite_render(game_return);

    for (int i = 0; i < 4; i++) {
        sol_sprite_set_frame(ai[i], 0);
        if (sol_player_ai(i) != SOL_SUCCESS) {
            sol_sprite_set_frame(ai[i], 1);
        }
        sol_sprite_render(ai[i]);
        sol_sprite_set_frame(leader[i], 0);
        sol_player_get(i, &player);
        sol_player_get_active(&active);
        if (sol_player_exists(i) == SOL_SUCCESS && player == active) {
            sol_sprite_set_frame(leader[i], 1);
        }
        sol_sprite_render(leader[i]);
    }

    for (int i = 0; i < 4; i++) {
        if (sol_player_exists(i) == SOL_SUCCESS) {
            sol_entity_t *player;
            sol_player_get(i, &player);
            int x = 56, y = 64;
            if (i == 1 || i == 3) { y += 60; }
            if (i == 2 || i == 3) { x += 50; }
            snprintf(buf, BUF_MAX, "%d/%d", player->stats.hp, player->stats.high_hp);
            sol_print_line_len(FONT_YELLOW, buf, xoffset + x * zoom, yoffset + (y + 0) * zoom, BUF_MAX);
            snprintf(buf, BUF_MAX, "%d/%d", player->stats.psp, player->stats.high_psp);
            sol_print_line_len(FONT_BLUE, buf, xoffset + x * zoom, yoffset + (y + 8) * zoom, BUF_MAX);
            if (player->combat_status) {
                snprintf(buf, BUF_MAX, "%d", player->combat_status);
            } else {
                snprintf(buf, BUF_MAX, "Okay");
            }
            sol_print_line_len(FONT_YELLOW, buf, xoffset + x * zoom, yoffset + (y + 16) * zoom, BUF_MAX);
            sol_sprite_set_frame(ports[i], (i == player_selected) ? 3 : 0);
            sol_sprite_render(ports[i]);
            sol_player_center(i, xoffset + x * zoom, yoffset + (y - 34) * zoom, 34 * zoom, 34 * zoom);
            sol_sprite_t spr;
            sol_player_get_sprite(i, &spr);
            sol_status_check(sol_sprite_get_info(spr, &info), "Unable to get sprite info.");
            if (info.h > 30 * zoom) {
                sol_sprite_set_frame(spr, 0);
                sol_sprite_render_box(spr, xoffset + x * zoom, yoffset + (y - 34) * zoom,
                    34 * zoom, 34 * zoom);
            } else {
                sol_player_render(i);
            }
        } else {
            sol_sprite_set_frame(ports[i], 2);
            sol_sprite_render(ports[i]);
        }
    }

    switch(mode) {
        case 0: render_character(); break;
        case 2: render_powers(); break;
        default: break;
    }

    sol_print_line_len(FONT_YELLOW, description, description_loc.x, description_loc.y, sizeof(description));
    sol_font_render_center(FONT_GREY, message, message_loc.x, message_loc.y, message_loc.w);
}

static int get_sprite_mouse_is_on(const uint32_t x, const uint32_t y) {
    if (sol_sprite_in_rect(character, x, y) == SOL_SUCCESS) { return character; }
    if (sol_sprite_in_rect(inv, x, y) == SOL_SUCCESS) { return inv; }
    if (sol_sprite_in_rect(powers, x, y) == SOL_SUCCESS) { return powers; }
    if (sol_sprite_in_rect(status, x, y) == SOL_SUCCESS) { return status; }
    if (sol_sprite_in_rect(game_menu, x, y) == SOL_SUCCESS) { return game_menu; }
    if (sol_sprite_in_rect(game_return, x, y) == SOL_SUCCESS) { return game_return; }
    //if (sol_sprite_in_rect(effects, x, y)) { return effects; }

    for (int i = 0; i < 4; i++) {
        if (sol_sprite_in_rect(ai[i], x, y) == SOL_SUCCESS) { return ai[i]; }
        if (sol_sprite_in_rect(leader[i], x, y) == SOL_SUCCESS) { return leader[i]; }
        //if (sol_sprite_in_rect(port_background[i], x, y)) { return port_background[i]; }
        if (sol_sprite_in_rect(ports[i], x, y) == SOL_SUCCESS) { return ports[i]; }
    }

    if (mode == 2) {
        if (sol_sprite_in_rect(buttons[0], x, y) == SOL_SUCCESS) { return buttons[0]; }
        if (sol_sprite_in_rect(buttons[1], x, y) == SOL_SUCCESS) { return buttons[1]; }
    }

    return SPRITE_ERROR;
}

int view_character_handle_mouse_movement(const uint32_t x, const uint32_t y) {
    static uint16_t last_sprite = SPRITE_ERROR;

    uint16_t cur_sprite = get_sprite_mouse_is_on(x, y);
    mousex = x; mousey = y;

    if (last_sprite != cur_sprite) {
        sol_sprite_increment_frame(cur_sprite, 1);
        if (last_sprite != SPRITE_ERROR) {
            sol_sprite_increment_frame(last_sprite, -1);
        }
    }
    
    last_sprite = cur_sprite;
    return 1;// handle
}

int view_character_handle_mouse_down(const uint32_t button, const uint32_t x, const uint32_t y) {
    sol_entity_t *player;
    for (int i = 0; i < 4; i++) {
        if (sol_sprite_in_rect(ports[i], x, y) == SOL_SUCCESS
                && sol_player_exists(i) == SOL_SUCCESS
                ) {
            player_selected = i;
            sol_player_get(player_selected, &player);
            strcpy(description, player->name);
        }
        if (sol_sprite_in_rect(leader[i], x, y) == SOL_SUCCESS) {
            sol_player_set_active(i);
        }
        if (sol_sprite_in_rect(ai[i], x, y) == SOL_SUCCESS) {
            sol_player_set_ai(i, sol_player_ai(i) == SOL_SUCCESS ? 0 : 1);
        }
    }
    return 1; // means I captured the mouse click
}

int view_character_handle_mouse_up(const uint32_t button, const uint32_t x, const uint32_t y) {
    if (power_to_display && sol_sprite_in_rect(power_background, x, y) == SOL_SUCCESS) {
        power_to_display = NULL;
        return 1;
    }

    if (button == SOL_MOUSE_BUTTON_RIGHT) {
        for (int i = 0; i < 4; i++) {
            if (sol_sprite_in_rect(ports[i], x, y) == SOL_SUCCESS) {
                slot_clicked = i;
                sol_window_push(&popup_window, 100, 75);
                sol_popup_set_message("INACTIVE CHARACTER");
                sol_popup_set_option(0, "NEW");
                sol_popup_set_option(1, "ADD");
                sol_popup_set_option(2, "CANCEL");
                last_selection = SELECT_POPUP;
                return 1;
            }
        }
        if (mode == 2) {
            if ((power_to_display = find_power(x, y))) {
                sol_description_set_message(power_to_display->description);
                sol_description_set_icon(power_to_display->icon_id);
                sol_window_push(&description_window, 0, 0);
                return 1;
            }
        }
    }
    if (button == SOL_MOUSE_BUTTON_LEFT) {
        if (mode == 2) {
            power_to_display = find_power(x, y);
            if (power_to_display) {
                sol_mouse_set_as_power(power_to_display);
                power_to_display = NULL;
                sol_window_pop();
                return 1;
            }
        }
    }

    sol_sprite_set_frame(character, 0);
    sol_sprite_set_frame(powers, 0);
    sol_sprite_set_frame(status, 0);

    if (sol_sprite_in_rect(game_return, x, y) == SOL_SUCCESS) { sol_window_pop(); return 1; } 
    if (sol_sprite_in_rect(character, x, y) == SOL_SUCCESS) { mode = 0; }
    if (sol_sprite_in_rect(inv, x, y) == SOL_SUCCESS) { sol_window_push(&inventory_window, 0, 0); return 1;}
    if (sol_sprite_in_rect(powers, x, y) == SOL_SUCCESS) { mode = 2; }
    if (sol_sprite_in_rect(status, x, y) == SOL_SUCCESS) { mode = 3; }

    if (mode == 2) {
        if (sol_sprite_in_rect(buttons[0], x, y) == SOL_SUCCESS) { return buttons[0]; }
        if (sol_sprite_in_rect(buttons[1], x, y) == SOL_SUCCESS) {
            level = (level + 1) % 6;
            if (level < 1) { level = 1; }
            set_power(0, level);
        }
    }

    return 1; // means I captured the mouse click
}

void view_character_free() {
    sol_status_check(sol_sprite_free(sun), "Unable to free sprite");
    sol_status_check(sol_sprite_free(panel), "Unable to free sprite");
    sol_status_check(sol_sprite_free(use), "Unable to free sprite");
    sol_status_check(sol_sprite_free(view_char), "Unable to free sprite");
    sol_status_check(sol_sprite_free(effects), "Unable to free sprite");

    for (int i = 0; i < 4; i++) {
        sol_status_check(sol_sprite_free(ai[i]), "Unable to free sprite");
        sol_status_check(sol_sprite_free(leader[i]), "Unable to free sprite");
        sol_status_check(sol_sprite_free(ports[i]), "Unable to free sprite");
    }
    for (int i = 0; i < 3; i++) {
        sol_status_check(sol_sprite_free(buttons[i]), "Unable to free sprite");
    }
    sol_status_check(sol_sprite_free(game_return), "Unable to free sprite");
    sol_status_check(sol_sprite_free(game_menu), "Unable to free sprite");
    sol_status_check(sol_sprite_free(character), "Unable to free sprite");
    sol_status_check(sol_sprite_free(inv), "Unable to free sprite");
    sol_status_check(sol_sprite_free(powers), "Unable to free sprite");
    sol_status_check(sol_sprite_free(status), "Unable to free sprite");
    sol_status_check(sol_sprite_free(power_highlight), "Unable to free sprite");
    sol_status_check(sol_sprite_free(power_background), "Unable to free sprite");
}

void view_character_return_control () {
    uint8_t sel;
    sol_label_group_set_font(FONT_YELLOW);
    if (last_selection == SELECT_POPUP) {
        if (sol_popup_get_selection(&sel) == SOL_SUCCESS && sel == POPUP_0) { // new
            sol_popup_clear_selection();
            sol_window_push(&new_character_window, 0, 0);
            last_selection = SELECT_NEW;
            return;
        }
        if (sol_popup_get_selection(&sel) == SOL_SUCCESS && sel == POPUP_1) { // ADD
            sol_popup_clear_selection();
            sol_add_load_save_set_mode(ACTION_ADD);
            sol_window_push(&als_window, 0, 0);
            last_selection = SELECT_ALS;
            return;
        }
    } else if (last_selection == SELECT_NEW) {
        sol_entity_t *pc;
        sol_new_character_get_pc(&pc);
        psin_t* psi;
        sol_new_character_get_psin(&psi);
        ssi_spell_list_t* spells;
        sol_new_character_get_spell_list(&spells);
        psionic_list_t* psionics;
        sol_new_character_get_psionic_list(&psionics);
        if (pc && psi && spells && psionics) {
            if (dnd2e_character_is_valid(pc)) {
                sol_player_set(slot_clicked, pc);
                sol_player_load(slot_clicked);
                warn ("TODO: Add back character creation to added list of chars!\n");
            } else {
                sol_window_push(&popup_window, 100, 75);
                sol_popup_set_message("Character was invalid.");
                sol_popup_set_option(0, "TRY AGAIN");
                sol_popup_set_option(1, "ADD");
                sol_popup_set_option(2, "CANCEL");
                last_selection = SELECT_POPUP;
                return;
            }
        }
    } else if (last_selection == SELECT_ALS) {
        if (sol_add_load_save_get_action() == ACTION_ADD) {
            uint32_t sel = sol_add_load_save_get_selection();
            //if (!ds_player_load_character_charsave(slot_clicked, sel)) {
            if (sol_load_character_charsave(slot_clicked, sel) != SOL_SUCCESS) {
                printf("Char loading failed.\n");
            } else {
                sol_player_load_zoom(slot_clicked, settings_zoom());
            }
        }
    }
    last_selection = SELECT_NONE;
}

sol_wops_t view_character_window = {
    .init = view_character_init,
    .cleanup = view_character_free,
    .render = view_character_render,
    .mouse_movement = view_character_handle_mouse_movement,
    .mouse_down = view_character_handle_mouse_down,
    .mouse_up = view_character_handle_mouse_up,
    .return_control = view_character_return_control,
    .name = "view",
    .grey_out_map = 1,
    .data = NULL
};
