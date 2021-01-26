#include <SDL2/SDL.h>
#include <stdio.h>
#include "map.h"
#include "font.h"
#include "screens/narrate.h"
#include "gameloop.h"
#include "../src/dsl.h"
#include "../src/dsl-region.h"
#include "../src/gff.h"
#include "../src/gff-map.h"
#include "../src/gff-image.h"
#include "../src/spells.h"

#define BUF_MAX (1<<12)
#define RES_MAX (1<<12)
#define NAME_MAX (1<<14)

static int done = 0;
static uint32_t last_tick = 0;
static const uint32_t TICK_AMT = 1000 / TICKS_PER_SEC;// Not fully correct...
static SDL_Renderer *renderer = NULL;
static SDL_Surface *surface = NULL;
static int gff_idx = 0, row_selected = 0, row_max = 0, entry_idx = 0, entry_max, res_idx = 0, res_max = 0;
static uint32_t res_ids[RES_MAX];
static gff_image_entry_t *cimg = NULL;
static int cframe = 0;
static double zoom = 1.0;
static int mapx = 0, mapy = 0;
static int cobj = 0, max_objs = 0;

static void browse_tick();
static void browse_handle_input();
static void browse_render();
static void move_gff_cursor(int amt);
static void print_gff_entries();
static void render_entry();
static void move_entry_cursor(int amt);
static void write_blob();
static void move_res_cursor(int amt);
static void move_frame_cursor(int amt);

static void browse_handle_input() {
    //const Uint8 *key_state = SDL_GetKeyboardState(NULL);
    SDL_Event event;
    while (SDL_PollEvent(&event) != 0) {
        switch(event.type) {
            case SDL_QUIT:
                done = 1;
                break;
            case SDL_KEYDOWN:
                switch (event.key.keysym.sym) {
                    case SDLK_ESCAPE: done = 1; break;
                    case SDLK_PAGEDOWN: move_gff_cursor(1); break;
                    case SDLK_PAGEUP: move_gff_cursor(-1); break;
                    case SDLK_DOWN: move_entry_cursor(1); break;
                    case SDLK_UP: move_entry_cursor(-1); break;
                    case SDLK_RIGHT: move_res_cursor(1); break;
                    case SDLK_LEFT: move_res_cursor(-1); break;
                    case SDLK_w: write_blob(); break;
                    case SDLK_f: move_frame_cursor(1); mapy += 1; cobj++;break;
                    case SDLK_d: mapx += 1; break;
                    case SDLK_s: mapy -= 1; cobj--; break;
                    case SDLK_e: mapx -= 1; break;
                    case SDLK_KP_MINUS: if (zoom > 1.0) {zoom -= 0.25;} break;
                    case SDLK_KP_PLUS: zoom += 0.25; break;
                }
                break;
            case SDL_MOUSEMOTION:
                //handle_mouse_motion();
                break;
            case SDL_MOUSEBUTTONDOWN:
                if (game_loop_is_waiting_for(WAIT_NARRATE_CONTINUE)) {
                    game_loop_signal(WAIT_NARRATE_CONTINUE, 0);
                }
                //handle_mouse_click();
                break;
        }
    }

    //if(key_state[SDL_SCANCODE_PAGEDOWN])  { row_selected++; }
    //if(key_state[SDL_SCANCODE_DOWN])  { ymappos += 2; handle_mouse_motion(); }
    //if(key_state[SDL_SCANCODE_UP])    { ymappos -= 2; handle_mouse_motion(); }
    //if(key_state[SDL_SCANCODE_LEFT])  { xmappos -= 2; handle_mouse_motion(); }
    //if(key_state[SDL_SCANCODE_RIGHT]) { xmappos += 2; handle_mouse_motion(); }
    //if(key_state[SDL_SCANCODE_D])     { player_move(PLAYER_DOWN); }
    //if(key_state[SDL_SCANCODE_E])     { player_move(PLAYER_UP); }
    //if(key_state[SDL_SCANCODE_S])     { player_move(PLAYER_LEFT); }
    //if(key_state[SDL_SCANCODE_F])     { player_move(PLAYER_RIGHT); }
}

static void clear_state() {
    if (cimg) {
        free(cimg);
        cimg = NULL;
    }
    mapx = mapy = 0;
    cframe = 0;
    cobj = max_objs = 0;
}

static void move_frame_cursor(int amt) {
    if (!cimg) { return; }

    cframe = (cimg->frame_num + cframe + amt) % cimg->frame_num;
}

static void move_res_cursor(int amt) {
    clear_state();

    if (res_max > 0) {
        res_idx = (res_max + res_idx + amt) % res_max;
    }
}

static void move_entry_cursor(int amt) {
    uint32_t type = 0;

    if (entry_max < 1) { return; } // avoid infinite loop.

    if (amt < -1 || amt > 1) {
        printf("ERROR: need to update code for mvoing amt more than 1!\n");
        exit(1);
    }

    entry_idx = (entry_max + entry_idx + amt) % entry_max;
    res_max = gff_get_resource_length(gff_idx, type = gff_get_type_id(gff_idx, entry_idx));
    if (res_max > RES_MAX) {
        fprintf(stderr, "ERROR: res_max (%d) > RES_MAX (%d)\n", res_max, RES_MAX);
        exit(1);
    }
    gff_get_resource_ids(gff_idx, type, res_ids);

    res_idx = res_max ? res_idx % res_max : 0;

    move_res_cursor(0);
}

static void move_gff_cursor(int amt) {
    if (row_max < 1) { return; } // avoid infinite loop.
    if (amt < -1 || amt > 1) {
        printf("ERROR: need to update code for mvoing amt more than 1!\n");
        exit(1);
    }

    row_selected = (row_selected + amt) % row_max;
    gff_idx = (gff_idx + amt) % NUM_FILES;

    if (row_selected < 0) {
        row_selected = row_max - 1;
        gff_idx = NUM_FILES - 1;
    }

    while (!open_files[gff_idx].filename) {
        gff_idx = (gff_idx + amt) % NUM_FILES;
        if (gff_idx < 0) {
            printf("ERROR: went passed bounds!\n");
            exit(1);
        }
    }

    entry_max = gff_get_number_of_types(gff_idx);

    move_entry_cursor(0); // update entry res points.
}

static void print_menu() {
    int row = 0;
    SDL_Rect rect;

    print_line_len(renderer, 0, "GFFs:", 20, 20, 1<<12);

    row_max = 0;
    for (int i = 0; i < NUM_FILES; i++) {
        if (open_files[i].filename) {
            print_line_len(renderer, 0, open_files[i].filename, 20, 40 + row++ * 20, 1<<12);
            row_max++;
        }
    }

    SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0x00, SDL_ALPHA_OPAQUE);
    rect.x = 10;
    rect.y = 40 + row_selected * 20;
    rect.h = 20;
    rect.w = 20 + strlen(open_files[gff_idx].filename) * 10;
    SDL_RenderDrawRect(renderer, &rect);
}

#define BLOB_MAX (1<<16)
static void write_blob() {
    char buf[BLOB_MAX];
    gff_chunk_header_t chunk = gff_find_chunk_header(gff_idx, gff_get_type_id(gff_idx, entry_idx), res_ids[res_idx]);
    gff_read_chunk(gff_idx, &chunk, buf, BLOB_MAX);
    FILE *file = fopen("out.dat", "wb");
    fwrite(buf, 1, chunk.length, file);
    fclose(file);
}

static void print_gff_entries() {
    char buf[BUF_MAX];
    entry_max = gff_get_number_of_types(gff_idx);
    SDL_Rect rect;

    if (entry_idx >= entry_max) { entry_idx = entry_max - 1; }

    snprintf(buf, BUF_MAX, "ENTRIES:");
    print_line_len(renderer, 0, buf, 220, 20, BUF_MAX);
    for (int i = 0; i < entry_max; i++) {
        get_gff_type_name(gff_get_type_id(gff_idx, i), buf);
        buf[4] = '\0';
        print_line_len(renderer, 0, buf, 220, 40 + 20 * i, BUF_MAX);
    }

    SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0x00, SDL_ALPHA_OPAQUE);
    rect.x = 210;
    rect.y = 40 + entry_idx * 20;
    rect.h = 20;
    rect.w = 60;
    SDL_RenderDrawRect(renderer, &rect);
}

static void browse_render() {
    SDL_SetRenderDrawColor( renderer, 0x00, 0x00, 0x00, 0xFF );
    SDL_RenderClear(renderer);
    print_menu();
    print_gff_entries();
    render_entry();
    SDL_RenderPresent(renderer);
}

static void browse_tick() {
    uint32_t cticks = SDL_GetTicks();
    uint32_t amt_to_wait = 0;
    last_tick += TICK_AMT;
    //debug("last_time = %u, cticks = %u\n", last_tick, cticks);
    if (last_tick < cticks) {
        debug("gameloop took too long, not sleeping...\n");
    } else {
        amt_to_wait = last_tick - cticks;
        //debug("going to wait %ums\n", amt_to_wait);
        SDL_Delay(amt_to_wait);
    }
}

void browse_loop(SDL_Surface *surface, SDL_Renderer *rend) {
    renderer = rend;

    narrate_init(renderer, 0, 0, 2.0); // to setup print_line

    browse_render();
    move_gff_cursor(1);
    move_gff_cursor(1);
    move_gff_cursor(1);
    //move_gff_cursor(1);
    //move_gff_cursor(1);
    //move_gff_cursor(1);
    //move_gff_cursor(1);
    //move_entry_cursor(-1);
    move_entry_cursor(1);
    move_entry_cursor(1);
    move_entry_cursor(1);
    move_entry_cursor(1);
    //move_entry_cursor(1);
    //move_entry_cursor(1);
    res_idx = 0;
    while (!done) {
        browse_handle_input();
        //Logic here...
        browse_render();
        browse_tick();
        /*
        rep_times++;
        if (in_replay_mode() && rep_times > 10) {
            replay_next();
            rep_times = 0;
        }
        */
    }
}

static void print_para_len(SDL_Renderer *renderer, const char *str,
        const int x, const int y, const int w, const int len) {
    for (int s = 0; s < len; s += w) {
        print_line_len(renderer, 0, str + s, x, y + (20 * s/w),
                s + w < len ? w : len - s - 1);
    }
}

static void render_entry_header();
static void render_entry_text();
static void render_entry_monr();
static void render_entry_rdff();
static void render_entry_name();
//static void render_entry_rdat();
static void render_entry_font();
static void render_entry_spin();
static void render_entry_pal();
static void render_entry_cpal();
static void render_entry_merr();
static void render_entry_bmp();
static void render_entry_icon();
static void render_entry_port();
static void render_entry_ojff();
static void render_entry_tile();
static void render_entry_rmap();
static void render_entry_gmap();
static void render_entry_etab();
static void render_entry_spst();
static void render_entry_psst();
static void render_entry_char();

static void render_entry() {
    switch(gff_get_type_id(gff_idx, entry_idx)) {
        case GFF_TEXT: render_entry_text(); break;
        case GFF_MONR: render_entry_monr(); break;
        case GFF_RDFF: render_entry_rdff(); break;
        case GFF_NAME: render_entry_name(); break;
        //case GFF_RDAT: render_entry_rdat(); break;
        case GFF_FONT: render_entry_font(); break;
        case GFF_SPIN: render_entry_spin(); break;
        case GFF_PAL: render_entry_pal(); break;
        case GFF_CPAL: render_entry_cpal(); break;
        case GFF_MERR: render_entry_merr(); break;
        case GFF_BMP: render_entry_bmp(); break;
        case GFF_ICON: render_entry_icon(); break;
        case GFF_PORT: render_entry_port(); break;
        case GFF_OJFF: render_entry_ojff(); break;
        case GFF_TILE: render_entry_tile(); break;
        case GFF_RMAP: render_entry_rmap(); break;
        case GFF_GMAP: render_entry_gmap(); break;
        case GFF_ETAB: render_entry_etab(); break;
        case GFF_SPST: render_entry_spst(); break;
        case GFF_PSST: render_entry_psst(); break;
        case GFF_CHAR: render_entry_char(); break;
        default:
            render_entry_header();
            print_line_len(renderer, 0, "Need to implement", 320, 40, 128);
    }
}

static void render_entry_header() {
    char buf[BUF_MAX];
    int32_t type = gff_get_type_id(gff_idx, entry_idx);
    snprintf(buf, BUF_MAX, "RESOURCE %d of %d (id: %d) %c%c%c%c\n", res_idx, res_max - 1, res_ids[res_idx],
        (type >> 0) & 0x000000FF,
        (type >> 8) & 0x000000FF,
        (type >> 16) & 0x000000FF,
        (type >> 24) & 0x000000FF
        );
    print_line_len(renderer, 0, buf, 320, 20, BUF_MAX);
}

static void render_entry_as_text(const int type_id) {
    char buf[1024];
    gff_chunk_header_t chunk = gff_find_chunk_header(gff_idx, type_id, res_ids[res_idx]);
    gff_read_chunk(gff_idx, &chunk, buf, 1024);
    render_entry_header();
    if (chunk.length) {
        print_para_len(renderer, buf, 320, 40, 40, chunk.length);
    }
}

static void render_entry_text() {
    render_entry_as_text(GFF_TEXT);
}

static void render_entry_spin() {
    render_entry_as_text(GFF_SPIN);
}

static void render_entry_merr() {
    render_entry_as_text(GFF_MERR);
}

static void render_entry_as_image(const int gff_idx, const int type_id, const int res_id,
        gff_palette_t *pal, const int x, const int y) {
    render_entry_header();
    SDL_Rect loc;
    char buf[BUF_MAX];

    if (!cimg) {
        gff_chunk_header_t chunk = gff_find_chunk_header(gff_idx, type_id, res_id);
        cimg = (gff_image_entry_t*) malloc(sizeof(gff_image_entry_t*) * chunk.length);
        gff_read_chunk(gff_idx, &chunk, &(cimg->data), chunk.length);
        cimg->data_len = chunk.length;
        cimg->frame_num = *(uint16_t*)(cimg->data + 4);
    }

    snprintf(buf, BUF_MAX-1, "current frame: %d, total frames: %d\n", cframe, cimg->frame_num);
    print_line_len(renderer, 0, buf, x, y, BUF_MAX);
    unsigned char* data = get_frame_rgba_palette_img(cimg, cframe, pal);
    loc.w = get_frame_width(gff_idx, type_id, res_id, cframe);
    loc.h = get_frame_height(gff_idx, type_id, res_id, cframe);
    SDL_Surface *surface = SDL_CreateRGBSurfaceFrom(data, loc.w, loc.h, 32, 
            4*loc.w, 0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);
    SDL_Texture *tex = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    loc.x = x;
    loc.y = y + 20;
    loc.w *= zoom;
    loc.h *= zoom;
    SDL_RenderCopy( renderer, tex, NULL, &loc);
    SDL_DestroyTexture(tex);
    free(data);
}

static void render_entry_bmp() {
    gff_palette_t *pal = open_files[RESOURCE_GFF_INDEX].pals->palettes;

    if (open_files[gff_idx].pals->len) {
        pal = open_files[gff_idx].pals->palettes;
    }

    render_entry_as_image(gff_idx, GFF_BMP, res_ids[res_idx], pal, 320, 40);
}

static void render_entry_icon() {
    render_entry_as_image(gff_idx, GFF_ICON, res_ids[res_idx], open_files[RESOURCE_GFF_INDEX].pals->palettes, 320, 40);
}

static void render_entry_port() {
    render_entry_as_image(gff_idx, GFF_PORT, res_ids[res_idx], open_files[RESOURCE_GFF_INDEX].pals->palettes, 320, 40);
}

static void render_entry_tile() {
    int num;
    gff_palette_t *pal = open_files[RESOURCE_GFF_INDEX].pals->palettes;
    
    sscanf(open_files[gff_idx].filename, "rgn%x.gff", &num);
    num--;
    if (num < open_files[DSLDATA_GFF_INDEX].pals->len) {
        pal = open_files[DSLDATA_GFF_INDEX].pals->palettes + num;
    }
    render_entry_as_image(gff_idx, GFF_TILE, res_ids[res_idx], pal, 320, 40);
}

/*
static void render_entry_rdat() {
    static char buf[BUF_MAX];
    snprintf(buf, BUF_MAX, "RESOURCE %d of %d \n", res_idx, res_max - 1);
    print_line_len(renderer, 0, buf, 320, 20, BUF_MAX);
    gff_chunk_header_t chunk = gff_find_chunk_header(gff_idx, GFF_RDAT, res_ids[res_idx]);
    gff_read_chunk(gff_idx, &chunk, buf, BUF_MAX);
    print_line_len(renderer, 0, buf, 320, 40, chunk.length);
}
*/

static void render_entry_name() {
    static char buf[BUF_MAX];
    static char names[NAME_MAX];

    if (res_max < 2) {
        gff_chunk_header_t chunk = gff_find_chunk_header(gff_idx, GFF_NAME, 1);
        res_max = chunk.length / 25;
        gff_read_chunk(gff_idx, &chunk, names, NAME_MAX);
    }

    snprintf(buf, BUF_MAX, "RESOURCE %d of %d \n", res_idx, res_max - 1);
    print_line_len(renderer, 0, buf, 320, 20, BUF_MAX);
    print_line_len(renderer, 0, names + 25*res_idx, 320, 40, BUF_MAX);
}

static void render_entry_ojff() {
    disk_object_t disk_obj;
    gff_chunk_header_t chunk = gff_find_chunk_header(gff_idx, GFF_OJFF, res_ids[res_idx]);
    char buf[BUF_MAX];

    render_entry_header();

    gff_read_chunk(gff_idx, &chunk, &disk_obj, chunk.length);
    print_line_len(renderer, 0, "Disk Object:", 320, 40, BUF_MAX);
    snprintf(buf, BUF_MAX, "offset = (%d, %d)", disk_obj.xoffset, disk_obj.yoffset);
    print_line_len(renderer, 0, buf, 320, 60, BUF_MAX);
    snprintf(buf, BUF_MAX, "pos = (%d, %d, %d)", disk_obj.xpos, disk_obj.ypos, disk_obj.zpos);
    print_line_len(renderer, 0, buf, 320, 80, BUF_MAX);
    snprintf(buf, BUF_MAX, "flags = 0x%x", disk_obj.flags);
    print_line_len(renderer, 0, buf, 320, 100, BUF_MAX);
    snprintf(buf, BUF_MAX, "object_index = %d", disk_obj.object_index);
    print_line_len(renderer, 0, buf, 320, 120, BUF_MAX);
    snprintf(buf, BUF_MAX, "bmp_id = %d", disk_obj.bmp_id);
    print_line_len(renderer, 0, buf, 320, 140, BUF_MAX);
    snprintf(buf, BUF_MAX, "script_id = %d", disk_obj.script_id);
    print_line_len(renderer, 0, buf, 320, 160, BUF_MAX);
}

static void render_entry_as_pal(const int type_id) {
    render_entry_header();
    gff_palettes_t *pals = read_palettes_type(gff_idx, type_id);
    const int pal_break = 64;

    for (int i = 0; i < PALETTE_SIZE; i++) {
        SDL_SetRenderDrawColor(renderer,
                pals->palettes[res_idx].color[i].r,
                pals->palettes[res_idx].color[i].g,
                pals->palettes[res_idx].color[i].b,
                0xFF);
        int x = ((2*i)%pal_break) + 300;
        int y = 40 + 2 * i / pal_break;
        SDL_RenderDrawPoint(renderer, x, y);
        SDL_RenderDrawPoint(renderer, x + 1, y);
        SDL_RenderDrawPoint(renderer, x, y + 1);
        SDL_RenderDrawPoint(renderer, x + 1, y + 1);
    }
    SDL_RenderDrawPoint(renderer, 400, 300);

    free(pals);
}

static void render_entry_pal() {
    render_entry_as_pal(GFF_PAL);
}

static void render_entry_cpal() {
    render_entry_as_pal(GFF_CPAL);
}

static void render_entry_monr() {
    render_entry_header();
    print_line_len(renderer, 0, "Need to implement", 320, 40, 128);
}

const char* rdff_type_names[] = {
    "",
    "OBJECT",
    "CONTAINER",
    "POINTER",
    "NEXT",
    "END"
};

const char *so_object_names[] = {
    "?",
    "Combat",
    "Item",
    "Mini"
};

static void print_combat(ds1_combat_t combat, int pos) {
    char buf[BUF_MAX];
    snprintf(buf, BUF_MAX, "Name: %s\n", combat.name);
    print_line_len(renderer, 0, buf, 320, pos, 128); pos += 20;
    snprintf(buf, BUF_MAX, "HP: %d\n", combat.hp);
    print_line_len(renderer, 0, buf, 320, pos, 128); pos += 20;
    snprintf(buf, BUF_MAX, "PSP: %d\n", combat.psp);
    print_line_len(renderer, 0, buf, 320, pos, 128); pos += 20;
    //snprintf(buf, BUF_MAX, "Character Index: %d\n", combat.char_index);
    //print_line_len(renderer, 0, buf, 320, pos, 128); pos += 20;
    snprintf(buf, BUF_MAX, "ID: %d\n", combat.id);
    print_line_len(renderer, 0, buf, 320, pos, 128); pos += 20;
    //snprintf(buf, BUF_MAX, "Weapon: %d\n", combat.weapon_index);
    //print_line_len(renderer, 0, buf, 320, pos, 128); pos += 20;
    //snprintf(buf, BUF_MAX, "Packed: %d\n", combat.pack_index);
    //print_line_len(renderer, 0, buf, 320, pos, 128); pos += 20;
    snprintf(buf, BUF_MAX, "Special Attack: %d\n", combat.special_attack);
    print_line_len(renderer, 0, buf, 320, pos, 128); pos += 20;
    snprintf(buf, BUF_MAX, "Special Defense: %d\n", combat.special_defense);
    print_line_len(renderer, 0, buf, 320, pos, 128); pos += 20;
    snprintf(buf, BUF_MAX, "Icon: %d\n", combat.icon);
    print_line_len(renderer, 0, buf, 320, pos, 128); pos += 20;
    snprintf(buf, BUF_MAX, "AC: %d\n", combat.ac);
    print_line_len(renderer, 0, buf, 320, pos, 128); pos += 20;
    snprintf(buf, BUF_MAX, "move: %d\n", combat.move);
    print_line_len(renderer, 0, buf, 320, pos, 128); pos += 20;
    snprintf(buf, BUF_MAX, "status: %d\n", combat.status);
    print_line_len(renderer, 0, buf, 320, pos, 128); pos += 20;
    snprintf(buf, BUF_MAX, "allegiance: %d\n", combat.allegiance);
    print_line_len(renderer, 0, buf, 320, pos, 128); pos += 20;
    snprintf(buf, BUF_MAX, "data: %d\n", combat.data);
    print_line_len(renderer, 0, buf, 320, pos, 128); pos += 20;
    snprintf(buf, BUF_MAX, "thac0: %d\n", combat.thac0);
    print_line_len(renderer, 0, buf, 320, pos, 128); pos += 20;
    snprintf(buf, BUF_MAX, "priority: %d\n", combat.priority);
    print_line_len(renderer, 0, buf, 320, pos, 128); pos += 20;
    snprintf(buf, BUF_MAX, "str: %d, dex: %d\n, con: %d",
       combat.stats.STR, combat.stats.DEX, combat.stats.CON);
    print_line_len(renderer, 0, buf, 320, pos, 128); pos += 20;
    snprintf(buf, BUF_MAX, "int: %d, wis: %d\n, cha: %d",
       combat.stats.INT, combat.stats.WIS, combat.stats.CHA);
    print_line_len(renderer, 0, buf, 320, pos, 128); pos += 20;
    print_line_len(renderer, 0, "Not Shown: weapon, packed, character id", 320, pos, 128); pos += 20;
}

static void print_item(ds1_item_t item, int pos) {
    char buf[BUF_MAX];
    snprintf(buf, BUF_MAX, "id: %d\n", item.id);
    print_line_len(renderer, 0, buf, 320, pos, 128); pos += 20;
    snprintf(buf, BUF_MAX, "quantity: %d\n", item.quantity);
    print_line_len(renderer, 0, buf, 320, pos, 128); pos += 20;
    snprintf(buf, BUF_MAX, "next: %d\n", item.next);
    print_line_len(renderer, 0, buf, 320, pos, 128); pos += 20;
    snprintf(buf, BUF_MAX, "value: %d\n", item.value);
    print_line_len(renderer, 0, buf, 320, pos, 128); pos += 20;
    snprintf(buf, BUF_MAX, "pack index: %d\n", item.pack_index);
    print_line_len(renderer, 0, buf, 320, pos, 128); pos += 20;
    snprintf(buf, BUF_MAX, "item index: %d\n", item.item_index);
    print_line_len(renderer, 0, buf, 320, pos, 128); pos += 20;
    snprintf(buf, BUF_MAX, "Icon: %d\n", item.icon);
    print_line_len(renderer, 0, buf, 320, pos, 128); pos += 20;
    snprintf(buf, BUF_MAX, "charges: %d\n", item.charges);
    print_line_len(renderer, 0, buf, 320, pos, 128); pos += 20;
    snprintf(buf, BUF_MAX, "special: %d\n", item.special);
    print_line_len(renderer, 0, buf, 320, pos, 128); pos += 20;
    snprintf(buf, BUF_MAX, "priority: %d\n", item.priority);
    print_line_len(renderer, 0, buf, 320, pos, 128); pos += 20;
    snprintf(buf, BUF_MAX, "slot: %d\n", item.slot);
    print_line_len(renderer, 0, buf, 320, pos, 128); pos += 20;
    snprintf(buf, BUF_MAX, "name_index: %d\n", item.name_index);
    print_line_len(renderer, 0, buf, 320, pos, 128); pos += 20;
    snprintf(buf, BUF_MAX, "bonus: %d\n", item.bonus);
    print_line_len(renderer, 0, buf, 320, pos, 128); pos += 20;
    /*
    print_line_len(renderer, 0, "Not Shown: weapon, packed, character id", 320, pos, 128); pos += 20;
    */
}

#define RDFF_MAX (1<<12)
static void render_entry_rdff() {
    char buf[BUF_MAX];
    render_entry_header();
    so_object_t *so = NULL;
    rdff_disk_object_t rdff_buf[RDFF_MAX];
    rdff_disk_object_t *rdff = rdff_buf;
    gff_chunk_header_t chunk = gff_find_chunk_header(gff_idx, GFF_RDFF, res_ids[res_idx]);

    if (chunk.length > RDFF_MAX * sizeof(rdff_disk_object_t)) {
        error ("chunk.length (%d) greater than max (%d)!\n", chunk.length, RDFF_MAX);
        exit(1);
    }

    gff_read_chunk(gff_idx, &chunk, rdff, chunk.length);
    //printf("res_ids[%d] = %d\n", res_idx, res_ids[res_idx]);
    snprintf(buf, BUF_MAX, "load action: %d\n", rdff->load_action);
    print_line_len(renderer, 0, buf, 320, 40, 128);
    snprintf(buf, BUF_MAX, "blocknum: %d\n", rdff->blocknum);
    print_line_len(renderer, 0, buf, 320, 60, 128);
    snprintf(buf, BUF_MAX, "type: %s\n", rdff_type_names[rdff->type]);
    print_line_len(renderer, 0, buf, 320, 80, 128);
    snprintf(buf, BUF_MAX, "index: %d\n", rdff->index);
    print_line_len(renderer, 0, buf, 320, 100, 128);
    snprintf(buf, BUF_MAX, "from: %d\n", rdff->index);
    print_line_len(renderer, 0, buf, 320, 120, 128);
    snprintf(buf, BUF_MAX, "len: %d\n", rdff->len);
    print_line_len(renderer, 0, buf, 320, 140, 128);
    print_line_len(renderer, 0, "-----------------------", 320, 160, 128);
    print_line_len(renderer, 0, "Jumping to entry:", 320, 180, 128);

    switch(rdff->type) {
        case RDFF_OBJECT:
        case RDFF_CONTAINER: // I don't know the different between Container and Object!
            rdff++;
            so = gff_create_object((char*)rdff, rdff - 1, -1);
            snprintf(buf, BUF_MAX, "index: %d\n", rdff->index);
            print_line_len(renderer, 0, buf, 320, 200, 128);
            snprintf(buf, BUF_MAX, "type: %s\n", so_object_names[so->type]);
            print_line_len(renderer, 0, buf, 320, 220, 128);
            if (so->type == SO_DS1_COMBAT) { print_combat(so->data.ds1_combat, 240); }
            if (so->type == SO_DS1_ITEM) { print_item(so->data.ds1_item, 240); }
            break;
        default:
            snprintf(buf, BUF_MAX, "unknown type: %d\n", rdff->type);
            print_line_len(renderer, 0, buf, 320, 220, 128);
            break;
    }
    /*
    rdff++;
    while (rdff->load_action != -1 && rdff->load_action < 5) {
        snprintf(buf, BUF_MAX, "load action: %d\n", rdff->load_action);
        print_line_len(renderer, 0, buf, 320, pos, 128);
        rdff++;
        pos += 20;
    }
    */
}

#define FONT_NUM (1<<8)
static void render_entry_font() {
    char buf[BUF_MAX];
    ds_font_t font[FONT_NUM];
    SDL_Rect loc;
    gff_chunk_header_t chunk = gff_find_chunk_header(gff_idx, GFF_FONT, res_ids[res_idx]);
    if ((sizeof(ds_font_t) * FONT_NUM) < chunk.length) {
        print_line_len(renderer, 0, "ERROR font length > font buf, need to fix!", 340, 20, BUF_MAX);
        return;
    }
    gff_read_chunk(gff_idx, &chunk, font, chunk.length);
    snprintf(buf, BUF_MAX, "RESOURCE %d of %d \n", res_idx, res_max - 1);
    print_line_len(renderer, 0, buf, 320, 20, BUF_MAX);
    for (int c = 0; c < 255; c++) {
        ds_char_t *ds_char = (ds_char_t*)(((uint8_t*)font) + font->char_offset[c]);
        if (ds_char->width) {
            char *data = (char*)create_font_rgba(gff_idx, c, 0xFFFF00FF, 0x000000FF);
            loc.w = ds_char->width;
            loc.h = font->height;
            surface = SDL_CreateRGBSurfaceFrom(data, loc.w, loc.h, 32, 4*loc.w,
                    0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);
            SDL_Texture *tex = SDL_CreateTextureFromSurface(renderer, surface);
            loc.x = 320 + 12 * (c % 32);
            loc.y = 40  + 20 * (c / 32);
            loc.w *= 2;
            loc.h *= 2;
            SDL_FreeSurface(surface);
            SDL_RenderCopy( renderer, tex, NULL, &loc);
            free(data);
            SDL_DestroyTexture(tex);
        }
    }
}

static dsl_region_t *region;
static void render_entry_rmap() {
    render_entry_header();
    static char *cfile = NULL;
    static int tiles_len;
    static SDL_Texture **tiles = NULL;
    uint32_t width, height;
    unsigned char *data;
    SDL_Surface* tile = NULL;
    SDL_Rect loc;

    if (cfile && cfile != open_files[gff_idx].filename) {
        printf("Need to clean dsl_region_t!\n");
        //SDL_DestroyTexture(tiles[0]);
        for (int i = 0; i < tiles_len; i++) {
            if (tiles[i]) {
                SDL_DestroyTexture(tiles[i]);
            }
            tiles[i] = NULL;
        }
        tiles_len = 0;
        if (tiles) {
            free(tiles);
            tiles = NULL;
        }
        cfile = NULL;
    }

    if (!cfile) {
        cfile = open_files[gff_idx].filename;
        if (!cfile) {
            error("cfile is null!\n");
            exit(1);
        }
        region = dsl_load_region(gff_idx);
        tiles_len = region->num_tiles + 1;
        tiles = (SDL_Texture**) malloc(sizeof(SDL_Texture*) * (tiles_len));
        memset(tiles, 0x0, sizeof(SDL_Texture*) * tiles_len);
        for (int i = 0; i < region->num_tiles; i++) {
            dsl_region_get_tile(region, i, &width, &height, &data);

            tile = SDL_CreateRGBSurfaceFrom(data, width, height, 32, 4*width, 0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);
            if (region->ids[i] > (tiles_len)) {
                error ("region->ids[%d] is out of bounds!\n", i);
                exit(1);
            }
            tiles[region->ids[i]] = SDL_CreateTextureFromSurface(renderer, tile);
            SDL_FreeSurface(tile);

            free(data);
        }
        unsigned int *gmap_ids = gff_get_id_list(region->gff_file, GFF_GMAP);
        gff_chunk_header_t chunk = gff_find_chunk_header(region->gff_file, GFF_GMAP, gmap_ids[0]);
        if (!gff_read_chunk(region->gff_file, &chunk, region->flags, chunk.length)) {
            error ("Unable to read GFF_GMAP chunk!\n");
            exit(1);
        }

        region->flags_size = chunk.length;
        free(gmap_ids);
    }

    loc.x = 320;
    loc.y = 40;
    loc.w = 16 * zoom;
    loc.h = 16 * zoom;
    if (mapx < 0) { mapx = 0; }
    if (mapy < 0) { mapy = 0; }
    if (mapx >= MAP_ROWS) { mapx = MAP_ROWS; }
    if (mapy >= MAP_COLUMNS) { mapy = MAP_COLUMNS; }
    for (int i = mapx; i < MAP_ROWS; i++) {
        for (int j = mapy; j < MAP_COLUMNS; j++) {
            SDL_RenderCopy(renderer, tiles[region->tile_ids[i][j]], NULL, &loc);
            loc.x += loc.w;
        }
        loc.y += loc.h;
        loc.x = 320;
    }
}

static void render_entry_gmap() {
    render_entry_rmap();
    SDL_Point points[5];

    for (int i = mapx; i < MAP_ROWS; i++) {
        for (int j = mapy; j < MAP_COLUMNS; j++) {
            if (dsl_region_is_block(region, i, j)) {
                //printf("(%d, %d)\n, ", i, j);
                SDL_SetRenderDrawColor(renderer, 0xFF, 0x00, 0x00, SDL_ALPHA_OPAQUE);
            } else if (dsl_region_is_actor(region, i, j)) {
                SDL_SetRenderDrawColor(renderer, 0x00, 0xFF, 0x00, SDL_ALPHA_OPAQUE);
            } else { continue; }
            points[0].x = 320 + (j - mapy) * 16 * zoom;
            points[0].y = 40  + (i - mapx) * 16 * zoom;
            points[1].x = 320 + (j - mapy + 1) * 16 * zoom;
            points[1].y = 40  + (i - mapx) * 16 * zoom;
            points[2].x = 320 + (j - mapy + 1) * 16 * zoom;
            points[2].y = 40  + (i - mapx + 1) * 16 * zoom;
            points[3].x = 320 + (j - mapy) * 16 * zoom;
            points[3].y = 40  + (i - mapx + 1) * 16 * zoom;
            points[4].x = 320 + (j - mapy) * 16 * zoom;
            points[4].y = 40  + (i - mapx) * 16 * zoom;
            SDL_RenderDrawLines(renderer, points, 5);
        }
    }
}

static void render_entry_etab() {
    static int lobj = -1;
    char buf[BUF_MAX];
    max_objs = gff_map_get_num_objects(gff_idx, res_ids[res_idx]);
    disk_object_t dobj;
    gff_map_object_t *obj = NULL;

    if (cobj < 0) { cobj = max_objs - 1; }
    if (cobj >= max_objs) { cobj = 0; }
    if (lobj != cobj) {
        free(cimg);
        cframe = 0;
        cimg = NULL;
        lobj = cobj;
    }
    render_entry_header();
    obj = open_files[gff_idx].entry_table + cobj;
    snprintf(buf, BUF_MAX, "%d of %d etab objects\n", cobj, max_objs);
    print_line_len(renderer, 0, buf, 320, 40, BUF_MAX);
    snprintf(buf, BUF_MAX, "pos: (%d, %d, %d)\n", obj->xpos, obj->ypos, obj->zpos);
    print_line_len(renderer, 0, buf, 320, 60, BUF_MAX);
    snprintf(buf, BUF_MAX, "flags: 0x%x\n", obj->flags);
    print_line_len(renderer, 0, buf, 320, 80, BUF_MAX);

    if (!gff_read_object(obj->index, &dobj)) {
        snprintf(buf, BUF_MAX, "Unable to read obj: %d\n", obj->index);
        print_line_len(renderer, 0, buf, 320, 80, BUF_MAX);
        return;
    }
    snprintf(buf, BUF_MAX, "disk object[%d]: (object_index not displayed.)\n", obj->index);
    print_line_len(renderer, 0, buf, 320, 100, BUF_MAX);
    snprintf(buf, BUF_MAX, "    .flags = 0x%x\n", dobj.flags);
    print_line_len(renderer, 0, buf, 320, 120, BUF_MAX);
    snprintf(buf, BUF_MAX, "    .offset = (%d, %d)\n", dobj.xoffset, dobj.yoffset);
    print_line_len(renderer, 0, buf, 320, 140, BUF_MAX);
    snprintf(buf, BUF_MAX, "    .pos = (%d, %d, %d)\n", dobj.xpos, dobj.ypos, dobj.zpos);
    print_line_len(renderer, 0, buf, 320, 160, BUF_MAX);
    snprintf(buf, BUF_MAX, "    .script_id = %d\n", dobj.script_id);
    print_line_len(renderer, 0, buf, 320, 180, BUF_MAX);
    snprintf(buf, BUF_MAX, "    .bmp_id = %d\n", dobj.bmp_id);
    print_line_len(renderer, 0, buf, 320, 200, BUF_MAX);
    gff_palette_t *pal = open_files[RESOURCE_GFF_INDEX].pals->palettes;
    render_entry_as_image(OBJEX_GFF_INDEX, GFF_BMP, dobj.bmp_id, pal, 340, 220);
}

static void render_entry_spst() {
    char name[32];
    char buf[BUF_MAX];
    size_t pos;
    spell_list_t spells;
    gff_chunk_header_t chunk = gff_find_chunk_header(gff_idx, GFF_SPST, res_ids[res_idx]);
    gff_read_chunk(gff_idx, &chunk, &spells, sizeof(spells));
    render_entry_header();
    pos = snprintf(buf, BUF_MAX, "Character Wizard/Cleric abilities: ");
    for (int i = 0; i < WIZ_MAX; i++) {
        if (spells.spells[i]) {
            spell_get_wizard_name(i, name);
            pos += snprintf(buf + pos, BUF_MAX - pos, "%s, ", name);
        }
    }
    for (int i = 0; i < CLERIC_MAX; i++) {
        if (spells.spells[68 + i]) {
            spell_get_cleric_name(i, name);
            pos += snprintf(buf + pos, BUF_MAX - pos, "%s, ", name);
        }
    }
    print_para_len(renderer, buf, 320, 40, 40, pos);
}

static void render_entry_psst() {
    uint8_t psionics[34];
    char name[32];
    char buf[BUF_MAX];
    size_t pos;

    gff_chunk_header_t chunk = gff_find_chunk_header(gff_idx, GFF_PSST, res_ids[res_idx]);
    gff_read_chunk(gff_idx, &chunk, &psionics, sizeof(psionics));
    render_entry_header();
    pos = snprintf(buf, BUF_MAX, "Character Psionic abilities: ");
    for (int i = 0; i < 34; i++) {
        if (psionics[i]) {
            spell_get_psionic_name(i, name);
            pos += snprintf(buf + pos, BUF_MAX - pos, "%s, ", name);
        }
    }
    pos += snprintf(buf + pos, BUF_MAX - pos, "\n");
    print_para_len(renderer, buf, 320, 40, 40, pos);
}

static void render_entry_char() {
    char buf[BUF_MAX];
    gff_chunk_header_t chunk = gff_find_chunk_header(gff_idx, GFF_CHAR, res_ids[res_idx]);
    gff_read_chunk(gff_idx, &chunk, &buf, sizeof(buf));
    ds1_combat_t *combat = (ds1_combat_t*)(buf + 10);
   // printf("combat->hp = %d\n", combat->hp);
    print_combat(*combat, 40);
    ds_character_t *character = (ds_character_t*)(buf + 0x4E);
    printf("exp = %d\n", character->current_xp);

    render_entry_header();
}
