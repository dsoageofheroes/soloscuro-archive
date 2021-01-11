#include <SDL2/SDL.h>
#include <stdio.h>
#include "screens/narrate.h"
#include "../src/gameloop.h"
#include "../src/dsl.h"
#include "../src/gff.h"
#include "../src/gff-map.h"
#include "../src/gff-image.h"

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

static void browse_tick();
static void browse_handle_input();
static void browse_render();
static void move_gff_cursor(int amt);
static void print_gff_entries();
static void render_entry();
static void move_entry_cursor(int amt);
static void write_blob();

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
                    case SDLK_RIGHT: res_idx = (res_idx + 1) % res_max; break;
                    case SDLK_LEFT: res_idx = (res_max + res_idx - 1) % res_max; break;
                    case SDLK_w: write_blob(); break;
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
    move_entry_cursor(0); // update entry res points.
}

static void print_menu() {
    int row = 0;
    SDL_Rect rect;

    print_line_len(renderer, "GFFs:", 20, 20, 1<<12);

    row_max = 0;
    for (int i = 0; i < NUM_FILES; i++) {
        if (open_files[i].filename) {
            print_line_len(renderer, open_files[i].filename, 20, 40 + row++ * 20, 1<<12);
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
    print_line_len(renderer, buf, 220, 20, BUF_MAX);
    for (int i = 0; i < entry_max; i++) {
        get_gff_type_name(gff_get_type_id(gff_idx, i), buf);
        buf[4] = '\0';
        print_line_len(renderer, buf, 220, 40 + 20 * i, BUF_MAX);
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
    browse_render();
    //move_gff_cursor(1);
    move_entry_cursor(1);
    move_entry_cursor(1);
    move_entry_cursor(1);
    move_entry_cursor(1);
    move_entry_cursor(1);
    move_entry_cursor(1);
    move_entry_cursor(1);
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
        print_line_len(renderer, str + s, x, y + (20 * s/w),
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
        default:
            render_entry_header();
            print_line_len(renderer, "Need to implement", 320, 40, 128);
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
    print_line_len(renderer, buf, 320, 20, BUF_MAX);
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

static void render_entry_bmp() {
    render_entry_header();
    SDL_Rect loc;

    unsigned char* data = get_frame_rgba_with_palette(gff_idx, GFF_BMP, res_ids[res_idx], 0, 0);
    loc.w = get_frame_width(gff_idx, GT_BMP, res_ids[res_idx], 0);
    loc.h = get_frame_height(gff_idx, GT_BMP, res_ids[res_idx], 0);
    SDL_Surface *surface = SDL_CreateRGBSurfaceFrom(data, loc.w, loc.h, 32, 
            4*loc.w, 0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);
    SDL_Texture *tex = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    loc.x = 320;
    loc.y = 40;
    SDL_RenderCopy( renderer, tex, NULL, &loc);
    free(data);
}

/*
static void render_entry_rdat() {
    static char buf[BUF_MAX];
    snprintf(buf, BUF_MAX, "RESOURCE %d of %d \n", res_idx, res_max - 1);
    print_line_len(renderer, buf, 320, 20, BUF_MAX);
    gff_chunk_header_t chunk = gff_find_chunk_header(gff_idx, GFF_RDAT, res_ids[res_idx]);
    gff_read_chunk(gff_idx, &chunk, buf, BUF_MAX);
    print_line_len(renderer, buf, 320, 40, chunk.length);
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
    print_line_len(renderer, buf, 320, 20, BUF_MAX);
    print_line_len(renderer, names + 25*res_idx, 320, 40, BUF_MAX);
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
    //unsigned long len = 0;
    //char *text = gff_get_raw_bytes(gff_idx, GFF_TEXT, res_ids[res_idx], &len);
    render_entry_header();
    print_line_len(renderer, "Need to implement", 320, 40, 128);
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
    print_line_len(renderer, buf, 320, pos, 128); pos += 20;
    snprintf(buf, BUF_MAX, "HP: %d\n", combat.hp);
    print_line_len(renderer, buf, 320, pos, 128); pos += 20;
    snprintf(buf, BUF_MAX, "PSP: %d\n", combat.psp);
    print_line_len(renderer, buf, 320, pos, 128); pos += 20;
    //snprintf(buf, BUF_MAX, "Character Index: %d\n", combat.char_index);
    //print_line_len(renderer, buf, 320, pos, 128); pos += 20;
    snprintf(buf, BUF_MAX, "ID: %d\n", combat.id);
    print_line_len(renderer, buf, 320, pos, 128); pos += 20;
    //snprintf(buf, BUF_MAX, "Weapon: %d\n", combat.weapon_index);
    //print_line_len(renderer, buf, 320, pos, 128); pos += 20;
    //snprintf(buf, BUF_MAX, "Packed: %d\n", combat.pack_index);
    //print_line_len(renderer, buf, 320, pos, 128); pos += 20;
    snprintf(buf, BUF_MAX, "Special Attack: %d\n", combat.special_attack);
    print_line_len(renderer, buf, 320, pos, 128); pos += 20;
    snprintf(buf, BUF_MAX, "Special Defense: %d\n", combat.special_defense);
    print_line_len(renderer, buf, 320, pos, 128); pos += 20;
    snprintf(buf, BUF_MAX, "Icon: %d\n", combat.icon);
    print_line_len(renderer, buf, 320, pos, 128); pos += 20;
    snprintf(buf, BUF_MAX, "AC: %d\n", combat.ac);
    print_line_len(renderer, buf, 320, pos, 128); pos += 20;
    snprintf(buf, BUF_MAX, "move: %d\n", combat.move);
    print_line_len(renderer, buf, 320, pos, 128); pos += 20;
    snprintf(buf, BUF_MAX, "status: %d\n", combat.status);
    print_line_len(renderer, buf, 320, pos, 128); pos += 20;
    snprintf(buf, BUF_MAX, "allegiance: %d\n", combat.allegiance);
    print_line_len(renderer, buf, 320, pos, 128); pos += 20;
    snprintf(buf, BUF_MAX, "data: %d\n", combat.data);
    print_line_len(renderer, buf, 320, pos, 128); pos += 20;
    snprintf(buf, BUF_MAX, "thac0: %d\n", combat.thac0);
    print_line_len(renderer, buf, 320, pos, 128); pos += 20;
    snprintf(buf, BUF_MAX, "priority: %d\n", combat.priority);
    print_line_len(renderer, buf, 320, pos, 128); pos += 20;
    snprintf(buf, BUF_MAX, "str: %d, dex: %d\n, con: %d",
       combat.stats.STR, combat.stats.DEX, combat.stats.CON);
    print_line_len(renderer, buf, 320, pos, 128); pos += 20;
    snprintf(buf, BUF_MAX, "int: %d, wis: %d\n, cha: %d",
       combat.stats.INT, combat.stats.WIS, combat.stats.CHA);
    print_line_len(renderer, buf, 320, pos, 128); pos += 20;
    print_line_len(renderer, "Not Shown: weapon, packed, character id", 320, pos, 128); pos += 20;
}

static void print_item(ds1_item_t item, int pos) {
    char buf[BUF_MAX];
    snprintf(buf, BUF_MAX, "id: %d\n", item.id);
    print_line_len(renderer, buf, 320, pos, 128); pos += 20;
    snprintf(buf, BUF_MAX, "quantity: %d\n", item.quantity);
    print_line_len(renderer, buf, 320, pos, 128); pos += 20;
    snprintf(buf, BUF_MAX, "next: %d\n", item.next);
    print_line_len(renderer, buf, 320, pos, 128); pos += 20;
    snprintf(buf, BUF_MAX, "value: %d\n", item.value);
    print_line_len(renderer, buf, 320, pos, 128); pos += 20;
    snprintf(buf, BUF_MAX, "pack index: %d\n", item.pack_index);
    print_line_len(renderer, buf, 320, pos, 128); pos += 20;
    snprintf(buf, BUF_MAX, "item index: %d\n", item.item_index);
    print_line_len(renderer, buf, 320, pos, 128); pos += 20;
    snprintf(buf, BUF_MAX, "Icon: %d\n", item.icon);
    print_line_len(renderer, buf, 320, pos, 128); pos += 20;
    snprintf(buf, BUF_MAX, "charges: %d\n", item.charges);
    print_line_len(renderer, buf, 320, pos, 128); pos += 20;
    snprintf(buf, BUF_MAX, "special: %d\n", item.special);
    print_line_len(renderer, buf, 320, pos, 128); pos += 20;
    snprintf(buf, BUF_MAX, "priority: %d\n", item.priority);
    print_line_len(renderer, buf, 320, pos, 128); pos += 20;
    snprintf(buf, BUF_MAX, "slot: %d\n", item.slot);
    print_line_len(renderer, buf, 320, pos, 128); pos += 20;
    snprintf(buf, BUF_MAX, "name_index: %d\n", item.name_index);
    print_line_len(renderer, buf, 320, pos, 128); pos += 20;
    snprintf(buf, BUF_MAX, "bonus: %d\n", item.bonus);
    print_line_len(renderer, buf, 320, pos, 128); pos += 20;
    /*
    print_line_len(renderer, "Not Shown: weapon, packed, character id", 320, pos, 128); pos += 20;
    */
}

static void render_entry_rdff() {
    char buf[BUF_MAX];
    render_entry_header();
    unsigned long len;
    so_object_t *so = NULL;
    rdff_disk_object_t *rdff = (rdff_disk_object_t*) gff_get_raw_bytes(gff_idx, GFF_RDFF, res_ids[res_idx], &len);
    //printf("res_ids[%d] = %d\n", res_idx, res_ids[res_idx]);
    snprintf(buf, BUF_MAX, "load action: %d\n", rdff->load_action);
    print_line_len(renderer, buf, 320, 40, 128);
    snprintf(buf, BUF_MAX, "blocknum: %d\n", rdff->blocknum);
    print_line_len(renderer, buf, 320, 60, 128);
    snprintf(buf, BUF_MAX, "type: %s\n", rdff_type_names[rdff->type]);
    print_line_len(renderer, buf, 320, 80, 128);
    snprintf(buf, BUF_MAX, "index: %d\n", rdff->index);
    print_line_len(renderer, buf, 320, 100, 128);
    snprintf(buf, BUF_MAX, "from: %d\n", rdff->index);
    print_line_len(renderer, buf, 320, 120, 128);
    snprintf(buf, BUF_MAX, "len: %d\n", rdff->len);
    print_line_len(renderer, buf, 320, 140, 128);
    print_line_len(renderer, "-----------------------", 320, 160, 128);
    print_line_len(renderer, "Jumping to entry:", 320, 180, 128);

    switch(rdff->type) {
        case RDFF_OBJECT:
        case RDFF_CONTAINER: // I don't know the different between Container and Object!
            rdff++;
            so = gff_create_object((char*)rdff, rdff - 1, -1);
            snprintf(buf, BUF_MAX, "index: %d\n", rdff->index);
            print_line_len(renderer, buf, 320, 200, 128);
            snprintf(buf, BUF_MAX, "type: %s\n", so_object_names[so->type]);
            print_line_len(renderer, buf, 320, 220, 128);
            if (so->type == SO_DS1_COMBAT) { print_combat(so->data.ds1_combat, 240); }
            if (so->type == SO_DS1_ITEM) { print_item(so->data.ds1_item, 240); }
            break;
        default:
            snprintf(buf, BUF_MAX, "unknown type: %d\n", rdff->type);
            print_line_len(renderer, buf, 320, 220, 128);
            break;
    }
    /*
    rdff++;
    while (rdff->load_action != -1 && rdff->load_action < 5) {
        snprintf(buf, BUF_MAX, "load action: %d\n", rdff->load_action);
        print_line_len(renderer, buf, 320, pos, 128);
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
        print_line_len(renderer, "ERROR font length > font buf, need to fix!", 340, 20, BUF_MAX);
        return;
    }
    gff_read_chunk(gff_idx, &chunk, font, chunk.length);
    snprintf(buf, BUF_MAX, "RESOURCE %d of %d \n", res_idx, res_max - 1);
    print_line_len(renderer, buf, 320, 20, BUF_MAX);
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
