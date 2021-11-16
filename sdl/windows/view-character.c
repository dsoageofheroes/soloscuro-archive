#include "view-character.h"
#include "../main.h"
#include "../../src/gff.h"
#include "../../src/gff-char.h"
#include "gfftypes.h"
#include "../../src/rules.h"
#include "../../src/settings.h"
#include "narrate.h"
#include "../player.h"
#include "inventory.h"
#include "new-character.h"
#include "popup.h"
#include "../mouse.h"
#include "../../src/dsl.h"
#include "add-load-save.h"
#include "../../src/player.h"
#include "../../src/ds-load-save.h"
#include "../font.h"
#include "../sprite.h"

// Sprites
static uint16_t ai[4];
static uint16_t leader[4];
static uint16_t ports[4];
static uint16_t buttons[3];
static label_t power_name, power_level;
static uint16_t effects, view_char, use, panel, sun;
static uint16_t character, inv, powers, status;
static uint16_t game_menu, game_return;
static uint16_t slots;
static uint16_t power_highlight, power_background;
enum {SELECT_NONE, SELECT_POPUP, SELECT_NEW, SELECT_ALS};
static int8_t last_selection = SELECT_NONE;
static uint8_t slot_clicked;
static int32_t player_selected = 0;
static uint32_t xoffset, yoffset;
static uint32_t mousex = 0, mousey = 0;
static int mode = 2; // 0 = char, 2 = powers, 3 = status
#define MAX_POWERS (50)
static power_t *power_list[MAX_POWERS];
static power_t *power_to_display = NULL;
static int level = 1;

static SDL_Rect initial_locs[] = {{ 155, 28, 0, 0 }, // description
                                  { 60, 155, 0, 0 }, // message
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
    label_set_text(&power_level, buf);
}

void view_character_init(const uint32_t _x, const uint32_t _y) {
    gff_palette_t *pal = open_files[RESOURCE_GFF_INDEX].pals->palettes + 0;
    SDL_Renderer * renderer = rend = main_get_rend();
    const float zoom = settings_zoom();
    uint32_t x = _x / settings_zoom(), y = _y / settings_zoom();
    xoffset = _x;
    yoffset = _y;
    player_selected = player_get_slot(player_get_active());

    memset(description, 0x0, sizeof(description));
    memset(message, 0x0, sizeof(message));
    memset(power_list, 0x0, sizeof(power_t*) * MAX_POWERS);

    panel = sprite_new(renderer, pal, 0 + x, 0 + y, zoom, RESOURCE_GFF_INDEX, GFF_BMP, 11000);
    effects = sprite_new(renderer, pal, 80 + x, 0 + y, zoom, RESOURCE_GFF_INDEX, GFF_BMP, 20075);
    view_char = sprite_new(renderer, pal, 60 + x, 0 + y, zoom, RESOURCE_GFF_INDEX, GFF_BMP, 20079);
    use = sprite_new(renderer, pal, 115 + x, 0 + y, zoom, RESOURCE_GFF_INDEX, GFF_BMP, 20080);
    sun = sprite_new(renderer, pal, 50 + x, 0 + y - 11, zoom, RESOURCE_GFF_INDEX, GFF_BMP, 20028);

    character = sprite_new(renderer, pal, 45 + x, 155 + y, zoom, RESOURCE_GFF_INDEX, GFF_ICON, 10100);
    inv = sprite_new(renderer, pal, 68 + x, 155 + y, zoom, RESOURCE_GFF_INDEX, GFF_ICON, 11102);
    powers = sprite_new(renderer, pal, 91 + x, 155 + y, zoom, RESOURCE_GFF_INDEX, GFF_ICON, 11103);
    status = sprite_new(renderer, pal, 112 + x, 155 + y, zoom, RESOURCE_GFF_INDEX, GFF_ICON, 11104);
    game_return = sprite_new(renderer, pal, 252 + x, 155 + y, zoom, RESOURCE_GFF_INDEX, GFF_ICON, 10108);
    game_menu = sprite_new(renderer, pal, 222 + x, 155 + y, zoom, RESOURCE_GFF_INDEX, GFF_ICON, 11101);
    slots = sprite_new(renderer, pal, 53 + x, 3 + y, zoom, RESOURCE_GFF_INDEX, GFF_BMP, 13007);
    power_highlight = sprite_new(renderer, pal, 53 + x, 3 + y, zoom, RESOURCE_GFF_INDEX, GFF_BMP, 20088);
    power_background = sprite_new(renderer, pal, 53 + x, 3 + y, zoom, RESOURCE_GFF_INDEX, GFF_BMP, 15002);

    ai[0] = sprite_new(renderer, pal, 45 + x, 40 + y, zoom, RESOURCE_GFF_INDEX, GFF_ICON, 11111);
    ai[1] = sprite_new(renderer, pal, 45 + x, 100 + y, zoom, RESOURCE_GFF_INDEX, GFF_ICON, 11111);
    ai[2] = sprite_new(renderer, pal, 95 + x, 40 + y, zoom, RESOURCE_GFF_INDEX, GFF_ICON, 11111);
    ai[3] = sprite_new(renderer, pal, 95 + x, 100 + y, zoom, RESOURCE_GFF_INDEX, GFF_ICON, 11111);
    leader[0] = sprite_new(renderer, pal, 45 + x, 30 + y, zoom, RESOURCE_GFF_INDEX, GFF_ICON, 11106);
    leader[1] = sprite_new(renderer, pal, 45 + x, 90 + y, zoom, RESOURCE_GFF_INDEX, GFF_ICON, 11106);
    leader[2] = sprite_new(renderer, pal, 95 + x, 30 + y, zoom, RESOURCE_GFF_INDEX, GFF_ICON, 11106);
    leader[3] = sprite_new(renderer, pal, 95 + x, 90 + y, zoom, RESOURCE_GFF_INDEX, GFF_ICON, 11106);
    ports[0] = sprite_new(renderer, pal, 55 + x, 30 + y, zoom, RESOURCE_GFF_INDEX, GFF_ICON, 11100);
    ports[1] = sprite_new(renderer, pal, 55 + x, 90 + y, zoom, RESOURCE_GFF_INDEX, GFF_ICON, 11100);
    ports[2] = sprite_new(renderer, pal, 105 + x, 30 + y, zoom, RESOURCE_GFF_INDEX, GFF_ICON, 11100);
    ports[3] = sprite_new(renderer, pal, 105 + x, 90 + y, zoom, RESOURCE_GFF_INDEX, GFF_ICON, 11100);
    buttons[0] = sprite_new(renderer, pal, 131 + x, 157 + y, zoom, RESOURCE_GFF_INDEX, GFF_ICON, 11109);
    buttons[1] = sprite_new(renderer, pal, 172 + x, 157 + y, zoom, RESOURCE_GFF_INDEX, GFF_ICON, 11110);
    buttons[2] = sprite_new(renderer, pal, 172 + x, 157 + y, zoom, RESOURCE_GFF_INDEX, GFF_ICON, 13006);
    power_name = create_label_at_pos(0, -1, "MAGE", FONT_GREY, (135 + x) * zoom, (158 + y) * zoom);
    power_level = create_label_at_pos(0, -1, "LEVEL 1", FONT_GREY, (175 + x) * zoom, (158 + y) * zoom);

    for (int i = 0; i < 4; i++) {
        sprite_set_frame(ports[i], 2);
    }

    description_loc = apply_params(initial_locs[0], x, y);
    message_loc.x = (75 + x) * zoom;
    message_loc.y = (175 + y) * zoom;
    message_loc.h = 8 * zoom;
    message_loc.w = 180 * zoom;

    set_zoom(&description_loc, zoom);

    strcpy(description, "description");
    if (player_get_active() && player_get_active()->name) {
        strcpy(description, player_get_active()->name);
    }
    strcpy(message, "message");
    label_create_group();
    label_group_set_font(FONT_YELLOW);
    label_set_positions(143 * zoom, 30 * zoom, SCREEN_VIEW_CHARACTER);

    set_power(0, level);
}

static void render_character(SDL_Renderer *renderer) {
    const float zoom = settings_zoom();

    if (!player_get(player_selected)) { return; }

    label_set_group(player_get(player_selected), SCREEN_VIEW_CHARACTER);
    label_set_positions(143 * zoom, 30 * zoom, SCREEN_VIEW_CHARACTER);
    label_render_stats(xoffset, yoffset);
    label_render_gra(xoffset + (64 * zoom), yoffset - (49 * zoom));
    label_render_class_and_combat(xoffset + (64 * zoom), yoffset - (42 * zoom));

    sprite_set_frame(slots, 0);
    item_t *items = player_get(player_selected)->inv;
    animate_sprite_t *as = NULL;
    sprite_set_location(slots, xoffset + (205 * zoom), yoffset + (41 * zoom));
    sprite_render(main_get_rend(), slots);
    if (items) {
        //as = items[2].sprite.data;
        as = &(items[2].anim);
        if (as) {
            sprite_center_spr(as->spr, slots);
            sprite_render(renderer, as->spr);
        }
    }

    sprite_set_location(slots, xoffset + (223 * zoom), yoffset + (41 * zoom));
    sprite_render(main_get_rend(), slots);
    if (items) {
        //as = items[3].sprite.data;
        as = &(items[3].anim);
        if (as) {
            sprite_center_spr(as->spr, slots);
            sprite_render(renderer, as->spr);
        }
    }

    sprite_set_location(slots, xoffset + (241 * zoom), yoffset + (41 * zoom));
    sprite_render(main_get_rend(), slots);
    if (items) {
        //as = items[10].sprite.data;
        as = &(items[10].anim);
        if (as) {
            sprite_center_spr(as->spr, slots);
            sprite_render(renderer, as->spr);
        }
    }
}

static power_t* find_power(const uint32_t x, const uint32_t y) {
    size_t power_pos = 0;
    while (power_list[power_pos]) {
        animate_sprite_t *as = &(power_list[power_pos]->icon);
        if (as && sprite_in_rect(as->spr, x, y)) {
            return power_list[power_pos];
        }
        power_pos++;
    }

    return NULL;
}

static void sprite_highlight(const uint32_t x, const uint32_t y) {
    const float zoom = settings_zoom();
    power_t *pw = find_power(x, y);

    if (!pw) { return; }

    animate_sprite_t *as = &(pw->icon);
    sprite_set_frame(power_highlight, 4);
    sprite_set_location(power_highlight, sprite_getx(as->spr) - 1 * zoom,
            sprite_gety(as->spr) - 1 * zoom);
    sprite_render(main_get_rend(), power_highlight);
}

static void render_powers(SDL_Renderer *renderer) {
    const float zoom = settings_zoom();
    sprite_render(renderer, buttons[0]);
    sprite_render(renderer, buttons[1]);

    label_render(&power_name, renderer);
    label_render(&power_level, renderer);

    for (int i = 0; i < MAX_POWERS && power_list[i]; i++) {
        animate_sprite_t *as = power_get_icon(power_list[i]);
        sprite_set_location(as->spr, xoffset + (170 + (i % 5) * 20) * zoom,
            yoffset + (42 + (i / 5) * 20) * zoom);
        sprite_render(renderer, as->spr);
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

static void render_power_to_display(SDL_Renderer *renderer) {
    const float zoom = settings_zoom();
    char *msg = power_to_display->description;
    int next_index = 0, pos = 0, amt = 20;
    int lines = 0;

    return;

    sprite_set_location(power_background, xoffset + (90 * zoom) , yoffset + (48 * zoom));
    sprite_render(main_get_rend(), power_background);
    animate_sprite_t *as = &(power_to_display->icon);
    sprite_set_location(as->spr, xoffset + (100) * zoom, yoffset + (58 * zoom));
    sprite_render(renderer, as->spr);

    while ((next_index = get_next_len(msg + pos, amt))) {
        print_line_len(renderer, FONT_BLACK, msg + pos, 
                (lines == 0) ? xoffset + (120 * zoom) : xoffset + (100 * zoom),
                yoffset + ((64 + 10 * lines) * zoom), next_index);
        pos += next_index;
        lines++;
        amt = 26;
    }
}

#define BUF_MAX (1<<12)

void view_character_render(void *data) {
    char buf[BUF_MAX];
    const float zoom = settings_zoom();
    SDL_Renderer *renderer = main_get_rend();

    sprite_render(renderer, sun);
    sprite_render(renderer, panel);
    sprite_render(renderer, view_char);

    sprite_set_frame((mode == 0) ? character
            : (mode == 2) ? powers
            : status, 3);

    sprite_render(renderer, character);
    sprite_render(renderer, inv);
    sprite_render(renderer, powers);
    sprite_render(renderer, status);
    sprite_render(renderer, game_menu);
    sprite_render(renderer, game_return);

    for (int i = 0; i < 4; i++) {
        sprite_set_frame(ai[i], 0);
        if (player_ai(i)) {
            sprite_set_frame(ai[i], 1);
        }
        sprite_render(renderer, ai[i]);
        sprite_set_frame(leader[i], 0);
        if (player_exists(i) && player_get(i) == player_get_active()) {
            sprite_set_frame(leader[i], 1);
        }
        sprite_render(renderer, leader[i]);
    }

    print_line_len(renderer, FONT_YELLOW, description, description_loc.x, description_loc.y, sizeof(description));
    //print_line_len(renderer, FONT_GREY, message, message_loc.x, message_loc.y, sizeof(message));
    font_render_center(renderer, FONT_GREY, message, message_loc);

    for (int i = 0; i < 4; i++) {
        if (player_exists(i)) {
            entity_t *player = player_get(i);
            int x = 56, y = 64;
            if (i == 1 || i == 3) { y += 60; }
            if (i == 2 || i == 3) { x += 50; }
            snprintf(buf, BUF_MAX, "%d/%d", player->stats.hp, player->stats.high_hp);
            print_line_len(renderer, FONT_YELLOW, buf, xoffset + x * zoom, yoffset + (y + 0) * zoom, BUF_MAX);
            snprintf(buf, BUF_MAX, "%d/%d", player->stats.psp, player->stats.high_psp);
            print_line_len(renderer, FONT_BLUE, buf, xoffset + x * zoom, yoffset + (y + 8) * zoom, BUF_MAX);
            if (player->combat_status) {
                snprintf(buf, BUF_MAX, "%d", player->combat_status);
            } else {
                snprintf(buf, BUF_MAX, "Okay");
            }
            print_line_len(renderer, FONT_YELLOW, buf, xoffset + x * zoom, yoffset + (y + 16) * zoom, BUF_MAX);
            sprite_set_frame(ports[i], (i == player_selected) ? 3 : 0);
            sprite_render(renderer, ports[i]);
            player_center(i, xoffset + x * zoom, yoffset + (y - 34) * zoom, 34 * zoom, 34 * zoom);
            uint16_t spr = player_get_sprite(i);
            if (sprite_geth(spr) > 30 * zoom) {
                sprite_set_frame(spr, 0);
                sprite_render_box(rend, spr, xoffset + x * zoom, yoffset + (y - 34) * zoom,
                    34 * zoom, 34 * zoom);
            } else {
                player_render(rend, i);
            }
        } else {
            sprite_set_frame(ports[i], 2);
            sprite_render(renderer, ports[i]);
        }
    }

    switch(mode) {
        case 0: render_character(renderer); break;
        case 2: render_powers(renderer); break;
        default: break;
    }
    return;

    if (power_to_display) {
        render_power_to_display(renderer);
    }
}

static int get_sprite_mouse_is_on(const uint32_t x, const uint32_t y) {
    if (sprite_in_rect(character, x, y)) { return character; }
    if (sprite_in_rect(inv, x, y)) { return inv; }
    if (sprite_in_rect(powers, x, y)) { return powers; }
    if (sprite_in_rect(status, x, y)) { return status; }
    if (sprite_in_rect(game_menu, x, y)) { return game_menu; }
    if (sprite_in_rect(game_return, x, y)) { return game_return; }
    //if (sprite_in_rect(effects, x, y)) { return effects; }

    for (int i = 0; i < 4; i++) {
        if (sprite_in_rect(ai[i], x, y)) { return ai[i]; }
        if (sprite_in_rect(leader[i], x, y)) { return leader[i]; }
        //if (sprite_in_rect(port_background[i], x, y)) { return port_background[i]; }
        if (sprite_in_rect(ports[i], x, y)) { return ports[i]; }
    }

    if (mode == 2) {
        if (sprite_in_rect(buttons[0], x, y)) { return buttons[0]; }
        if (sprite_in_rect(buttons[1], x, y)) { return buttons[1]; }
    }

    return SPRITE_ERROR;
}

int view_character_handle_mouse_movement(const uint32_t x, const uint32_t y) {
    static uint16_t last_sprite = SPRITE_ERROR;

    uint16_t cur_sprite = get_sprite_mouse_is_on(x, y);
    mousex = x; mousey = y;

    if (last_sprite != cur_sprite) {
        sprite_set_frame(cur_sprite, sprite_get_frame(cur_sprite) + 1);
        if (last_sprite != SPRITE_ERROR) {
            sprite_set_frame(last_sprite, sprite_get_frame(last_sprite) - 1);
        }
    }
    
    last_sprite = cur_sprite;
    return 1;// handle
}

int view_character_handle_mouse_down(const uint32_t button, const uint32_t x, const uint32_t y) {
    //static int32_t player_selected = 0;
    for (int i = 0; i < 4; i++) {
        if (sprite_in_rect(ports[i], x, y)
                && player_exists(i)
                ) {
            player_selected = i;
            strcpy(description, player_get(player_selected)->name);
        }
        if (sprite_in_rect(leader[i], x, y)) {
            player_set_active(i);
        }
        if (sprite_in_rect(ai[i], x, y)) {
            player_set_ai(i, !player_ai(i));
        }
    }
    return 1; // means I captured the mouse click
}

int view_character_handle_mouse_up(const uint32_t button, const uint32_t x, const uint32_t y) {
    if (power_to_display && sprite_in_rect(power_background, x, y)) {
        power_to_display = NULL;
        return 1;
    }

    if (button == SDL_BUTTON_RIGHT) {
        for (int i = 0; i < 4; i++) {
            if (sprite_in_rect(ports[i], x, y)) {
                slot_clicked = i;
                window_push(&popup_window, 100, 75);
                popup_set_message("INACTIVE CHARACTER");
                popup_set_option(0, "NEW");
                popup_set_option(1, "ADD");
                popup_set_option(2, "CANCEL");
                last_selection = SELECT_POPUP;
            }
        }
        if (mode == 2) {
            if ((power_to_display = find_power(x, y))) {
                strcpy(message, power_to_display->name);
            }
        }
    }
    if (button == SDL_BUTTON_LEFT) {
        if (mode == 2) {
            power_to_display = find_power(x, y);
            if (power_to_display) {
                mouse_set_as_power(power_to_display);
                power_to_display = NULL;
                window_pop();
                return 1;
            }
        }
    }

    sprite_set_frame(character, 0);
    sprite_set_frame(powers, 0);
    sprite_set_frame(status, 0);

    if (sprite_in_rect(game_return, x, y)) { window_pop(); return 1; } 
    if (sprite_in_rect(character, x, y)) { mode = 0; }
    if (sprite_in_rect(inv, x, y)) { window_push(&inventory_window, 0, 0); return 1;}
    if (sprite_in_rect(powers, x, y)) { mode = 2; }
    if (sprite_in_rect(status, x, y)) { mode = 3; }

    if (mode == 2) {
        if (sprite_in_rect(buttons[0], x, y)) { return buttons[0]; }
        if (sprite_in_rect(buttons[1], x, y)) {
            level = (level + 1) % 6;
            if (level < 1) { level = 1; }
            set_power(0, level);
        }
    }

    return 1; // means I captured the mouse click
}

void view_character_free() {
    sprite_free(sun);
    sprite_free(panel);
    sprite_free(use);
    sprite_free(view_char);
    sprite_free(effects);

    for (int i = 0; i < 4; i++) {
        sprite_free(ai[i]);
        sprite_free(leader[i]);
        sprite_free(ports[i]);
    }
    for (int i = 0; i < 3; i++) {
        sprite_free(buttons[i]);
    }
    sprite_free(game_return);
    sprite_free(game_menu);
    sprite_free(character);
    sprite_free(inv);
    sprite_free(powers);
    sprite_free(status);
    sprite_free(power_highlight);
    sprite_free(power_background);
}

void view_character_return_control () {
    label_group_set_font(FONT_YELLOW);
    if (last_selection == SELECT_POPUP) {
        if (popup_get_selection() == POPUP_0) { // new
            popup_clear_selection();
            window_push(&new_character_window, 0, 0);
            last_selection = SELECT_NEW;
            return;
        }
        if (popup_get_selection() == POPUP_1) { // ADD
            popup_clear_selection();
            add_load_save_set_mode(ACTION_ADD);
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
                warn ("TODO: Add back character creation!\n");
                //gff_char_add_character(pc, psi, spells, psionics, name);
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
            //if (!ds_player_load_character_charsave(slot_clicked, sel)) {
            if (!ds_load_character_charsave(slot_clicked, sel)) {
                printf("Char loading failed.\n");
            } else {
                player_load(slot_clicked, settings_zoom());
            }
        }
    }
    last_selection = SELECT_NONE;
}

wops_t view_character_window = {
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
