#include "gff.h"
#include "gfftypes.h"
#include "settings.h"
#include "ssi-gui.h"

#include <stdlib.h>
#include <string.h>

static void apply_overlay(sol_window_t *win);

static int load_button_from_gff(const int res_id, sol_button_t *button) {
    char buf[4096];
    ssi_button_t *ssi_button;
    int pal_idx = gff_get_game_type() == DARKSUN_ONLINE
        ? RESFLOP_GFF_INDEX
        : RESOURCE_GFF_INDEX;

    if (!button) { return 0; }

    memset(button, 0x0, sizeof(sol_button_t));
    button->spr = SPRITE_ERROR;
    gff_chunk_header_t chunk = gff_find_chunk_header(RESOURCE_GFF_INDEX, GFF_BUTN, res_id);
    if (!gff_read_chunk(RESOURCE_GFF_INDEX, &chunk, buf, 4096)) {
        return 0;
    }

    ssi_button = (ssi_button_t*)buf;
    button->ssi_id = ssi_button->rh.id;
    button->icon_id = ssi_button->icon_id;
    button->base_width = ssi_button->frame.width;
    button->base_height = ssi_button->frame.height;

    //render_entry_as_image(gff_idx, GFF_ICON, button->icon_id, open_files[pal_idx].pals->palettes, 320, 92);
    sol_status_check(sol_sprite_new(open_files[pal_idx].pals->palettes, 0, 0, settings_zoom(), RESOURCE_GFF_INDEX, GFF_ICON, button->icon_id, &button->spr),
        "Unable to load button's sprite.");

    return 1;
}

static int load_frame_from_gff(const int res_id, sol_frame_t *frame) {
    char buf[4096];
    ssi_app_frame_t *ssi_frame;

    if (!frame) { return 0; }

    memset(frame, 0x0, sizeof(sol_frame_t));
    frame->spr = SPRITE_ERROR;
    gff_chunk_header_t chunk = gff_find_chunk_header(RESOURCE_GFF_INDEX, GFF_APFM, res_id);
    if (!gff_read_chunk(RESOURCE_GFF_INDEX, &chunk, buf, 4096)) {
        return 0;
    }

    ssi_frame = (ssi_app_frame_t*)buf;
    frame->ssi_id = ssi_frame->rh.id;
    frame->base_width = ssi_frame->frame.width;
    frame->base_height = ssi_frame->frame.height;
    frame->event = ssi_frame->event_filter;
    //frame->icon_id = ssi_frame->icon_id;
    //frame->base_width = ssi_frame->frame.width;
    //frame->base_height = ssi_frame->frame.height;

    //render_entry_as_image(gff_idx, GFF_ICON, button->icon_id, open_files[pal_idx].pals->palettes, 320, 92);

    return 1;
}

static int load_box_from_gff(const int res_id, sol_box_t *box) {
    char buf[4096];
    ssi_ebox_t *ssi_box;
    int pal_idx = gff_get_game_type() == DARKSUN_ONLINE
        ? RESFLOP_GFF_INDEX
        : RESOURCE_GFF_INDEX;

    if (!box) { return 0; }

    memset(box, 0x0, sizeof(sol_box_t));
    box->spr = SPRITE_ERROR;
    gff_chunk_header_t chunk = gff_find_chunk_header(RESOURCE_GFF_INDEX, GFF_EBOX, res_id);
    if (!gff_read_chunk(RESOURCE_GFF_INDEX, &chunk, buf, 4096)) {
        return 0;
    }

    ssi_box = (ssi_ebox_t*)buf;
    box->ssi_id = ssi_box->rh.id;
    box->base_width = ssi_box->frame.width;
    box->base_height = ssi_box->frame.height;
    box->bmp_id = ssi_box->frame.border_bmp;
    box->base_width = ssi_box->frame.width;
    box->base_height = ssi_box->frame.height;

    sol_status_check(sol_sprite_new(open_files[pal_idx].pals->palettes, 0, 0, settings_zoom(), RESOURCE_GFF_INDEX, GFF_BMP, box->bmp_id, &box->spr),
        "Unable to load box's sprite.");

    return 1;
}

extern sol_window_t *sol_window_from_gff(const int res_id) {
    sol_window_t *ret = NULL;
    ssi_window_t *ssi_wind = NULL;
    char          buf[4096];

    gff_chunk_header_t chunk = gff_find_chunk_header(RESOURCE_GFF_INDEX, GFF_WIND, res_id);
    if (!gff_read_chunk(RESOURCE_GFF_INDEX, &chunk, buf, 4096)) {
        return NULL;
    }

    ssi_wind = (ssi_window_t*) buf;
    ret = calloc(1, sizeof(sol_window_t));
    ret->ssi_id = ssi_wind->rh.id;
    ret->base_width = ssi_wind->frame.width;
    ret->base_height = ssi_wind->frame.height;
    ret->startx = ssi_wind->x;
    ret->starty = ssi_wind->y;

    for (int i = 0; i < ssi_wind->itemCount; i++) {
        ssi_gui_item_t *item = (ssi_gui_item_t*)(buf + sizeof(ssi_window_t) + 12 + i *(sizeof(ssi_gui_item_t)));
        printf("item (0x%x): %d, ", item->type, item->id);
        switch (item->type) {
            case 0x4c434341:
                printf("ACCL");
                break;
            case 0x4d465041:
                printf("APFM");
                ret->num_frames++;
                ret->frames = realloc(ret->frames, sizeof(sol_frame_t) * ret->num_frames);
                load_frame_from_gff(item->id, ret->frames + (ret->num_frames - 1));
                ret->frames[ret->num_frames - 1].offsetx = item->init_bounds.xmin;
                ret->frames[ret->num_frames - 1].offsety = item->init_bounds.ymin;
                break;
            case 0x4e545542:
                ret->num_buttons++;
                ret->buttons = realloc(ret->buttons, sizeof(sol_button_t) * ret->num_buttons);
                load_button_from_gff(item->id, ret->buttons + (ret->num_buttons - 1));
                ret->buttons[ret->num_buttons - 1].offsetx = item->init_bounds.xmin;
                ret->buttons[ret->num_buttons - 1].offsety = item->init_bounds.ymin;
                printf("BUTTON: %d", ret->buttons[ret->num_buttons - 1].icon_id);
                break;
            case 0x584f4245:
                printf("EBOX!");
                ret->num_boxes++;
                ret->boxes = realloc(ret->boxes, sizeof(sol_button_t) * ret->num_boxes);
                load_box_from_gff(item->id, ret->boxes + (ret->num_boxes - 1));
                ret->boxes[ret->num_boxes - 1].offsetx = item->init_bounds.xmin;
                ret->boxes[ret->num_boxes - 1].offsety = item->init_bounds.ymin;
                break;
        }
        printf("\n");
    }
    apply_overlay(ret);

    return ret;
}

extern void sol_window_free_base(sol_window_t *win) {
    if (!win) { return; }

    if (win->buttons) {
        for (int i = 0; i < win->num_buttons; i++) {
            sol_status_check(sol_sprite_free(win->buttons[i].spr), "Unable to free sprite");
            if (win->buttons[i].text) { free(win->buttons[i].text); }
        }
        free(win->buttons);
        win->buttons = NULL;
        win->num_buttons = 0;
    }

    if (win->frames) {
        for (int i = 0; i < win->num_frames; i++) {
            sol_status_check(sol_sprite_free(win->frames[i].spr), "Unable to free sprite");
        }
        free(win->frames);
        win->frames = NULL;
        win->num_frames = 0;
    }

    if (win->boxes) {
        for (int i = 0; i < win->num_boxes; i++) {
            sol_status_check(sol_sprite_free(win->boxes[i].spr), "Unable to free sprite");
        }
        free(win->boxes);
        win->boxes = NULL;
        win->num_boxes = 0;
    }

    if (win->underlays) {
        for (int i = 0; i < win->num_underlays; i++) {
            sol_status_check(sol_sprite_free(win->underlays[i].spr), "Unable to free sprite");
        }
        free(win->underlays);
        win->underlays = NULL;
        win->num_underlays = 0;
    }

    free(win);
}

extern size_t sol_window_get_button(sol_window_t *win, const uint32_t x, const uint32_t y) {
    if (!win) { return -1; }
    sol_sprite_info_t info;

    for (int i = 0; i < win->num_buttons; i++) {
        sol_status_check(sol_sprite_get_info(win->buttons[i].spr, &info), "Unable to get window button sprite info");
        uint32_t sx = info.x;
        uint32_t sy = info.y;
        uint32_t sw = info.w;
        uint32_t sh = info.h;
        //printf("x = %d, @(%d, %d) (%d x %d)\n", x, sx, sy, sw, sh);
        if (sx <= x && (sx + sw) >= x && sy <= y && (sy + sh) >= y) {
            return i;
        }
    }

    return -1;
}

extern void sol_window_set_pos(sol_window_t *win, const int x, const int y) {
    if (!win) { return; }
    const float zoom = settings_zoom();

    for (int i = 0; i < win->num_buttons; i++) {
        sol_sprite_set_location(win->buttons[i].spr, win->buttons[i].offsetx * zoom + x, win->buttons[i].offsety * zoom + y);
    }
    for (int i = 0; i < win->num_boxes; i++) {
        sol_sprite_set_location(win->boxes[i].spr, win->boxes[i].offsetx * zoom + x, win->boxes[i].offsety * zoom + y);
    }
    for (int i = 0; i < win->num_underlays; i++) {
        sol_sprite_set_location(win->underlays[i].spr, win->underlays[i].offsetx * zoom + x, win->underlays[i].offsety * zoom + y);
    }
}

extern void sol_window_render_base(sol_window_t *win) {
    sol_sprite_info_t info;
    if (!win) { return; }

    for (int i = 0; i < win->num_underlays; i++) {
        sol_sprite_render(win->underlays[i].spr);
    }

    for (int i = 0; i < win->num_boxes; i++) {
        sol_sprite_render(win->boxes[i].spr);
    }

    for (int i = 0; i < win->num_buttons; i++) {
        if (win->buttons[i].disabled) { continue; }
        sol_sprite_render(win->buttons[i].spr);
        if (win->buttons[i].text) {
            sol_status_check(sol_sprite_get_info(win->buttons[i].spr, &info), "Unable to get window button sprite info");
            if (i == 0) { printf("%d:'%s' (%d x %d) %d\n", i, win->buttons[i].text,
                info.x,
                info.y,
                win->buttons[i].offsetx
                ); }
            sol_print_line_len(win->buttons[i].font, win->buttons[i].text,
                info.x + 2 * settings_zoom(),
                info.y + 2 * settings_zoom(),
                32);
        }
    }
}

extern void sol_button_set_font(sol_button_t *button, const sol_font_t font) {
    if (!button) { return; }
    button->font = font;
}

extern void sol_button_set_text(sol_button_t *button, const char *text, const sol_font_t font) {
    if (!button) { return; }
    if (button->text) { free(button->text); }
    button->text = strdup(text);
    sol_button_set_font(button, font);
}

extern int sol_button_is_empty(sol_button_t *button) {
    return (!button || !button->text || !button->text[0]) ? 1 : 0;
}

extern void sol_button_set_enabled(sol_button_t *button, const int val) {
    if (!button) { return; }
    button->disabled = !val;
}

static void load_frame_bmp(sol_frame_t *frame, int res_id) {
    sol_sprite_info_t info;
    int pal_idx = gff_get_game_type() == DARKSUN_ONLINE
        ? RESFLOP_GFF_INDEX
        : RESOURCE_GFF_INDEX;

    if (!frame) { return; }

    memset(frame, 0x0, sizeof(sol_frame_t));
    frame->spr = SPRITE_ERROR;
    frame->bmp_id = res_id;
    sol_status_check(sol_sprite_new(open_files[pal_idx].pals->palettes, 0, 0, settings_zoom(), RESOURCE_GFF_INDEX, GFF_BMP, frame->bmp_id, &frame->spr),
        "Unable to load frame's sprite");
    sol_status_check(sol_sprite_get_info(frame->spr, &info), "Unable to get frame sprite info");
    frame->base_width = info.w;
    frame->base_height = info.h;
}

static void add_underlay(sol_window_t *win, const int res_id, const int x, const int y) {
    win->num_underlays += 1;
    win->underlays = realloc(win->underlays, sizeof(sol_frame_t) * win->num_underlays);

    load_frame_bmp(win->underlays + win->num_underlays - 1, res_id);
    win->underlays[win->num_underlays - 1].offsetx = x;
    win->underlays[win->num_underlays - 1].offsety = y;
}

static void main_menu_setup(sol_window_t *win) {
    add_underlay(win, 20028, 47, 25);
    add_underlay(win, 20029, 4, 45);
}

static void narrate_top_setup(sol_window_t *win) {
    add_underlay(win, 3007, 0, 0);
    sol_sprite_set_alpha(win->underlays[0].spr, 128);
    sol_sprite_set_alpha(win->boxes[0].spr, 0);
}

static void narrate_bottom_setup(sol_window_t *win) {
    add_underlay(win, 3007, 0, 0);
    sol_sprite_set_alpha(win->underlays[0].spr, 128);
    sol_sprite_set_alpha(win->buttons[1].spr, 0);
    sol_sprite_set_alpha(win->buttons[2].spr, 0);
    sol_sprite_set_alpha(win->buttons[3].spr, 0);
    sol_sprite_set_alpha(win->buttons[4].spr, 0);
    sol_sprite_set_alpha(win->buttons[5].spr, 0);
}

static void save_menu_setup(sol_window_t *win) {
    add_underlay(win, 3009, 0, 0);
}

static void save_delete_menu_setup(sol_window_t *win) {
    add_underlay(win, 3009, 0, 0);
}

static void psi_gen_setup(sol_window_t *win) {
    add_underlay(win, 20087, 0, 0);
}

static void sphere_gen_setup(sol_window_t *win) {
    add_underlay(win, 20087, 0, 0);
}

static void examine0_gen_setup(sol_window_t *win) {
    add_underlay(win, 3020, 0, 0);
}

static void examine1_gen_setup(sol_window_t *win) {
    add_underlay(win, 15000, 0, 0);
}

static void game_menu_setup(sol_window_t *win) {
    add_underlay(win, 10000, 0, 0);
}

static void game_view_character(sol_window_t *win) {
    add_underlay(win, 20028, 47, 0);
    add_underlay(win, 11000, 0, 0);
}

static void inventory_setup(sol_window_t *win) {
    add_underlay(win, 13001, 0, 0);
    // 13007 is button
}

static void contianer_setup(sol_window_t *win) {
    add_underlay(win, 13002, 0, 0);
    //add_underlay(win, 13003, 0, 0);
    // 20088 is button
    // 13007 is button
}

static void alert_setup(sol_window_t *win) {
    add_underlay(win, 14000, 0, 0);
    //13007 is button
}

static void quick_cast_alert(sol_window_t *win) {
    (void)win; // I don't know the background.
    //add_underlay(win, 14000, 0, 0);
    //13007 is button
}

static void view_item_setup(sol_window_t *win) {
    add_underlay(win, 15001, 0, 0);
}

static void view_spell_setup(sol_window_t *win) {
    add_underlay(win, 15002, 0, 0);
    // the ebox has no background.
}

static void preference_setup(sol_window_t *win) {
    add_underlay(win, 10000, 0, 0);
    // Music (75, 26) probably 127 length.
    // Sound Effect (75, 45) total 127
    // difficulty (65, 64) total {Easy, Normal, Hard, Very Hard}
}

static void training_setup(sol_window_t *win) {
    add_underlay(win, 17000, 0, 0);
    //write text "CHOOSE A SPELL,<char name>" at Y=81
    // button 17300 has "LEVEL <char level>"
    // frames are the spell selections.
}

static void psi_training_setup(sol_window_t *win) {
    add_underlay(win, 17001, 0, 0);
    //write text "PICK A PSI POWER,<char name>" at Y=81
    //button 11319 has <char level>
    // button 11320 has discipline
    // frames are the psi selections.
}

static void dual_training_setup(sol_window_t *win) {
    add_underlay(win, 17002, 0, 0);
    // frames are the psi selections.
    // Ya, look...
}

static void apply_overlay(sol_window_t *win) {
    if (!win) { return; }

    switch (win->ssi_id) {
        case 3000: main_menu_setup(win); break;
        case 3001: /* Nothing: game events */ break;
        case 3004: /* Nothing? looks like death sequence */ break;
        case 3007: narrate_top_setup(win); break;
        case 3008: narrate_bottom_setup(win); break;
        case 3009: save_menu_setup(win); break;
        case 3012: psi_gen_setup(win); break;
        case 3013: sphere_gen_setup(win); break;
        case 3020: examine0_gen_setup(win); break;
        case 3024: save_delete_menu_setup(win); break;
        case 3500: /* IDK */ break;
        case 10500: game_menu_setup(win); break;
        case 10501: /* Nothing: This is an alert box. */ break;
        case 11500: game_view_character(win); break;
        case 13500: inventory_setup(win); break;
        case 13501: contianer_setup(win); break;
        case 14000: alert_setup(win); break;
        case 14001: alert_setup(win); break;
        case 14002: quick_cast_alert(win); break;
        case 15500: examine1_gen_setup(win); break;
        case 15502: view_item_setup(win); break;
        case 15503: view_spell_setup(win); break;
        case 16500: preference_setup(win); break;
        case 17500: training_setup(win); break;
        case 17501: psi_training_setup(win); break;
        case 17502: dual_training_setup(win); break;
    }
}
