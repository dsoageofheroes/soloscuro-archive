#include <SDL2/SDL.h>
#include <stdio.h>
#include "screens/narrate.h"
#include "../src/gameloop.h"
#include "../src/dsl.h"
#include "../src/gff.h"
#include "../src/gff-map.h"

#define BUF_MAX (1<<12)
#define RES_MAX (1<<12)

static int done = 0;
static uint32_t last_tick = 0;
static const uint32_t TICK_AMT = 1000 / TICKS_PER_SEC;// Not fully correct...
static SDL_Renderer *renderer = NULL;
static int gff_idx = 0, row_selected = 0, row_max = 0, entry_idx = 0, entry_max, res_idx = 0, res_max = 0;
static uint32_t res_ids[RES_MAX];

static void browse_tick();
static void browse_handle_input();
static void browse_render();
static void move_gff_cursor(int amt);
static void print_gff_entries();
static void render_entry();
static void move_entry_cursor(int amt);

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

void browse_loop(SDL_Renderer *rend) {
    renderer = rend;
    browse_render();
    move_gff_cursor(1);
    move_entry_cursor(1);
    res_idx = 272;
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

static void render_entry_header();
static void render_entry_text();
static void render_entry_monr();
static void render_entry_rdff();

static void render_entry() {
    switch(gff_get_type_id(gff_idx, entry_idx)) {
        case GFF_TEXT: render_entry_text(); break;
        case GFF_MONR: render_entry_monr(); break;
        case GFF_RDFF: render_entry_rdff(); break;
    }
}

static void render_entry_header() {
    char buf[BUF_MAX];
    snprintf(buf, BUF_MAX, "RESOURCE %d of %d (id: %d)\n", res_idx, res_max - 1, res_ids[res_idx]);
    print_line_len(renderer, buf, 320, 20, BUF_MAX);
}

static void render_entry_text() {
    unsigned long len = 0;
    char *text = gff_get_raw_bytes(gff_idx, GFF_TEXT, res_ids[res_idx], &len);
    render_entry_header();
    print_line_len(renderer, text, 320, 40, len);
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
    //snprintf(buf, BUF_MAX, "direction: %d\n", combat.direction);
    //print_line_len(renderer, buf, 320, pos, 128); pos += 20;
    print_line_len(renderer, "Not Shown: weapon, packed, character id", 320, pos, 128); pos += 20;
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
