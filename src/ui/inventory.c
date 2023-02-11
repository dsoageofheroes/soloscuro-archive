#include "add-load-save.h"
#include "gfftypes.h"
#include "narrate.h"
#include "new-character.h"
#include "popup.h"
#include "view-character.h"
#include "mouse.h"
#include "port.h"
#include "gff.h"
#include "gff-char.h"
#include "rules.h"
#include "gpl.h"
#include "player.h"
#include "ds-load-save.h"
#include "settings.h"

#include <string.h>

// Sprites
static sol_sprite_t ai[4], leader[4], ports[4];
static sol_sprite_t flag_spear, parchment, frame_message, panel, slots;
static sol_sprite_t character, inv, magic, status;
static sol_sprite_t game_menu, game_return = SPRITE_ERROR;
enum {SELECT_NONE, SELECT_POPUP, SELECT_NEW, SELECT_ALS};
static int8_t last_selection = SELECT_NONE;
static uint8_t slot_clicked;
static uint32_t mousex, mousey;
static int char_selected;
static uint32_t xoffset, yoffset;

static sol_dim_t initial_locs[] = {{ 59, 5, 0, 0 }, // name
                                  { 52, 165, 174, 0 }, // description
};
static sol_dim_t slot_locs[] = {
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

static sol_dim_t name_loc, description_loc;
static char name[25];
static char description[128];

static void set_zoom(sol_dim_t *loc, float zoom) {
    loc->x *= zoom;
    loc->y *= zoom;
    loc->w *= zoom;
    loc->h *= zoom;
}

static sol_dim_t apply_params(const sol_dim_t rect, const uint16_t x, const uint16_t y) {
    sol_dim_t ret;
    ret.x = rect.x + x;
    ret.y = rect.y + y;
    ret.w = rect.w;
    ret.h = rect.h;
    return ret;
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

    memset(name, 0x0, sizeof(name));
    memset(description, 0x0, sizeof(description));

    sol_status_check(sol_sprite_new(pal, 0 + x, 0 + y, zoom, RESOURCE_GFF_INDEX, GFF_BMP, 13001, &panel),
            "Unable to create inv panel sprite");
    sol_status_check(sol_sprite_new(pal, 232 + x, 5 + y, zoom, RESOURCE_GFF_INDEX, GFF_BMP, 13004, &flag_spear),
            "Unable to create inv flag_spear sprite");
    sol_status_check(sol_sprite_new(pal, 75 + x, 36 + y, zoom, RESOURCE_GFF_INDEX, GFF_BMP, 13005, &parchment),
            "Unable to create inv parchment sprite");
    sol_status_check(sol_sprite_new(pal, 53 + x, 3 + y, zoom, RESOURCE_GFF_INDEX, GFF_BMP, 13000, &frame_message),
            "Unable to create inv frame_message sprite");
    sol_status_check(sol_sprite_new(pal, 53 + x, 3 + y, zoom, RESOURCE_GFF_INDEX, GFF_BMP, 13007, &slots),
            "Unable to create inv slots sprite");
    sol_sprite_set_frame(frame_message, 0);
    sol_sprite_set_location(frame_message, (53 + x) * zoom, (183 + y) * zoom);
    sol_sprite_set_frame(frame_message, 3);
    sol_sprite_set_location(frame_message, (45 + x) * zoom, (161 + y) * zoom);
    for (int i = 0; i < 23; i++) {
        sol_sprite_set_frame(slots, i);
        sol_sprite_set_location(slots, (x + slot_locs[i].x) * zoom, (y + slot_locs[i].y) * zoom);
    }

    sol_status_check(sol_sprite_new(pal, 160 + x, 181 + y, zoom, RESOURCE_GFF_INDEX, GFF_ICON, 10100, &character),
            "Unable to create inv character sprite");
    sol_status_check(sol_sprite_new(pal, 185 + x, 181 + y, zoom, RESOURCE_GFF_INDEX, GFF_ICON, 11102, &inv),
            "Unable to create inv inv sprite");
    sol_sprite_set_frame(inv, 3);
    sol_status_check(sol_sprite_new(pal, 210 + x, 181 + y, zoom, RESOURCE_GFF_INDEX, GFF_ICON, 11103, &magic),
            "Unable to create inv magic sprite");
    sol_status_check(sol_sprite_new(pal, 235 + x, 181 + y, zoom, RESOURCE_GFF_INDEX, GFF_ICON, 11104, &status),
            "Unable to create inv status sprite");
    sol_status_check(sol_sprite_new(pal, 288 + x, 181 + y, zoom, RESOURCE_GFF_INDEX, GFF_ICON, 10108, &game_return),
            "Unable to create inv game_return sprite");
    sol_status_check(sol_sprite_new(pal, 258 + x, 181 + y, zoom, RESOURCE_GFF_INDEX, GFF_ICON, 11101, &game_menu),
            "Unable to create inv game_menu sprite");
    sol_sprite_set_frame(game_menu, 2);

    sol_status_check(sol_sprite_new(pal, 2 + x, 14 + y, zoom, RESOURCE_GFF_INDEX, GFF_ICON, 11111, &ai[0]),
            "Unable to create inv ai sprite");
    sol_status_check(sol_sprite_new(pal, 2 + x, 62 + y, zoom, RESOURCE_GFF_INDEX, GFF_ICON, 11111, &ai[1]),
            "Unable to create inv ai sprite");
    sol_status_check(sol_sprite_new(pal, 2 + x, 110 + y, zoom, RESOURCE_GFF_INDEX, GFF_ICON, 11111, &ai[2]),
            "Unable to create inv ai sprite");
    sol_status_check(sol_sprite_new(pal, 2 + x, 158 + y, zoom, RESOURCE_GFF_INDEX, GFF_ICON, 11111, &ai[3]),
            "Unable to create inv ai sprite");
    sol_status_check(sol_sprite_new(pal, 2 + x, 4 + y, zoom, RESOURCE_GFF_INDEX, GFF_ICON, 11106, &leader[0]),
            "Unable to create inv leader sprite");
    sol_status_check(sol_sprite_new(pal, 2 + x, 52 + y, zoom, RESOURCE_GFF_INDEX, GFF_ICON, 11106, &leader[1]),
            "Unable to create inv leader sprite");
    sol_status_check(sol_sprite_new(pal, 2 + x, 100 + y, zoom, RESOURCE_GFF_INDEX, GFF_ICON, 11106, &leader[2]),
            "Unable to create inv leader sprite");
    sol_status_check(sol_sprite_new(pal, 2 + x, 148 + y, zoom, RESOURCE_GFF_INDEX, GFF_ICON, 11106, &leader[3]),
            "Unable to create inv leader sprite");
    sol_status_check(sol_sprite_new(pal, 12 + x, 4 + y, zoom, RESOURCE_GFF_INDEX, GFF_ICON, 11100, &ports[0]),
            "Unable to create inv ports sprite");
    sol_status_check(sol_sprite_new(pal, 12 + x, 52 + y, zoom, RESOURCE_GFF_INDEX, GFF_ICON, 11100, &ports[1]),
            "Unable to create inv ports sprite");
    sol_status_check(sol_sprite_new(pal, 12 + x, 100 + y, zoom, RESOURCE_GFF_INDEX, GFF_ICON, 11100, &ports[2]),
            "Unable to create inv ports sprite");
    sol_status_check(sol_sprite_new(pal, 12 + x, 148 + y, zoom, RESOURCE_GFF_INDEX, GFF_ICON, 11100, &ports[3]),
            "Unable to create inv ports sprite");

    for (int i = 0; i < 4; i++) {
        sol_sprite_set_frame(ports[i], 2);
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

static int do_slot_highlight(const uint16_t frame, const uint16_t pos) {
    sol_sprite_info_t info;
    if (sol_sprite_in_rect(slots, mousex, mousey) == SOL_SUCCESS) {
        sol_status_check(sol_sprite_get_info(slots, &info), "Unable to get slots sprite info");
        uint32_t x = info.x;
        uint32_t y = info.y;

        sol_sprite_set_frame(slots, 7); // highight

        sol_sprite_set_location(slots, x, y);
        sol_sprite_render(slots);
        sol_sprite_set_frame(slots, frame);
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
        sol_status_check(sol_print_line_len(FONT_YELLOW, item->name, xoffset + 235 * zoom, yoffset + ypos * zoom, BUF_MAX),
                "Unable to print.");
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

    sol_status_check(sol_print_line_len(FONT_YELLOW, buf, xoffset + 235 * zoom, yoffset + (ypos + offset) * zoom, BUF_MAX),
            "Unable to print.");
    return offset + 7;
}

static void render_character() {
    char buf[BUF_MAX];
    entity_t *player = sol_player_get(char_selected);
    if (!player) { return; }
    inventory_t *player_items = sol_player_get_inventory(char_selected);
    const float zoom = settings_zoom();

    if (player->name == NULL) { return; } // no character.
    strcpy(name, player->name);

    sol_status_check(sol_print_line_len(FONT_YELLOW, "STR:", xoffset + 235 * zoom, yoffset + 53 * zoom, BUF_MAX),
            "Unable to print.");
    sol_status_check(sol_print_line_len(FONT_YELLOW, "DEX:", xoffset + 235 * zoom, yoffset + 60 * zoom, BUF_MAX),
            "Unable to print.");
    sol_status_check(sol_print_line_len(FONT_YELLOW, "CON:", xoffset + 235 * zoom, yoffset + 67 * zoom, BUF_MAX),
            "Unable to print.");
    sol_status_check(sol_print_line_len(FONT_YELLOW, "INT:", xoffset + 235 * zoom, yoffset + 74 * zoom, BUF_MAX),
            "Unable to print.");
    sol_status_check(sol_print_line_len(FONT_YELLOW, "WIS:", xoffset + 235 * zoom, yoffset + 81 * zoom, BUF_MAX),
            "Unable to print.");
    sol_status_check(sol_print_line_len(FONT_YELLOW, "CHA:", xoffset + 235 * zoom, yoffset + 88 * zoom, BUF_MAX),
            "Unable to print.");
    snprintf(buf, BUF_MAX, "%d\n", player->stats.str);
    sol_status_check(sol_print_line_len(FONT_YELLOW, buf, xoffset + 260 * zoom, yoffset + 53 * zoom, BUF_MAX),
            "Unable to print.");
    snprintf(buf, BUF_MAX, "%d\n", player->stats.dex);
    sol_status_check(sol_print_line_len(FONT_YELLOW, buf, xoffset + 260 * zoom, yoffset + 60 * zoom, BUF_MAX),
            "Unable to print.");
    snprintf(buf, BUF_MAX, "%d\n", player->stats.con);
    sol_status_check(sol_print_line_len(FONT_YELLOW, buf, xoffset + 260 * zoom, yoffset + 67 * zoom, BUF_MAX),
            "Unable to print.");
    snprintf(buf, BUF_MAX, "%d\n", player->stats.intel);
    sol_status_check(sol_print_line_len(FONT_YELLOW, buf, xoffset + 260 * zoom, yoffset + 74 * zoom, BUF_MAX),
            "Unable to print.");
    snprintf(buf, BUF_MAX, "%d\n", player->stats.wis);
    sol_status_check(sol_print_line_len(FONT_YELLOW, buf, xoffset + 260 * zoom, yoffset + 81 * zoom, BUF_MAX),
            "Unable to print.");
    snprintf(buf, BUF_MAX, "%d\n", player->stats.cha);
    sol_status_check(sol_print_line_len(FONT_YELLOW, buf, xoffset + 260 * zoom, yoffset + 88 * zoom, BUF_MAX),
            "Unable to print.");

    snprintf(buf, BUF_MAX, "PSI:  %d/%d\n", player->stats.psp, player->stats.high_psp);
    sol_status_check(sol_print_line_len(FONT_YELLOW, buf, xoffset + 235 * zoom, yoffset + 100 * zoom, BUF_MAX),
            "Unable to print.");

    snprintf(buf, BUF_MAX, "AC: %d\n", dnd2e_calc_ac(player));
    sol_status_check(sol_print_line_len(FONT_YELLOW, buf, xoffset + 235 * zoom, yoffset + 115 * zoom, BUF_MAX),
            "Unable to print.");

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
    sol_sprite_set_frame(slots, 5);
    sol_sprite_set_location(slots, x, y);
    sol_sprite_render(slots);
    sol_sprite_set_frame(slots, frame);
    sol_sprite_set_location(slots, x, y);
    sol_sprite_render(slots);
    animate_sprite_t *as = items ? &(items[slot].anim) : NULL;

    if (as) {
        sol_sprite_center_spr(as->spr, slots);
        sol_sprite_render(as->spr);
    }

    do_slot_highlight(5, 14);

    if (item_allowed_in_slot(sol_mouse_get_item(), slot)) {
        sol_sprite_set_frame(slots, sol_sprite_in_rect(slots, mousex, mousey) == SOL_SUCCESS ? 8 : 4);
        sol_sprite_set_location(slots, x, y);
        sol_sprite_render(slots);
    }
}

void inventory_window_render(void *data) {
    const float zoom = settings_zoom();
    sol_sprite_info_t info;

    description[0] = '\0';
    entity_t *player = sol_player_get(char_selected);
    item_t *items = player ? player->inv : NULL;
    animate_sprite_t *as = NULL;

    sol_sprite_render(panel);
    sol_sprite_render(parchment);
    sol_sprite_render(flag_spear);
    sol_sprite_set_frame(frame_message, 0);
    sol_sprite_render(frame_message);
    sol_sprite_set_frame(frame_message, 1);
    sol_sprite_render(frame_message);
    sol_sprite_set_frame(frame_message, 3);
    sol_sprite_render(frame_message);

    sol_sprite_render(character);
    sol_sprite_render(inv);
    sol_sprite_render(magic);
    sol_sprite_render(status);
    sol_sprite_render(game_menu);
    sol_sprite_render(game_return);

    for (int i = 0; i < 4; i++) {
        sol_sprite_set_frame(ai[i], 0);
        if (sol_player_ai(i)) {
            sol_sprite_set_frame(ai[i], 1);
        }
        sol_sprite_set_frame(leader[i], 0);
        if (sol_player_exists(i) && sol_player_get(i) == sol_player_get_active()) {
            sol_sprite_set_frame(leader[i], 1);
        }
        sol_sprite_render(ai[i]);
        sol_sprite_render(leader[i]);
        //sol_sprite_render(port_background[i]);
        sol_sprite_set_frame(ports[i], sol_player_exists(i) ? 1 : 2);
        sol_sprite_render(ports[i]);
        if (sol_sprite_in_rect(ports[i], mousex, mousey) == SOL_SUCCESS) {
            sol_sprite_set_frame(ports[i], 1);
            sol_sprite_render(ports[i]);
            if (sol_player_exists(i)) {
                snprintf(description, 128, "%s%s",
                        i == char_selected ? "" : "SELECT ",
                        player->name);
            } else {
                strcpy(description, "PRESS RIGHT MOUSE BUTTON");
            }
        }
        if (char_selected == i) {
            sol_sprite_set_frame(ports[i], 3);
            sol_sprite_render(ports[i]);
        } else {
            sol_sprite_set_frame(ports[i], 0);
            sol_sprite_render(ports[i]);
        }
    }

    for (int i = 9; i < 23; i++) {
        as = items ? item_icon(items + i - 9) : NULL;
        sol_sprite_set_frame(slots, i);
        sol_status_check(sol_sprite_get_info(slots, &info), "Unable to get slots sprite info");
        int32_t x = info.x;
        int32_t y = info.y;
        if (as && as->spr != SPRITE_ERROR) {
            sol_sprite_set_frame(slots, 2);
            sol_sprite_set_location(slots, x, y);
            sol_sprite_render(slots);
            sol_sprite_center_spr(as->spr, slots);
            sol_sprite_render(as->spr);
        } else {
            sol_sprite_render(slots);
        }
        do_slot_highlight(i, i - 9);
        if (item_allowed_in_slot(sol_mouse_get_item(), i - 9)) {
            sol_sprite_set_frame(slots, sol_sprite_in_rect(slots, mousex, mousey) == SOL_SUCCESS ? 8 : 4);
            sol_sprite_set_location(slots, x, y);
            sol_sprite_render(slots);
        }
    }

    sol_sprite_set_frame(slots, 0);
    for (int i = 0; i < 6; i++) {
        render_backpack_slot(14 + 2 * i, 0, xoffset + 186 * zoom, yoffset + (18 + 18 * i) * zoom, items);
        render_backpack_slot(15 + 2 * i, 0, xoffset + 204 * zoom, yoffset + (18 + 18 * i) * zoom, items);
    }

    if (sol_sprite_in_rect(character, mousex, mousey) == SOL_SUCCESS) { strcpy(description, "VIEW CHARACTER"); }
    if (sol_sprite_in_rect(inv, mousex, mousey) == SOL_SUCCESS) { strcpy(description, "VIEW INVENTORY"); }
    if (sol_sprite_in_rect(magic, mousex, mousey) == SOL_SUCCESS) { strcpy(description, "CAST SPELL/USE PSIONIC"); }
    if (sol_sprite_in_rect(status, mousex, mousey) == SOL_SUCCESS) { strcpy(description, "CURRENT SPELL EFFECTS"); }
    if (sol_sprite_in_rect(game_return, mousex, mousey) == SOL_SUCCESS) { strcpy(description, "GAME RETURN"); }

    sol_status_check(sol_print_line_len(FONT_YELLOW, name, name_loc.x, name_loc.y, sizeof(name)),
            "Unable to print.");
    sol_font_render_center(FONT_GREY, description, description_loc.x, description_loc.y, description_loc.w);

    render_character();

    for (int i = 0; i < 4; i++) {
        sol_player_center(i, xoffset + 12 * zoom, yoffset + (4 + 48 * i) * zoom, 34 * zoom, 34 * zoom);
        sol_sprite_t spr = sol_player_get_sprite(i);
        sol_status_check(sol_sprite_get_info(spr, &info), "Unable to get sprite info.");
        if (info.h > 30 * zoom) {
            sol_sprite_set_frame(spr, 0);
            sol_sprite_render_box(spr, xoffset + 15 * zoom, yoffset + 11 + (48 * i) * zoom,
                30 * zoom, 30 * zoom);
        } else {
            sol_player_render(i);
        }
    }

    if (sol_player_exists(char_selected)) {
        sol_player_center_portrait(char_selected, xoffset + (75) * zoom, yoffset + (36) * zoom, 90 * zoom, 125 * zoom);
        sol_player_render_portrait(char_selected);
    }
}

static int get_sprite_mouse_is_on(const uint32_t x, const uint32_t y) {
    if (sol_sprite_in_rect(character, x, y) == SOL_SUCCESS) { return character; }
    if (sol_sprite_in_rect(magic, x, y) == SOL_SUCCESS) { return magic; }
    if (sol_sprite_in_rect(status, x, y) == SOL_SUCCESS) { return status; }
    if (sol_sprite_in_rect(game_return, x, y) == SOL_SUCCESS) { return game_return; }

    for (int i = 0; i < 4; i++) {
        if (sol_sprite_in_rect(ai[i], x, y) == SOL_SUCCESS) { return ai[i]; }
        if (sol_sprite_in_rect(leader[i], x, y) == SOL_SUCCESS) { return leader[i]; }
    }

    return SPRITE_ERROR;
}

int inventory_window_handle_mouse_movement(const uint32_t x, const uint32_t y) {
    static uint16_t last_sprite = SPRITE_ERROR;

    uint16_t cur_sprite = get_sprite_mouse_is_on(x, y);

    mousex = x;
    mousey = y;

    if (last_sprite != cur_sprite) {
        sol_sprite_increment_frame(cur_sprite, 1);
        if (last_sprite != SPRITE_ERROR) {
            sol_sprite_increment_frame(last_sprite, -1);
        }
    }
    
    last_sprite = cur_sprite;
    return 1;// handle
}

int inventory_window_handle_mouse_down(const uint32_t button, const uint32_t x, const uint32_t y) {
    for (int i = 0; i < 4; i++) {
        //if (sol_sprite_in_rect(ports[i], x, y)
                //&& sol_player_exists(i)
                //) {
            //player_selected = i;
            //strcpy(description, sol_player_get(player_selected)->name);
        //}
        if (sol_sprite_in_rect(leader[i], x, y) == SOL_SUCCESS) {
            sol_player_set_active(i);
        }
        if (sol_sprite_in_rect(ai[i], x, y) == SOL_SUCCESS) {
            sol_player_set_ai(i, !sol_player_ai(i));
        }
    }
    return 1; // means I captured the mouse click
}

static void clicked_slot(const int slot) {
    item_t* mouse_item = sol_mouse_get_item();
    entity_t *player = sol_player_get(char_selected);
    item_t *player_item = player->inv + slot;

    if (mouse_item) { // mouse has an item
        if (!item_allowed_in_slot(sol_mouse_get_item(), slot)) { return; }
        if (player_item->ds_id) { // If we are doing a swap
            // Order matters.
            player_item = item_dup(player_item);
            sol_entity_copy_item(player, mouse_item, slot);
            sol_mouse_set_as_item(player_item);
            item_free_except_graphics(player_item);
        } else {
            sol_entity_copy_item(player, mouse_item, slot);
            sol_mouse_free_item();
        }
    } else if (player_item->ds_id) {
        sol_mouse_set_as_item(player->inv + slot);
        sol_entity_clear_item(player, slot);
    }
}

int inventory_window_handle_mouse_up(const uint32_t button, const uint32_t x, const uint32_t y) {
    const float zoom = settings_zoom();

    for (int i = 0; i < 4; i++) {
        if (sol_sprite_in_rect(ports[i], x, y) == SOL_SUCCESS) {
            if (button == SOL_MOUSE_BUTTON_RIGHT) {
                slot_clicked = i;
                sol_window_push(&popup_window, 100, 75);
                sol_popup_set_message("INACTIVE CHARACTER");
                sol_popup_set_option(0, "NEW");
                sol_popup_set_option(1, "ADD");
                sol_popup_set_option(2, "CANCEL");
                last_selection = SELECT_POPUP;
            } else if (button == SOL_MOUSE_BUTTON_LEFT) {
                if (sol_player_exists(i)) {
                    char_selected = i;
                }
            }
        }
    }

    if (sol_sprite_in_rect(game_return, x, y) == SOL_SUCCESS) {
        sol_window_pop();
        return 1;
    } 

    if (sol_sprite_in_rect(character, x, y) == SOL_SUCCESS) {
        sol_window_push(&view_character_window, 0, 10);
        return 1;
    } 

    for (int i = 9; i < 23; i++) {
        sol_sprite_set_frame(slots, i);
        if (sol_sprite_in_rect(slots, x, y) == SOL_SUCCESS) {
            clicked_slot(i-9);
        }
    }

    sol_sprite_set_frame(slots, 5);
    for (int i = 0; i < 6; i++) {
        sol_sprite_set_location(slots, xoffset + 186 * zoom, yoffset + (18 + 18 * i) * zoom);
        if (sol_sprite_in_rect(slots, x, y) == SOL_SUCCESS) {
            clicked_slot(14 + 2 * i);
        }
        sol_sprite_set_location(slots, xoffset + 204 * zoom, yoffset + (18 + 18 * i) * zoom);
        if (sol_sprite_in_rect(slots, x, y) == SOL_SUCCESS) {
            clicked_slot(15 + 2 * i);
        }
    }

    return 1; // means I captured the mouse click
}

void inventory_window_free() {
    sol_status_check(sol_sprite_free(panel), "Unable to free sprite");
    sol_status_check(sol_sprite_free(frame_message), "Unable to free sprite");
    sol_status_check(sol_sprite_free(parchment), "Unable to free sprite");
    sol_status_check(sol_sprite_free(flag_spear), "Unable to free sprite");

    for (int i = 0; i < 4; i++) {
        sol_status_check(sol_sprite_free(ai[i]), "Unable to free sprite");
        sol_status_check(sol_sprite_free(leader[i]), "Unable to free sprite");
        sol_status_check(sol_sprite_free(ports[i]), "Unable to free sprite");
    }
    sol_status_check(sol_sprite_free(game_return), "Unable to free sprite");
    sol_status_check(sol_sprite_free(game_menu), "Unable to free sprite");
    sol_status_check(sol_sprite_free(character), "Unable to free sprite");
    sol_status_check(sol_sprite_free(inv), "Unable to free sprite");
    sol_status_check(sol_sprite_free(magic), "Unable to free sprite");
    sol_status_check(sol_sprite_free(status), "Unable to free sprite");
    sol_status_check(sol_sprite_free(slots), "Unable to free sprite");
}

void inventory_window_return_control () {
    if (last_selection == SELECT_POPUP) {
        if (sol_popup_get_selection() == POPUP_0) { // new
            sol_popup_clear_selection();
            sol_window_push(&new_character_window, 0, 0);
            last_selection = SELECT_NEW;
            return;
        }
        if (sol_popup_get_selection() == POPUP_1) { // ADD
            sol_popup_clear_selection();
            sol_window_push(&als_window, 0, 0);
            last_selection = SELECT_ALS;
            return;
        }
    } else if (last_selection == SELECT_NEW) {
        entity_t *pc = sol_new_character_get_pc();
        psin_t* psi = sol_new_character_get_psin();
        ssi_spell_list_t* spells = sol_new_character_get_spell_list();
        psionic_list_t* psionics = sol_new_character_get_psionic_list();
        //char *name = new_character_get_name();
        if (pc && psi && spells && psionics) {
            if (dnd2e_character_is_valid(pc)) {// && dnd2e_psin_is_valid(pc, psi)) {
                warn ("TODO: PUT BACK IN CHARACTER ADDING!\n");
                //gff_char_add_character(pc, psi, spells, psionics, name);
                sol_player_load_zoom(slot_clicked, settings_zoom());
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
            if (sol_load_character_charsave(slot_clicked, sel) != SOL_SUCCESS) {
                printf("Char loading failed.\n");
            } else {
                sol_player_load_zoom(slot_clicked, settings_zoom());
                char_selected = slot_clicked;
            }
        }
    }
    last_selection = SELECT_NONE;
}

sol_wops_t inventory_window = {
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
