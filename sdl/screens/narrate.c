#include "narrate.h"
#include "../../src/dsl.h"
#include "../../src/dsl-execute.h"
#include "../../src/dsl-narrate.h"
#include "../../src/gff.h"
#include "../../src/gfftypes.h"

#define STARTX (60)
#define MAX_CHARS (256)
#define MAX_PORTRAITS (256)
#define MAX_TEXT (4096)
#define MAX_LINE (128)
#define MAX_OPTIONS (32)

static SDL_Texture *background = NULL;
static SDL_Rect background_loc = { STARTX + 0, 0, 0, 0 };
static SDL_Rect menu_loc =       { STARTX + 0, 480, 0, 0 };
static SDL_Texture *border = NULL;
static SDL_Rect border_loc = { STARTX + 10, 10, 0, 0 };
static SDL_Texture *font_table[MAX_CHARS];
static SDL_Rect font_locs[MAX_CHARS];

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

void load_portraits(SDL_Renderer *renderer) {
    unsigned long len;
    unsigned char *data;
    unsigned int w, h, id;
    SDL_Surface *surface = NULL;
    unsigned int *ids = gff_get_id_list(DSLDATA_GFF_INDEX, GT_PORT);
    unsigned int num_ids = gff_get_resource_length(DSLDATA_GFF_INDEX, GT_PORT);
    memset(portraits_loc, 0x0, sizeof(SDL_Rect) * MAX_PORTRAITS);
    memset(portraits, 0x0, sizeof(SDL_Texture*) * MAX_PORTRAITS);
    memset(font_table, 0x0, sizeof(SDL_Texture*) * MAX_CHARS);
    memset(font_locs, 0x0, sizeof(SDL_Rect) * MAX_CHARS);
    memset(narrate_text, 0x0, sizeof(char) * MAX_TEXT);
    memset(menu_options, 0x0, sizeof(char) * MAX_LINE * MAX_OPTIONS);

    for (int i = 0; i < num_ids; i++) {
        id = ids[i];
        data = (unsigned char*)gff_get_raw_bytes(DSLDATA_GFF_INDEX, GT_PORT, id, &len);
        data = get_portrait(data, &w, &h);
        surface = SDL_CreateRGBSurfaceFrom(data, w, h, 32,
            4*w, 0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);
        portraits[id] = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_FreeSurface(surface);
        portraits_loc[id].w = 2 * w;
        portraits_loc[id].h = 2 * h;
        portraits_loc[id].x = STARTX + 26;
        portraits_loc[id].y = 24;
    }

    if (ids) { free(ids); }
}

static void create_font(SDL_Renderer *renderer) {
    unsigned long len;
    ds_char_t *ds_char = NULL;
    uint32_t fg_color = 0xFFFF00FF;
    uint32_t bg_color = 0x000000FF;
    ds_font_t *dsfont = (ds_font_t*) gff_get_raw_bytes(RESOURCE_GFF_INDEX, GT_FONT, 100, &len);
    char *data = NULL;
    SDL_Surface *surface = NULL;
    for (int c = 0; c < MAX_CHARS; c++ ) {
        ds_char = (ds_char_t*)(((uint8_t*)dsfont) + dsfont->char_offset[c]);
        data = (char*)create_font_rgba(RESOURCE_GFF_INDEX, c, fg_color, bg_color);
        font_locs[c].w = ds_char->width;
        font_locs[c].h = dsfont->height;
        surface = SDL_CreateRGBSurfaceFrom(data, font_locs[c].w, font_locs[c].h, 32, 4*font_locs[c].w,
                0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);
        font_table[c] = SDL_CreateTextureFromSurface(renderer, surface);
        font_locs[c].w *= 2;
        font_locs[c].h *= 2;
        SDL_FreeSurface(surface);
    }
}

void narrate_init(SDL_Renderer *renderer) {
    uint32_t palette_id = gff_get_palette_id(RESOURCE_GFF_INDEX, 0);
    background = create_texture(renderer, RESOURCE_GFF_INDEX, GT_BMP, 3007, 0, palette_id, &background_loc);
    border = create_texture(renderer, RESOURCE_GFF_INDEX, GT_BMP, 12000, 0, palette_id, &border_loc);
    background_loc.w *= 2;
    background_loc.h *= 2;
    menu_loc.w = background_loc.w;
    menu_loc.h = background_loc.h;
    border_loc.w *= 2;
    border_loc.h *= 2;
    load_portraits(renderer);
    create_font(renderer);
    SDL_SetTextureAlphaMod( background, 192 );
    display = 0; // start off as off
}

void print_line_len(SDL_Renderer *renderer, const char *text, size_t x, size_t y, const uint32_t len) {
    size_t c;
    if (text == NULL) { return; }
    for (int i = 0; text[i] && i < len; i++) {
        c = text[i];
        font_locs[c].x = x;
        font_locs[c].y = y;
        SDL_RenderCopy(renderer, font_table[c], NULL, &font_locs[c]);
        x += font_locs[c].w;
        //debug("Need to print '%c'\n", i);
    }
}

void print_text(SDL_Renderer *renderer) {
    const uint32_t len = 48;
    uint32_t amt = 48;
    size_t y = 16;
    uint32_t i = 0;
    while (i < text_pos) {
        amt = len;
        while (i + amt < text_pos && amt > 0 && narrate_text[i + amt] != ' ') { amt--; }
        print_line_len(renderer, narrate_text + i, 200, y, amt);
        i += amt;
        while (isspace(narrate_text[i])) { i++; }
        y += 16;
    }
}

void print_menu(SDL_Renderer *renderer) {
    size_t x = 140, y = 490;
    SDL_RenderCopy(renderer, background, NULL, &menu_loc);
    for (int i = 0; i < MAX_OPTIONS; i++) {
        print_line_len(renderer, menu_options[i], x, y, 0x7FFFFFFF);
        y += 20;
    }
}

void port_narrate_clear() {
    clear();
}

void port_narrate_close() {
    display = 0;
    clear();
}

void narrate_render(void *data, SDL_Renderer *renderer) {
    if (display) {
        SDL_RenderCopy(renderer, background, NULL, &background_loc);
        SDL_RenderCopy(renderer, border, NULL, &border_loc);
        SDL_RenderCopy(renderer, portraits[portrait_index], NULL, &portraits_loc[portrait_index]);
        print_text(renderer);
        if (display_menu) {
            print_menu(renderer);
        }
    }
}

static void add_text(const char *to_add) {
    if (!to_add) { return; }
    if (!strcmp(to_add, "END")) {
        end_received = 1;
        return;
    }
    if (!strcmp(to_add, "CLOSE")) {
        close_received = 1;
        return;
    }

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
    return 0; // zero means I did not handle the mouse, so another screen may.
}

int narrate_handle_mouse_click(const uint32_t x, const uint32_t y) {
    int const height = 18;
    int y_test = 516;
    if (display_menu) {
        if (x >= 150 && x <= 600) {
            for (int i = 1; i < 8; i++) {
                if (y >= y_test  && y < y_test + height) {
                    narrate_select_menu(i);
                    return 1;
                }
                y_test += height;
            }
        }
    }
    /*
    if (display && !display_menu) {
        int call_resume = !end_received && !close_received;
        clear();

        if (call_resume) {
            dsl_resume_dialog();
        }

        return 1;
    }
    */
    return 0; // zero means I did not handle the mouse click, so another screen may.
}

void narrate_free() {
}

sops_t narrate_screen = {
    .init = narrate_init,
    .render = narrate_render,
    .mouse_movement = narrate_handle_mouse_movement,
    .mouse_click = narrate_handle_mouse_click,
    .data = NULL
};
