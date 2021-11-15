#include "narrate.h"
#include "utils.h"
#include "sprite.h"
#include "../../src/dsl.h"
#include "../../src/ds-narrate.h"
#include "../../src/entity-animation.h"
#include "../../src/gff.h"
#include "../../src/gfftypes.h"
#include "../../src/gameloop.h"
#include "../font.h"
#include "../main.h"
#include "../../src/settings.h"
#include <string.h>
#include <ctype.h>

#define STARTX (60)
#define MAX_PORTRAITS (256)
#define MAX_TEXT (4096)
#define MAX_LINE (128)
#define MAX_OPTIONS (32)

static uint16_t background;
static uint32_t xoffset, yoffset;
static uint16_t border;

static SDL_Texture *portraits[MAX_PORTRAITS];
static SDL_Rect portraits_loc[MAX_PORTRAITS];

static int display = 0;
static int display_menu = 0;
static int end_received = 0, close_received = 0;
static uint32_t portrait_index = 0;
static char narrate_text[MAX_TEXT];
static size_t text_pos = 0;
static int menu_pos = 0;
static char menu_options[MAX_OPTIONS][MAX_LINE];

static void clear() {
    int i;
    display = 0;
    display_menu = 0;
    text_pos = 0;
    end_received = 0;
    close_received = 0;
    menu_pos = 0;
    for (i = 0; i < MAX_OPTIONS; i++) {
        menu_options[i][0] = '\0';
    }
}

int narrate_is_open() { return display; }

#define PORT_MAX (1<<14)
void load_portraits(SDL_Renderer *renderer) {
    unsigned char *data;
    unsigned char buf[PORT_MAX];
    unsigned int w, h, id;
    SDL_Surface *surface = NULL;
    unsigned int *ids = gff_get_id_list(DSLDATA_GFF_INDEX, GFF_PORT);
    unsigned int num_ids = gff_get_resource_length(DSLDATA_GFF_INDEX, GFF_PORT);
    memset(portraits_loc, 0x0, sizeof(SDL_Rect) * MAX_PORTRAITS);
    memset(portraits, 0x0, sizeof(SDL_Texture*) * MAX_PORTRAITS);
    memset(narrate_text, 0x0, sizeof(char) * MAX_TEXT);
    memset(menu_options, 0x0, sizeof(char) * MAX_LINE * MAX_OPTIONS);

    for (int i = 0; i < num_ids; i++) {
        id = ids[i];
        gff_chunk_header_t chunk = gff_find_chunk_header(DSLDATA_GFF_INDEX, GFF_PORT, id);
        if (chunk.length > PORT_MAX) {
            error ("chunk.length (%d) is greater than PORT_MAX (%d)\n", chunk.length, PORT_MAX);
            exit (1);
        }
        gff_read_chunk(DSLDATA_GFF_INDEX, &chunk, buf, chunk.length);
        data = get_portrait(buf, &w, &h);
        surface = SDL_CreateRGBSurfaceFrom(data, w, h, 32,
            4*w, 0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);
        portraits[id] = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_FreeSurface(surface);
        free(data);
        portraits_loc[id].w = 2 * w;
        portraits_loc[id].h = 2 * h;
        portraits_loc[id].x = STARTX + 26 + 20; // fix this
        portraits_loc[id].y = 24;
    }

    if (ids) { free(ids); }
}

void narrate_init(const uint32_t x, const uint32_t y) {
    SDL_Renderer *renderer = main_get_rend();
    const float zoom = settings_zoom();
    xoffset = 0;//x / zoom;
    yoffset = 0;//y / zoom;
    gff_palette_t *pal = open_files[RESOURCE_GFF_INDEX].pals->palettes + 0;

    background = sprite_new(renderer, pal, 0 + xoffset, 0 + yoffset, zoom,
            RESOURCE_GFF_INDEX, GFF_BMP, 3007);
    sprite_set_location(background, sprite_getx(background) + (main_get_width() - sprite_getw(background)) / 2, sprite_gety(background));
    border = sprite_new(renderer, pal, 0, 0, zoom, RESOURCE_GFF_INDEX, GFF_BMP, 12000);
    sprite_set_location(border,
            sprite_getx(background) + 5 * zoom,
            sprite_gety(background) + 5 * zoom);
    load_portraits(renderer);
    sprite_set_alpha(background, 192);
    display = 0; // start off as off
}

void print_text(SDL_Renderer *renderer) {
    const uint32_t len = 48;
    uint32_t amt = 48;
    size_t y = 16;
    uint32_t i = 0;
    while (i < text_pos) {
        amt = len;
        while (i + amt < text_pos && amt > 0 && narrate_text[i + amt] != ' ') { amt--; }
        print_line_len(renderer, 0, narrate_text + i, 200, y, amt);
        i += amt;
        while (isspace(narrate_text[i])) { i++; }
        y += 16;
    }
}

void print_menu(SDL_Renderer *renderer) {
    size_t x = 140, y = 490;
    uint32_t sx = sprite_getx(background);
    uint32_t sy = sprite_gety(background);

    sprite_set_location(background, sx, y - 5);
    sprite_render(renderer, background);

    for (int i = 0; i < MAX_OPTIONS; i++) {
        print_line_len(renderer, 0, menu_options[i], x, y, 0x7FFFFFFF);
        y += 20;
    }

    sprite_set_location(background, sx, sy);
}

void port_narrate_clear() {
    clear();
}

void port_narrate_close() {
    display = 0;
    clear();
}

void narrate_render(void *data) {
    SDL_Renderer *renderer = main_get_rend();
    if (display) {
        sprite_render(renderer, background);
        sprite_render(renderer, border);
        //SDL_RenderCopy(renderer, border, NULL, &border_loc);
        SDL_RenderCopy(renderer, portraits[portrait_index], NULL, &portraits_loc[portrait_index]);
        print_text(renderer);
        if (display_menu) {
            print_menu(renderer);
        }
    }
}

void narrate_clear() {
    text_pos = 0;
}

static void add_text(const char *to_add) {
    if (!to_add) { return; }

    int len = strlen(to_add);

    if (text_pos + len >= MAX_TEXT) {
        error("Overflowed text buffer in narrate!");
        exit(1);
    }

    strcpy(narrate_text+text_pos, to_add);
    text_pos += len;
}

int8_t port_narrate_open(int16_t action, const char *text, int16_t index) {
    display = 1; // start off as off
    switch(action) {
        case NAR_ADD_MENU:
            display_menu = 1;
            //warn("I need to add_menu with index %d, text = '%s'\n", index, text);
            strncpy(menu_options[menu_pos], text, MAX_LINE);
            menu_options[menu_pos++][MAX_LINE-1] = '\0'; // guard
            break;
        case NAR_PORTRAIT:
            portrait_index = index;
            add_text(text);
            break;
        case NAR_SHOW_TEXT:
            add_text(text);
            break;
        case NAR_SHOW_MENU:
            display_menu = 1;
            break;
        case NAR_EDIT_BOX:
            warn("I need to show edit box (index =  %d), text = '%s'\n", index, text);
            break;
        default:
            error("narrate_open: ERROR unknown action %d\n", action);
            exit(1);
    }
    //debug("narrate_text = '%s'\n", narrate_text);
    return 0;
}

int narrate_handle_mouse_movement(const uint32_t x, const uint32_t y) {
    return display; // zero means I did not handle the mouse, so another window may.
}

int narrate_handle_mouse_down(const uint32_t button, const uint32_t x, const uint32_t y) {
    int const height = 18;
    int y_test = 516;
    int option;
    if (display_menu) {
        if (x >= 150 && x <= 600) {
            for (int i = 1; i < 8; i++) {
                if (y >= y_test  && y < y_test + height) {
                    option = narrate_select_menu(i);
                    if (option >= 0) {
                        sol_game_loop_signal(WAIT_NARRATE_SELECT, option);
                    }
                    return 1;
                }
                y_test += height;
            }
        }
    }

    return display; // zero means I did not handle the mouse click, so another window may.
}

int narrate_handle_key_down(const enum entity_action_e action) {
    if (!display) { return 0; }

    printf("narrate action: %d\n", action);
    switch (action) {
        case EA_ACTIVATE:
            if (sol_game_loop_is_waiting_for(WAIT_NARRATE_CONTINUE)) {
                narrate_clear();
                sol_game_loop_signal(WAIT_NARRATE_CONTINUE, 0);
            }
        default:
        break;
    }
    return 1; // Handle the key
}

void narrate_free() {
    sprite_free(background);
    sprite_free(border);
    for (int i = 0; i < MAX_PORTRAITS; i++) {
        SDL_DestroyTexture(portraits[i]);
    }
}

int port_ask_yes_no() {
    error("MUST ASK YES NO, for now NO\n");
    return 0;
}

wops_t narrate_window = {
    .init = narrate_init,
    .cleanup = narrate_free,
    .render = narrate_render,
    .mouse_movement = narrate_handle_mouse_movement,
    .mouse_down = narrate_handle_mouse_down,
    .key_down = narrate_handle_key_down,
    .mouse_up = NULL,
    .grey_out_map = 0,
    .data = NULL
};
