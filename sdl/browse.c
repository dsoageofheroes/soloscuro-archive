#include <SDL2/SDL.h>
#include <stdio.h>
#include "map.h"
#include "font.h"
#include "narrate.h"
#include "audio.h"
#include "gpl.h"
#include "../src/region.h"
#include "../src/gameloop.h"
#include "gff.h"
#include "gff-map.h"
#include "gff-image.h"
#include "gff-xmi.h"
#include "../src/wizard.h"
#include "../src/region-manager.h"

#define BUF_MAX (1<<12)
#define RES_MAX (1<<14)

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
static void play_xmi();

typedef struct ds1_it1r_s {
    uint8_t weapon_type;
    uint8_t data0; // always 0, probably structure alignment byte.
    uint16_t damage_type;
    uint8_t weight;
    uint16_t data1;
    uint8_t base_hp;
    uint8_t material;
    uint8_t placement;
    uint8_t range;// Need to confirm
    uint8_t num_attacks;
    uint8_t sides;
    uint8_t dice;
    int8_t mod;
    uint8_t flags;
    uint16_t legal_class;
    int8_t base_AC;
    uint8_t data2; // always 0, 1, or 2
} __attribute__ ((__packed__)) ds1_it1r_t;

const char *weapon_names[] = {
    "NONE",
    "MELEE",
    "MISSILE",
    "SHIELD",
    "USES AMMO",
    "THROWN",
};

const char *damage_types[] = {
    "POISON",
    "MAGIC_FIRE",
    "COLD",
    "BLUNT",
    "CUTTING",
    "POINTED",
    "ACID",
    "ELECTRIC",
    "DRAINING",
    "MAGIC",
    "MENTAL",
    "DEATH",
    "PLUS1",
    "PLUS2",
};

const char *material_types[] = {
    "WOOD",
    "BONE",
    "STONE",
    "OBSIDIAN",
    "METAL",
    "LEATHER",
    "BULKY",
    "IMMOBILE",
};

const char *placement_types[] = {
    "BACKPACK",
    "CHEST",
    "WAIST",
    "ARM",
    "FOOT",
    "HAND",
    "HEAD",
    "NECK",
    "CLOAK",
    "FINGER",
    "LEGS",
    "AMMO",
    "MISSILE",
};

const char *it1r_flags[] = {
    "TOGGLE",
    "BUNDLE",
    "BLOCKING",
    "CONTAINER",
    "USEABLE",
    "DOOR",
    "TWO HANDED",
    "ARMOR",
};

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
                    case SDLK_p: play_xmi(); break;
                    case SDLK_KP_MINUS: if (zoom > 1.0) {zoom -= 0.25;} break;
                    case SDLK_KP_PLUS: zoom += 0.25; break;
                }
                break;
            case SDL_MOUSEMOTION:
                //handle_mouse_motion();
                break;
            case SDL_MOUSEBUTTONDOWN:
                if (sol_game_loop_is_waiting_for(WAIT_NARRATE_CONTINUE)) {
                    sol_game_loop_signal(WAIT_NARRATE_CONTINUE, 0);
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

    sol_print_line_len(0, "GFFs:", 20, 20, 1<<12);

    row_max = 0;
    for (int i = 0; i < NUM_FILES; i++) {
        if (open_files[i].filename) {
            sol_print_line_len(0, open_files[i].filename, 20, 40 + row++ * 20, 1<<12);
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
    sol_print_line_len(0, buf, 220, 20, BUF_MAX);
    for (int i = 0; i < entry_max; i++) {
        gff_get_gff_type_name(gff_get_type_id(gff_idx, i), buf);
        buf[4] = '\0';
        sol_print_line_len(0, buf, 220, 40 + 20 * i, BUF_MAX);
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

    for (gff_idx = 0; !open_files[gff_idx].filename; gff_idx++) { ; }

    browse_render();
    //move_gff_cursor(1);
    //move_gff_cursor(1);
    //move_gff_cursor(1);
    //move_gff_cursor(1);
    //move_gff_cursor(1);
    //move_gff_cursor(1);
    //move_gff_cursor(1);
    //move_entry_cursor(-1);
    //move_entry_cursor(1);
    //move_entry_cursor(1);
    //move_entry_cursor(1);
    //move_entry_cursor(1);
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
        sol_print_line_len(0, str + s, x, y + (20 * s/w),
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
static void render_entry_psin();
static void render_entry_it1r();
static void render_entry_cact();
static void render_entry_scmd();
static void render_entry_gseq();
static void render_entry_lseq();
static void render_entry_pseq();
static void render_entry_bvoc();
static void render_entry_wind();
static void render_entry_apfm();

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
        case GFF_PSIN: render_entry_psin(); break;
        case GFF_IT1R: render_entry_it1r(); break;
        case GFF_CACT: render_entry_cact(); break;
        case GFF_SCMD: render_entry_scmd(); break;
        case GFF_GSEQ: render_entry_gseq(); break;
        case GFF_LSEQ: render_entry_lseq(); break;
        case GFF_PSEQ: render_entry_pseq(); break;
        case GFF_BVOC: render_entry_bvoc(); break;
        case GFF_WIND: render_entry_wind(); break;
        case GFF_APFM: render_entry_apfm(); break;
        default:
            render_entry_header();
            sol_print_line_len(0, "Need to implement", 320, 40, 128);
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
    sol_print_line_len(0, buf, 320, 20, BUF_MAX);
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
    sol_print_line_len(0, buf, x, y, BUF_MAX);
    unsigned char* data = gff_get_frame_rgba_palette_img(cimg, cframe, pal);
    if (!data) {
        sol_print_line_len(0, "BMP Data is not readable.", 320, 60, 128);
        return;
    }
    loc.w = gff_get_frame_width(gff_idx, type_id, res_id, cframe);
    loc.h = gff_get_frame_height(gff_idx, type_id, res_id, cframe);
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
    int pal_idx = gff_get_game_type() == DARKSUN_ONLINE
        ? RESFLOP_GFF_INDEX
        : RESOURCE_GFF_INDEX;
    gff_palette_t *pal = open_files[pal_idx].pals->palettes;

    if (open_files[gff_idx].pals->len) {
        pal = open_files[gff_idx].pals->palettes;
    }

    render_entry_as_image(gff_idx, GFF_BMP, res_ids[res_idx], pal, 320, 40);
}

static void render_entry_icon() {
    int pal_idx = gff_get_game_type() == DARKSUN_ONLINE
        ? RESFLOP_GFF_INDEX
        : RESOURCE_GFF_INDEX;
    render_entry_as_image(gff_idx, GFF_ICON, res_ids[res_idx], open_files[pal_idx].pals->palettes, 320, 40);
}

static void render_entry_port() {
    int pal_idx = gff_get_game_type() == DARKSUN_ONLINE
        ? RESFLOP_GFF_INDEX
        : RESOURCE_GFF_INDEX;
    render_entry_as_image(gff_idx, GFF_PORT, res_ids[res_idx], open_files[pal_idx].pals->palettes, 320, 40);
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
    sol_print_line_len(0, buf, 320, 20, BUF_MAX);
    gff_chunk_header_t chunk = gff_find_chunk_header(gff_idx, GFF_RDAT, res_ids[res_idx]);
    gff_read_chunk(gff_idx, &chunk, buf, BUF_MAX);
    sol_print_line_len(0, buf, 320, 40, chunk.length);
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
    sol_print_line_len(0, buf, 320, 20, BUF_MAX);
    sol_print_line_len(0, names + 25*res_idx, 320, 40, BUF_MAX);
}

static void render_entry_ojff() {
    disk_object_t disk_obj;
    gff_chunk_header_t chunk = gff_find_chunk_header(gff_idx, GFF_OJFF, res_ids[res_idx]);
    char buf[BUF_MAX];

    render_entry_header();

    gff_read_chunk(gff_idx, &chunk, &disk_obj, chunk.length);
    sol_print_line_len(0, "Disk Object:", 320, 40, BUF_MAX);
    snprintf(buf, BUF_MAX, "offset = (%d, %d)", disk_obj.xoffset, disk_obj.yoffset);
    sol_print_line_len(0, buf, 320, 60, BUF_MAX);
    snprintf(buf, BUF_MAX, "pos = (%d, %d, %d)", disk_obj.xpos, disk_obj.ypos, disk_obj.zpos);
    sol_print_line_len(0, buf, 320, 80, BUF_MAX);
    snprintf(buf, BUF_MAX, "flags = 0x%x", disk_obj.flags);
    sol_print_line_len(0, buf, 320, 100, BUF_MAX);
    snprintf(buf, BUF_MAX, "object_index = %d", disk_obj.object_index);
    sol_print_line_len(0, buf, 320, 120, BUF_MAX);
    snprintf(buf, BUF_MAX, "bmp_id = %d", disk_obj.bmp_id);
    sol_print_line_len(0, buf, 320, 140, BUF_MAX);
    snprintf(buf, BUF_MAX, "script_id = %d", disk_obj.script_id);
    sol_print_line_len(0, buf, 320, 160, BUF_MAX);
}

static void render_entry_as_pal(const int type_id) {
    render_entry_header();
    gff_palettes_t *pals = gff_read_palettes_type(gff_idx, type_id);
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

typedef struct monster_type_s {
    int16_t id;
    int16_t strength; // it looks like this is used to help calculate the strength
} monster_type_t;

typedef struct ds_region_mon_s {
    int16_t region;
    monster_type_t monsters[10];
} __attribute__ ((__packed__)) ds_region_mon_t;

// This is a big guess...
static void render_entry_monr() {
    char buf[BUF_MAX];
    int pos = 60;
    static ds_region_mon_t *mon = NULL;
    static int num_monsters = 0;
    int gff = gff_get_game_type() == DARKSUN_ONLINE
        ? RESFLOP_GFF_INDEX
        : RESOURCE_GFF_INDEX;

    render_entry_header();

    if (!mon) {
        gff_chunk_header_t chunk = gff_find_chunk_header(gff, GFF_MONR, 1);
        mon = malloc(chunk.length);
        gff_read_chunk(gff, &chunk, mon, chunk.length);
        num_monsters = chunk.length / sizeof(ds_region_mon_t);
    }

    if (mapy >= num_monsters) { mapy = 0; }
    if (mapy < 0) { mapy = num_monsters - 1; }

    snprintf(buf, BUF_MAX, "monster list %d of %d", mapy, num_monsters);
    sol_print_line_len(0, buf, 320, 40, BUF_MAX);

    for (int i = 0; i < 10; i++) {
        snprintf(buf, BUF_MAX, "region %d: = {id:%d, strength:%d}",
            mon[mapy].region, mon[mapy].monsters[i].id, mon[mapy].monsters[i].strength);
        sol_print_line_len(0, buf, 320, pos, BUF_MAX);
        pos += 20;
       // printf("%d: {%d, %d}\n", mon[0].region, mon[0].monsters[i].id, mon[0].monsters[i].strength);
    }
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
    sol_print_line_len(0, buf, 320, pos, 128); pos += 20;
    snprintf(buf, BUF_MAX, "HP: %d\n", combat.hp);
    sol_print_line_len(0, buf, 320, pos, 128); pos += 20;
    snprintf(buf, BUF_MAX, "PSP: %d\n", combat.psp);
    sol_print_line_len(0, buf, 320, pos, 128); pos += 20;
    //snprintf(buf, BUF_MAX, "Character Index: %d\n", combat.char_index);
    //sol_print_line_len(0, buf, 320, pos, 128); pos += 20;
    snprintf(buf, BUF_MAX, "ID: %d\n", combat.id);
    sol_print_line_len(0, buf, 320, pos, 128); pos += 20;
    //snprintf(buf, BUF_MAX, "Weapon: %d\n", combat.weapon_index);
    //sol_print_line_len(0, buf, 320, pos, 128); pos += 20;
    //snprintf(buf, BUF_MAX, "Packed: %d\n", combat.pack_index);
    //sol_print_line_len(0, buf, 320, pos, 128); pos += 20;
    snprintf(buf, BUF_MAX, "Special Attack: %d\n", combat.special_attack);
    sol_print_line_len(0, buf, 320, pos, 128); pos += 20;
    snprintf(buf, BUF_MAX, "Special Defense: %d\n", combat.special_defense);
    sol_print_line_len(0, buf, 320, pos, 128); pos += 20;
    snprintf(buf, BUF_MAX, "Icon: %d\n", combat.icon);
    sol_print_line_len(0, buf, 320, pos, 128); pos += 20;
    snprintf(buf, BUF_MAX, "AC: %d\n", combat.ac);
    sol_print_line_len(0, buf, 320, pos, 128); pos += 20;
    snprintf(buf, BUF_MAX, "move: %d\n", combat.move);
    sol_print_line_len(0, buf, 320, pos, 128); pos += 20;
    snprintf(buf, BUF_MAX, "status: %d\n", combat.status);
    sol_print_line_len(0, buf, 320, pos, 128); pos += 20;
    snprintf(buf, BUF_MAX, "allegiance: %d\n", combat.allegiance);
    sol_print_line_len(0, buf, 320, pos, 128); pos += 20;
    snprintf(buf, BUF_MAX, "data: %d\n", combat.data);
    sol_print_line_len(0, buf, 320, pos, 128); pos += 20;
    snprintf(buf, BUF_MAX, "thac0: %d\n", combat.thac0);
    sol_print_line_len(0, buf, 320, pos, 128); pos += 20;
    snprintf(buf, BUF_MAX, "priority: %d\n", combat.priority);
    sol_print_line_len(0, buf, 320, pos, 128); pos += 20;
    snprintf(buf, BUF_MAX, "str: %d, dex: %d\n, con: %d",
       combat.stats.str, combat.stats.dex, combat.stats.con);
    sol_print_line_len(0, buf, 320, pos, 128); pos += 20;
    snprintf(buf, BUF_MAX, "int: %d, wis: %d\n, cha: %d",
       combat.stats.intel, combat.stats.wis, combat.stats.cha);
    sol_print_line_len(0, buf, 320, pos, 128); pos += 20;
    sol_print_line_len(0, "Not Shown: weapon, packed, character id", 320, pos, 128); pos += 20;
}

static void print_item(ds1_item_t item, int pos) {
    char buf[BUF_MAX];
    static char names[NAME_MAX];

    gff_chunk_header_t chunk = gff_find_chunk_header(DSLDATA_GFF_INDEX, GFF_NAME, 1);
    gff_read_chunk(DSLDATA_GFF_INDEX, &chunk, names, NAME_MAX);

    FILE *f = fopen("out.dat", "wb+");
    fwrite(names, 1, chunk.length, f);
    fclose(f);

    snprintf(buf, BUF_MAX, "id: %d\n", item.id);
    sol_print_line_len(0, buf, 320, pos, 128); pos += 20;
    snprintf(buf, BUF_MAX, "quantity: %d\n", item.quantity);
    sol_print_line_len(0, buf, 320, pos, 128); pos += 20;
    snprintf(buf, BUF_MAX, "next: %d\n", item.next);
    sol_print_line_len(0, buf, 320, pos, 128); pos += 20;
    snprintf(buf, BUF_MAX, "value: %d\n", item.value);
    sol_print_line_len(0, buf, 320, pos, 128); pos += 20;
    snprintf(buf, BUF_MAX, "pack index: %d\n", item.pack_index);
    sol_print_line_len(0, buf, 320, pos, 128); pos += 20;
    snprintf(buf, BUF_MAX, "item index: %d\n", item.item_index);
    sol_print_line_len(0, buf, 320, pos, 128); pos += 20;
    snprintf(buf, BUF_MAX, "Icon: %d\n", item.icon);
    sol_print_line_len(0, buf, 320, pos, 128); pos += 20;
    snprintf(buf, BUF_MAX, "charges: %d\n", item.charges);
    sol_print_line_len(0, buf, 320, pos, 128); pos += 20;
    snprintf(buf, BUF_MAX, "special: %d\n", item.special);
    sol_print_line_len(0, buf, 320, pos, 128); pos += 20;
    snprintf(buf, BUF_MAX, "priority: %d\n", item.priority);
    sol_print_line_len(0, buf, 320, pos, 128); pos += 20;
    snprintf(buf, BUF_MAX, "slot: %d\n", item.slot);
    sol_print_line_len(0, buf, 320, pos, 128); pos += 20;

    if ((item.name_idx * 25) < NAME_MAX) {
        snprintf(buf, BUF_MAX, "name_index: %d ( %s)\n", item.name_idx, names + 25 * item.name_idx);
    } else {
        snprintf(buf, BUF_MAX, "name_index: %d\n", item.name_idx);
    }
    sol_print_line_len(0, buf, 320, pos, 128); pos += 20;
    snprintf(buf, BUF_MAX, "bonus: %d\n", item.bonus);
    sol_print_line_len(0, buf, 320, pos, 128); pos += 20;
    /*
    sol_print_line_len(0, "Not Shown: weapon, packed, character id", 320, pos, 128); pos += 20;
    */
}

#define RDFF_MAX (1<<12)
static void render_entry_rdff() {
    char buf[BUF_MAX];
    uint32_t len = 0;
    render_entry_header();
    so_object_t *so = NULL;
    rdff_disk_object_t rdff_buf[RDFF_MAX];
    rdff_disk_object_t *rdff = rdff_buf;
    rdff_disk_object_t *next = rdff;
    gff_chunk_header_t chunk = gff_find_chunk_header(gff_idx, GFF_RDFF, res_ids[res_idx]);

    if (chunk.length > RDFF_MAX * sizeof(rdff_disk_object_t)) {
        error ("chunk.length (%d) greater than max (%d)!\n", chunk.length, RDFF_MAX);
        exit(1);
    }

    gff_read_chunk(gff_idx, &chunk, rdff, chunk.length);
    //printf("res_ids[%d] = %d\n", res_idx, res_ids[res_idx]);
    snprintf(buf, BUF_MAX, "load action: %d\n", rdff->load_action);
    sol_print_line_len(0, buf, 320, 40, 128);
    snprintf(buf, BUF_MAX, "blocknum: %d\n", rdff->blocknum);
    sol_print_line_len(0, buf, 320, 60, 128);
    snprintf(buf, BUF_MAX, "type: %s\n", rdff_type_names[rdff->type]);
    sol_print_line_len(0, buf, 320, 80, 128);
    snprintf(buf, BUF_MAX, "index: %d\n", rdff->index);
    sol_print_line_len(0, buf, 320, 100, 128);
    snprintf(buf, BUF_MAX, "from: %d\n", rdff->index);
    sol_print_line_len(0, buf, 320, 120, 128);
    snprintf(buf, BUF_MAX, "len: %d\n", rdff->len);
    sol_print_line_len(0, buf, 320, 140, 128);
    sol_print_line_len(0, "-----------------------", 320, 160, 128);
    sol_print_line_len(0, "Jumping to entry:", 320, 180, 128);

    switch(rdff->load_action) {
        case RDFF_OBJECT:
        case RDFF_CONTAINER: // I don't know the different between Container and Object!
            rdff++;
            so = gff_create_object((char*)rdff, rdff - 1, -1);
            snprintf(buf, BUF_MAX, "index: %d\n", rdff->index);
            sol_print_line_len(0, buf, 320, 200, 128);
            snprintf(buf, BUF_MAX, "type: %s\n", so_object_names[so->type]);
            sol_print_line_len(0, buf, 320, 220, 128);
            if (so->type == SO_DS1_COMBAT) { print_combat(so->data.ds1_combat, 240); }
            if (so->type == SO_DS1_ITEM) { print_item(so->data.ds1_item, 240); }
            break;
        default:
            snprintf(buf, BUF_MAX, "unknown type: %d\n", rdff->type);
            sol_print_line_len(0, buf, 320, 220, 128);
            break;
    }
    printf("chunk.length = %d\n", chunk.length);
    // There appears to be more, but I don't know them all...
    printf("---------START-------------, total len = %d\n", chunk.length);
    len += sizeof(rdff_disk_object_t) + next->len;
    len = 0;
    //len = rdff->index + sizeof(rdff_disk_object_t);
    //while (len < chunk.length) {
    for (int i = 0; i < 305 && len < chunk.length; i++) {
        next = (rdff_disk_object_t*)((char*)(rdff_buf) + len);
        switch(next->load_action) {
            case RDFF_OBJECT:
                printf("OBJECT\n");
                break;
            case RDFF_CONTAINER:
                printf("CONTAINER\n");
                break;
            case RDFF_DATA:
                printf("DATA\n");
                if (next->type == CHAR_OBJECT) {
                    ds_character_t *ch = (ds_character_t*)((char*)(rdff_buf) + len + sizeof(rdff_disk_object_t));
                    printf("ch: base_move = %d, ac = %d\n", ch->base_move, ch->base_ac);
                }
                break;
            case RDFF_NEXT:
                printf("NEXT\n");
                break;
            case RDFF_END:
                printf("END\n");
                break;
        }
        len += sizeof(rdff_disk_object_t) + next->len;
        printf("next = {la = %d, blockNum = %d, len = %d, type = %d, index = %d}\n", next->load_action, next->blocknum, next->len, next->type, next->index);
    }
    //next = (rdff_disk_object_t*)(((char*)next) + sizeof(rdff_disk_object_t) + next->len);
    /*
    rdff++;
    while (rdff->load_action != -1 && rdff->load_action < 5) {
        snprintf(buf, BUF_MAX, "load action: %d\n", rdff->load_action);
        sol_print_line_len(0, buf, 320, pos, 128);
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
        sol_print_line_len(0, "ERROR font length > font buf, need to fix!", 340, 20, BUF_MAX);
        return;
    }
    gff_read_chunk(gff_idx, &chunk, font, chunk.length);
    snprintf(buf, BUF_MAX, "RESOURCE %d of %d \n", res_idx, res_max - 1);
    sol_print_line_len(0, buf, 320, 20, BUF_MAX);
    for (unsigned short c = 0; c < 255; c++) {
        ds_char_t *ds_char = (ds_char_t*)(((uint8_t*)font) + *(font->char_offset + c));
        if (ds_char->width) {
            char *data = (char*)gff_create_font_rgba(gff_idx, c, 0xFFFF00FF, 0x000000FF);
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

static struct sol_region_s *region = NULL;;
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
        region = region_manager_get_region(res_ids[res_idx]);
        tiles_len = region->num_tiles + 1;
        tiles = (SDL_Texture**) malloc(sizeof(SDL_Texture*) * (tiles_len));
        memset(tiles, 0x0, sizeof(SDL_Texture*) * tiles_len);
        for (uint32_t i = 0; i < region->num_tiles; i++) {
            region_get_tile(region, i, &width, &height, &data);

            tile = SDL_CreateRGBSurfaceFrom(data, width, height, 32, 4*width, 0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);
            //if (region->ids[i] > (tiles_len)) {
                //error ("region->ids[%d] is out of bounds!\n", i);
                //exit(1);
            //}
            //tiles[region->ids[i]] = SDL_CreateTextureFromSurface(renderer, tile);
            tiles[region->tile_ids[i]] = SDL_CreateTextureFromSurface(renderer, tile);
            SDL_FreeSurface(tile);

            free(data);
        }
        unsigned int *gmap_ids = gff_get_id_list(region->gff_file, GFF_GMAP);
        gff_chunk_header_t chunk = gff_find_chunk_header(region->gff_file, GFF_GMAP, gmap_ids[0]);
        if (!gff_read_chunk(region->gff_file, &chunk, region->flags, chunk.length)) {
            error ("Unable to read GFF_GMAP chunk!\n");
            exit(1);
        }

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
            size_t tile_id = region->tiles[i][j];
            SDL_RenderCopy(renderer, tiles[tile_id], NULL, &loc);
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
            if (region_is_block(region, i, j)) {
                //printf("(%d, %d)\n, ", i, j);
                SDL_SetRenderDrawColor(renderer, 0xFF, 0x00, 0x00, SDL_ALPHA_OPAQUE);
            /*} else if (dsl_region_is_actor(region, i, j)) {
                SDL_SetRenderDrawColor(renderer, 0x00, 0xFF, 0x00, SDL_ALPHA_OPAQUE);*/
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
    int gff = gff_get_game_type() == DARKSUN_ONLINE
        ? RESFLOP_GFF_INDEX
        : RESOURCE_GFF_INDEX;

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
    sol_print_line_len(0, buf, 320, 40, BUF_MAX);
    snprintf(buf, BUF_MAX, "pos: (%d, %d, %d)\n", obj->xpos, obj->ypos, obj->zpos);
    sol_print_line_len(0, buf, 320, 60, BUF_MAX);
    snprintf(buf, BUF_MAX, "flags: 0x%x\n", obj->flags);
    sol_print_line_len(0, buf, 320, 80, BUF_MAX);

    if (!gff_read_object(obj->index, &dobj)) {
        snprintf(buf, BUF_MAX, "Unable to read obj: %d\n", obj->index);
        sol_print_line_len(0, buf, 320, 80, BUF_MAX);
        return;
    }
    snprintf(buf, BUF_MAX, "disk object[%d]: (object_index not displayed.)\n", obj->index);
    sol_print_line_len(0, buf, 320, 100, BUF_MAX);
    snprintf(buf, BUF_MAX, "    .flags = 0x%x\n", dobj.flags);
    sol_print_line_len(0, buf, 320, 120, BUF_MAX);
    snprintf(buf, BUF_MAX, "    .offset = (%d, %d)\n", dobj.xoffset, dobj.yoffset);
    sol_print_line_len(0, buf, 320, 140, BUF_MAX);
    snprintf(buf, BUF_MAX, "    .pos = (%d, %d, %d)\n", dobj.xpos, dobj.ypos, dobj.zpos);
    sol_print_line_len(0, buf, 320, 160, BUF_MAX);
    snprintf(buf, BUF_MAX, "    .script_id = %d\n", dobj.script_id);
    sol_print_line_len(0, buf, 320, 180, BUF_MAX);
    snprintf(buf, BUF_MAX, "    .bmp_id = %d\n", dobj.bmp_id);
    sol_print_line_len(0, buf, 320, 200, BUF_MAX);
    gff_palette_t *pal = open_files[gff].pals->palettes;
    render_entry_as_image(OBJEX_GFF_INDEX, GFF_BMP, dobj.bmp_id, pal, 340, 220);
}

static void render_entry_spst() {
    char name[32];
    char buf[BUF_MAX];
    size_t pos;
    ssi_spell_list_t spells;
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

enum {
    CLASS_AIR         = (1<<0),
    CLASS_EARTH       = (1<<1),
    CLASS_FIRE        = (1<<2),
    CLASS_WATER       = (1<<3),
    CLASS_DRUID       = (1<<4),
    CLASS_FIGHER      = (1<<5),
    CLASS_GLADIATOR   = (1<<6),
    CLASS_PRESERVER   = (1<<7),
    CLASS_PSIONICIST  = (1<<8),
    CLASS_RANGER      = (1<<9),
    CLASS_ROGUE       = (1<<10),
    CLASS_DEFILER     = (1<<11),
    CLASS_TEMPLAR     = (1<<12),
    CLASS_ALL_CLERIC  = (1<<13),
};

static const char *class_names[] = {
    "AIR CLERIC",
    "EARTH CLERIC",
    "FIRE CLERIC",
    "WATER CLERIC",
    "DRUID",
    "FIGHTER",
    "GLADIATOR",
    "PRESERVER",
    "PSIONICIST",
    "RANGER",
    "ROGUE",
    "DEFILER",
    "TEMPLAR",
};
/*

static const char *race_names[] = {
    "MONSTER",
    "HUMAN",
    "DWARF",
    "ELF",
    "HALFELF",
    "HALFGIANT",
    "HALFLING",
    "MUL",
    "THRI-KREEN",
};


static const char *alignment_names[] = {
    "LAWFUL GOOD",
    "LAWFUL NEUTRAL",
    "LAWFUL EVIL",
    "NEUTRAL_GOOD",
    "TRUE NEUTRAL",
    "NEUTRAL EVIL",
    "CHAOTIC GOOD",
    "CHAOTIC NEUTRAL",
    "CHAOTIC EVIL",
};

static const char *real_class_names[] = {
    "NONE",
    "AIR CLERIC",
    "EARTH CLERIC",
    "FIRE CLERIC",
    "WATER CLERIC",
    "AIR DRUID",
    "EARTH DRUID",
    "FIRE DRUID",
    "WATER DRUID",
    "FIGHTER",
    "GLADIATOR",
    "PRESERVER",
    "PSIONICIST",
    "AIR RANGER",
    "EARTH RANGER",
    "FIRE RANGER",
    "WATER RANGER",
    "ROGUE",
    "DEFILER",
    "TEMPLAR",
};
*/

const char *rdff_actions[] = {
    "NOTHING",
    "OBJECT",
    "CONTIANER",
    "DATA",
    "NEXT",
};

const char *rdff_types[] = {
    "NOTHING",
    "OBJECT",
    "COMBAT",
    "CHARREC",
    "ITEM1R",
    "MINI",
};

const char *slot_names[] = {
    "ARM",
    "AMMO",
    "MISSLE",
    "HAND1",
    "FINGER",
    "WAIST",
    "LEGS",
    "HEAD",
    "NECK",
    "CHEST",
    "HAND2",
    "FINGER1",
    "CLOAK",
    "FOOT",
    "B00",
    "B01",
    "B02",
    "B03",
    "B04",
    "B05",
    "B07",
    "B08",
    "B09",
    "B12",
    "B13",
    "B14",
};

static void render_entry_char() {
    char buf[BUF_MAX];
    rdff_header_t *rdff;
    size_t offset = 0;
    gff_chunk_header_t chunk = gff_find_chunk_header(gff_idx, GFF_CHAR, res_ids[res_idx]);
    gff_read_chunk(gff_idx, &chunk, &buf, sizeof(buf));
    rdff = (rdff_disk_object_t*) (buf);
    //int8_t  load_action;
    //int8_t  blocknum;
    //int16_t type;
    //int16_t index;
    //int16_t from;
    //int16_t len;

    printf("action = %d(%s),", rdff->load_action, rdff_actions[rdff->load_action]);
    printf("blocknum = %d,", rdff->blocknum);
    printf("type = %d(%s),", rdff->type, rdff_types[rdff->type]);
    printf("index = %d,", rdff->index);
    printf("from = %d,", rdff->from);
    printf("len = %d\n", rdff->len);
    ds1_combat_t *combat = (ds1_combat_t*)(buf + 10);
   // printf("combat->hp = %d\n", combat->hp);
    print_combat(*combat, 40);
    offset += 10 + rdff->len;
    rdff = (rdff_disk_object_t*)(((char*)(rdff)) + 10 + rdff->len);
    printf("action = %d(%s),", rdff->load_action, rdff_actions[rdff->load_action]);
    printf("blocknum = %d,", rdff->blocknum);
    printf("type = %d(%s),", rdff->type, rdff_types[rdff->type]);
    printf("index = %d,", rdff->index);
    printf("from = %d,", rdff->from);
    printf("len = %d\n", rdff->len);
    /*
    ds_character_t *character = (ds_character_t*)(buf + 0x4E);
    printf("exp = %d, %d\n", character->current_xp, character->high_xp);
    printf("hp = base = %d, high = %d\n", character->base_hp, character->high_hp);
    printf("base_psp = %d, id = %d\n", character->base_psp, character->id);
    for (int i = 0; i < 12; i++) {
        if ((character->legal_class >> (i)) & 0x0001) {
            //printf("%s\n", class_names[0]);
            printf("%s, ", class_names[i]);
        }
    }
    printf("\n");
    printf("data1[0] = %d, data1[1] = %d\n", character->data1[0], character->data1[1]);
    */
    /*
    printf("%s\n", (character->gender == 1) ? "MALE" : "FEMALE");
    //printf("%s\n", race_names[character->race]);
    //printf("%s\n", alignment_names[character->alignment]);
    printf("stats = %d, %d, %d, %d, %d, %d\n",
            character->stats.str,
            character->stats.dex,
            character->stats.con,
            character->stats.intel,
            character->stats.wis,
            character->stats.cha
          );
    printf("classes = {%d, %d, %d}\n", character->real_class[0],
            character->real_class[1],
            character->real_class[2]);
    //printf("classes = {%s, %s, %s}\n", real_class_names[character->real_class[0]],
            //real_class_names[character->real_class[1]],
            //real_class_names[character->real_class[2]]);
    printf("level = {%d, %d, %d}\n", character->level[0], character->level[1], character->level[2]);
    printf("base_ac = %d, base_move = %d, magic_res = %d\n", character->base_ac, character->base_move,
    character->magic_resistance);
    printf("num_blows = %d\n", character->num_blows);
    printf("num_attacks = {%d, %d, %d}\n", character->num_attacks[0], character->num_attacks[1],
    character->num_attacks[2]);
    printf("num_dice = {%d, %d, %d}\n", character->num_dice[0], character->num_dice[1],
    character->num_dice[2]);
    printf("num_sides = {%d, %d, %d}\n", character->num_sides[0], character->num_sides[1],
    character->num_sides[2]);
    printf("num_bonuses = {%d, %d, %d}\n", character->num_bonuses[0], character->num_bonuses[1],
    character->num_bonuses[2]);
    printf("saving throws: %d %d %d %d %d\n",
        character->saving_throw.paral,
        character->saving_throw.wand,
        character->saving_throw.petr,
        character->saving_throw.breath,
        character->saving_throw.spell);
    printf("allegiance = %d, size = %d, spell_group = %d\n",
        character->allegiance, character->size, character->spell_group);
    printf("high_level = {%d, %d, %d}\n",
        character->high_level[0], character->high_level[1], character->high_level[2]);
    printf("sound = %d, attack_sound = %d, psi_group = %d, palette = %d\n",
        character->sound_fx,character->attack_sound,character->psi_group,character->palette);
    */

/*
    offset += 10 + rdff->len;
    rdff = (rdff_disk_object_t*)(((char*)(rdff)) + 10 + rdff->len);
    printf("action = %d(%s),", rdff->load_action, rdff_actions[rdff->load_action]);
    printf("blocknum = %d,", rdff->blocknum);
    printf("type = %d(%s),", rdff->type, rdff_types[rdff->type]);
    printf("index = %d,", rdff->index);
    printf("from = %d,", rdff->from);
    printf("len = %d\n", rdff->len);
    ds1_item_t *item = (ds1_item_t*)(((char*)(rdff)) + 10);
    printf("id = %d, qty = %d, next = %d,", item->id, item->quantity, item->next);
    printf("value = %d, pack = %d, item = %d,", item->value, item->pack_index, item->item_index);
    printf("icon = %d, charges = %d, special = %d,", item->icon, item->charges, item->special);
    printf("priority = %d, slot = %d, name_index = %d,", item->priority, item->slot, item->name_idx);
    printf("bonus = %d", item->bonus);
    printf("\n");
    */
    offset += 10 + rdff->len;
    while (offset < chunk.length) {
        rdff = (rdff_disk_object_t*)(((char*)(rdff)) + 10 + rdff->len);
        if (rdff->type == 1) {
            ds1_item_t *item = (ds1_item_t*)(((char*)(rdff)) + 10);
            printf("id = %d, qty = %d, next = %d,", item->id, item->quantity, item->next);
            printf("value = %d, pack = %d, item = %d,", item->value, item->pack_index, item->item_index);
            printf("icon = %d, charges = %d, special = 0x%x,", item->icon, item->charges, item->special);
            printf("priority = 0x%x, slot = %d(%s), name_index = %d,", item->priority, item->slot, slot_names[item->slot], item->name_idx);
            printf("bonus = %d, data0 = %d", item->bonus, item->data0);
            printf("\n");
        }
        offset += 10 + rdff->len;
    }
    render_entry_header();
    //printf("offset = %ld\n", offset);
    printf("------------------------------------\n");
}

static void render_entry_psin() {
    char name[32];
    char buf[BUF_MAX];
    size_t pos = 0;
    psin_t psin;
    gff_chunk_header_t chunk = gff_find_chunk_header(gff_idx, GFF_PSIN, res_ids[res_idx]);
    gff_read_chunk(gff_idx, &chunk, &psin, sizeof(psin));

    render_entry_header();

    for (int i = 0; i < 3; i++) {
        if (psin.types[2*i]) {
            //printf("i = %d\n", i);
            spell_get_psin_name(i, name);
            pos += snprintf(buf + pos, BUF_MAX - pos, "%s, ", name);
        }
    }
    pos += snprintf(buf + pos, BUF_MAX - pos, "\n");
    print_para_len(renderer, buf, 320, 40, 40, pos);
    /*
    printf("0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x\n",
        psin.types[0],
        psin.types[1],
        psin.types[2],
        psin.types[3],
        psin.types[4],
        psin.types[5],
        psin.types[6]);
    */
}


static void render_entry_it1r() {
    char buf[BUF_MAX];
    char it1rs[1<<14];
    size_t pos = 0;
    //rdff_header_t *rdff;
    //size_t offset = 0;
    gff_chunk_header_t chunk = gff_find_chunk_header(gff_idx, GFF_IT1R, res_ids[res_idx]);
    size_t amt = gff_read_chunk(gff_idx, &chunk, &it1rs, sizeof(it1rs));
    if (mapy < 0) {
        mapy = amt / sizeof(ds1_it1r_t) - 1;
    }
    if (mapy >= amt / sizeof(ds1_it1r_t)) {
        mapy = 0;
    }
    //rdff = (rdff_disk_object_t*) (buf);
    //printf("action = %d(%s),", rdff->load_action, rdff_actions[rdff->load_action]);
    //printf("blocknum = %d,", rdff->blocknum);
    //printf("type = %d(%s),", rdff->type, rdff_types[rdff->type]);
    //printf("index = %d,", rdff->index);
    //printf("from = %d,", rdff->from);
    //printf("len = %d\n", rdff->len);
    if (mapy >= 0) {
        ds1_it1r_t *it1r = (ds1_it1r_t*)(it1rs + (mapy * sizeof(ds1_it1r_t)));
        render_entry_header();

        //snprintf(buf, BUF_MAX, "entry %d of %ld", mapy, (uint32_t)(amt/sizeof(ds1_it1r_t)));
        snprintf(buf, BUF_MAX, "entry %d of %ld", mapy, (long int)(amt/sizeof(ds1_it1r_t)));
        sol_print_line_len(0, buf, 320, 40, BUF_MAX);

        pos = snprintf(buf, BUF_MAX, "Weapon and damage: ");
        for (int i = 1; i < 6; i++) {
            if (it1r->weapon_type & (1 << (i-1))) {
                pos += snprintf(buf + pos, BUF_MAX - pos, "%s, ", weapon_names[i]);
            }
        }
        for (int i = 0; i < 14; i++) {
            if (it1r->damage_type & (1 << (i))) {
                pos += snprintf(buf + pos, BUF_MAX - pos, "%s, ", damage_types[i]);
            }
        }
        sol_print_line_len(0, buf, 320, 60, BUF_MAX);

        pos = snprintf(buf, BUF_MAX, "weight: %d, base_hp: %d",
                it1r->weight, it1r->base_hp);
        sol_print_line_len(0, buf, 320, 80, BUF_MAX);

        pos = snprintf(buf, BUF_MAX, "Material:");
        pos += snprintf(buf + pos, BUF_MAX - pos, "%s %s %s ", material_types[it1r->material & 0x0F],
                it1r->material & 0x40 ? "NO Material" : "",
                it1r->material & 0x80 ? "NO Effect" : ""
                );
        sol_print_line_len(0, buf, 320, 100, BUF_MAX);

        pos = snprintf(buf, BUF_MAX, "Placement: %s", placement_types[it1r->placement]);
        sol_print_line_len(0, buf, 320, 120, BUF_MAX);

        snprintf(buf, BUF_MAX, "%d%s x %dD%d + %d\n",
                (it1r->num_attacks >> 1),
                (it1r->num_attacks & 0x01) ? ".5" : "",
                it1r->dice,
                it1r->sides,
                it1r->mod);
        sol_print_line_len(0, buf, 320, 140, BUF_MAX);

        pos = snprintf(buf, BUF_MAX, "Flags:");
        for (int i = 0; i < 9; i++) {
            if (it1r->flags & (1 << (i))) {
                pos += snprintf(buf + pos, BUF_MAX - pos, "%s, ", it1r_flags[i]);
            }
        }
        sol_print_line_len(0, buf, 320, 160, BUF_MAX);

        pos = snprintf(buf, BUF_MAX, "Legal Classes:");
        for (int i = 0; i < 13; i++) {
            if (it1r->legal_class & (1 << (i))) {
                pos += snprintf(buf + pos, BUF_MAX - pos, "%s, ", class_names[i]);
            }
        }
        sol_print_line_len(0, buf, 320, 180, BUF_MAX);

        pos = snprintf(buf, BUF_MAX, "base AC: %d", (int32_t)it1r->base_AC);
        sol_print_line_len(0, buf, 320, 200, BUF_MAX);

        //pos = snprintf(buf, BUF_MAX, "data0: %d", it1r->data0);
        //sol_print_line_len(0, buf, 320, 220, BUF_MAX);
        pos = snprintf(buf, BUF_MAX, "data1: %d (0x%x)", it1r->data1, it1r->data1);
        sol_print_line_len(0, buf, 320, 220, BUF_MAX);
        pos = snprintf(buf, BUF_MAX, "data2: %d", it1r->data2);
        sol_print_line_len(0, buf, 320, 240, BUF_MAX);
    }
}

static void render_entry_cact() {
    char buf[BUF_MAX];
    int16_t id;
    gff_chunk_header_t chunk = gff_find_chunk_header(gff_idx, GFF_CACT, res_ids[res_idx]);
    gff_read_chunk(gff_idx, &chunk, &id, sizeof(id));

    render_entry_header();

    snprintf(buf, BUF_MAX, "id: %d, [%d, %d]\n", id, (id >> 8) & 0xFF, id & 0xFF);
    sol_print_line_len(0, buf, 320, 40, BUF_MAX);
}

static void render_entry_scmd() {
    render_entry_header();
    char buf[BUF_MAX];
    int num_entries = 0;
    const int max_entries = 36;
    int ypos = 40;
    scmd_t* scmd = ssi_scmd_get(gff_idx, res_ids[res_idx], 0);
    while (num_entries < max_entries && scmd->flags != SCMD_JUMP) {
        snprintf(buf, BUF_MAX, "%c%c: bmp: %d, delay: %d, offset: (%d, %d), hot: (%d, %d)", 
            scmd->flags & SCMD_LAST ? 'L' : ' ',
            scmd->flags & SCMD_JUMP ? 'J' : ' ',
            scmd->bmp_idx, scmd->delay, scmd->xoffset, scmd->yoffset,
            scmd->xoffsethot, scmd->yoffsethot);
        sol_print_line_len(0, buf, 320, ypos, BUF_MAX);
        ypos += 12;
        snprintf(buf, BUF_MAX, "        sound: %d, flags = 0x%x",
            scmd->soundidx, scmd->flags);
        sol_print_line_len(0, buf, 320, ypos, BUF_MAX);
        ypos += 12;
        num_entries++;
        scmd++;
    }
    snprintf(buf, BUF_MAX, "%c%c: bmp: %d, delay: %d, offset: (%d, %d), hot: (%d, %d)", 
        scmd->flags & SCMD_LAST ? 'L' : ' ',
        scmd->flags & SCMD_JUMP ? 'J' : ' ',
        scmd->bmp_idx, scmd->delay, scmd->xoffset, scmd->yoffset,
        scmd->xoffsethot, scmd->yoffsethot);
    sol_print_line_len(0, buf, 320, ypos, BUF_MAX);
    //printf("num_entries = %d\n", num_entries);
}

static uint32_t music_type = 0;

static void play_xmi() {
    switch(music_type) {
        case GFF_GSEQ:
        case GFF_LSEQ:
        case GFF_PSEQ:
            sol_audio_play_xmi(gff_idx, music_type, res_ids[res_idx]);
            break;
        case GFF_BVOC:
            sol_audio_play_voc(gff_idx, music_type, res_ids[res_idx], 0.5);
            break;
    }
}

void render_entry_gseq() {
    render_entry_header();
    music_type = GFF_GSEQ;
    sol_print_line_len(0, "Press p to play.", 320, 60, 128);
}

void render_entry_lseq() {
    render_entry_header();
    music_type = GFF_LSEQ;
    sol_print_line_len(0, "Press p to play.", 320, 60, 128);
}

void render_entry_pseq() {
    render_entry_header();
    music_type = GFF_PSEQ;
    sol_print_line_len(0, "Press p to play.", 320, 60, 128);
}

void render_entry_bvoc() {
    render_entry_header();
    music_type = GFF_BVOC;
    sol_print_line_len(0, "Press p to play.", 320, 60, 128);
}

typedef struct resource_header_s {
    uint32_t type;
    uint32_t len;
    uint32_t id;
} resource_header_t;

typedef struct gui_rect_s {
    int16_t xmin, ymin;
    int16_t xmax, ymax;
} gui_rect_t;

typedef struct gui_region_s {
    uint16_t depth;
    gui_rect_t bounds;
    gui_rect_t regions[16];
} gui_region_t;

typedef struct gui_frame_s {
    uint16_t flags;
    uint16_t type;
    gui_rect_t bounds;
    gui_rect_t initbounds;
    gui_rect_t zonebounds;
    uint16_t width;
    uint16_t height;
    uint32_t border_bmp;
    int8_t data[4];
    uint32_t background_bmp;
    int8_t data1[4];
    char title[24];
} __attribute__ ((__packed__)) gui_frame_t;

typedef struct gui_window_s {
    resource_header_t rh;
    gui_region_t region;
    int16_t x;
    int16_t y;
    int8_t data[4];
    uint16_t flags;
    short data1;
    gui_frame_t  frame;
    int16_t offsetx;
    int16_t offsety;
    int8_t data2[4];
    uint8_t titleLen;
    uint16_t itemCount;
    uint32_t mem; // total mem needed
} __attribute__ ((__packed__)) gui_window_t;

typedef struct gui_item_s {
    int8_t data[4];
    uint32_t type;
    uint32_t id;
    gui_rect_t init_bounds;
    gui_rect_t item_bounds;
    uint16_t flags;
} __attribute__ ((__packed__)) gui_item_t;

void render_entry_wind() {
    render_entry_header();
    char buf[4096];
    gui_window_t *wind = NULL;

    gff_chunk_header_t chunk = gff_find_chunk_header(gff_idx, GFF_WIND, res_ids[res_idx]);
    if (!gff_read_chunk(gff_idx, &chunk, buf, 4096)) {
        printf("ERROR.\n");
        return;
    }

    wind = (gui_window_t*) buf;
    printf("->(%d, %d, %d)\n", wind->rh.id, wind->rh.len, wind->rh.type);
    printf("->(%d, %d)\n", wind->x, wind->y);
    printf("->%d:%d\n", wind->frame.width, wind->frame.height);
    printf("-> %d\n", wind->itemCount);
    printf("-> border: %d, background: %d\n", wind->frame.border_bmp, wind->frame.background_bmp);
    printf("-> len: %d\n", wind->titleLen);
    printf("----%ld----\n", sizeof(gui_window_t) + wind->titleLen);
    for (int i = 0; i < wind->itemCount; i++) {
        gui_item_t *item = (gui_item_t*) (buf + (sizeof(gui_window_t) + 12) + i * sizeof(gui_item_t));
        printf("item: %d, %d\n", item->type, item->id);
    }
    //printf("num items = %d\n", wind->itemCount);
}

typedef struct gui_app_frame_s {
    resource_header_t rh;
    gui_frame_t frame;
    uint8_t data[4];
    int16_t event_filter;
    uint8_t data1[4];
    uint8_t data2[4];
    uint8_t data3[4];
    int16_t snapMode;
    int16_t snapOriginX;
    int16_t snapOriginY;
    int16_t snapSizeX;
    int16_t snapSizeY;
    uint8_t data4[4];
} __attribute__ ((__packed__)) gui_app_frame_t;

void render_entry_apfm() {
    render_entry_header();
    char buf[4096];
    gui_app_frame_t *frame = NULL;

    gff_chunk_header_t chunk = gff_find_chunk_header(gff_idx, GFF_APFM, res_ids[res_idx]);
    if (!gff_read_chunk(gff_idx, &chunk, buf, 4096)) {
        printf("ERROR.\n");
        return;
    }
    frame = (gui_app_frame_t*) buf;

    printf("-> %d, %d, %d\n", frame->rh.id, frame->rh.len, frame->rh.type);
    printf("-> %d: %d\n", frame->frame.width, frame->frame.height);
    printf("background: %d, border: %d\n", frame->frame.background_bmp, frame->frame.border_bmp);
}
