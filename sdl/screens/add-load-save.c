#include "screen-main.h"
#include "../main.h"
#include "../sprite.h"
#include "../../src/gff.h"
#include "../../src/gfftypes.h"

static uint16_t background;
static uint16_t up_arrow, down_arrow;
static uint16_t action_btn, exit_btn, delete_btn;
static uint16_t title;
static uint16_t bar;

static uint16_t new_sprite_create(SDL_Renderer *renderer, gff_palette_t *pal,
        const int offsetx, const int offsety, const float zoom,
        const int gff_idx, const int type_id, const int res_id) {
    SDL_Rect tmp = {offsetx, offsety, 0, 0};
    return sprite_create(renderer, &tmp, pal, 0, 0, zoom, gff_idx, type_id, res_id);
}

static float zoom;
static uint16_t mousex, mousey;
static uint16_t selection;

void add_load_save_init(SDL_Renderer *renderer, const uint32_t x, const uint32_t y, const float _zoom) {
    gff_palette_t *pal = open_files[RESOURCE_GFF_INDEX].pals->palettes + 0;
    zoom = _zoom;
    selection = 0xFFFF;

    background = new_sprite_create(renderer, pal, 0 + x, 0 + y, zoom, RESOURCE_GFF_INDEX, GFF_BMP, 3009);
    up_arrow = new_sprite_create(renderer, pal, 215 + x, 30 + y, zoom, RESOURCE_GFF_INDEX, GFF_ICON, 2093);
    down_arrow = new_sprite_create(renderer, pal, 215 + x, 130 + y, zoom, RESOURCE_GFF_INDEX, GFF_ICON, 2094);
    exit_btn = new_sprite_create(renderer, pal, 230 + x, 50 + y, zoom, RESOURCE_GFF_INDEX, GFF_ICON, 2058);
    delete_btn = new_sprite_create(renderer, pal, 215 + x, 150 + y, zoom, RESOURCE_GFF_INDEX, GFF_ICON, 2097);
    //title = new_sprite_create(renderer, pal, 115 + x, 150 + y, zoom, RESOURCE_GFF_INDEX, GFF_ICON, 2056); // SAVE
    //title = new_sprite_create(renderer, pal, 115 + x, 150 + y, zoom, RESOURCE_GFF_INDEX, GFF_ICON, 6030); // Load
    title = new_sprite_create(renderer, pal, 115 + x, 0 + y, zoom, RESOURCE_GFF_INDEX, GFF_ICON, 6036); // Add
    //title = new_sprite_create(renderer, pal, 115 + x, 150 + y, zoom, RESOURCE_GFF_INDEX, GFF_ICON, 6038); // Drop
    //action_btn = new_sprite_create(renderer, pal, 230 + x, 30 + y, zoom, RESOURCE_GFF_INDEX, GFF_ICON, 2057); // save
    //action_btn = new_sprite_create(renderer, pal, 230 + x, 30 + y, zoom, RESOURCE_GFF_INDEX, GFF_ICON, 6031); // load
    action_btn = new_sprite_create(renderer, pal, 230 + x, 30 + y, zoom, RESOURCE_GFF_INDEX, GFF_ICON, 6037); // add
   // action_btn = new_sprite_create(renderer, pal, 230 + x, 30 + y, zoom, RESOURCE_GFF_INDEX, GFF_ICON, 6039); // drop

    bar = new_sprite_create(renderer, pal, 45 + x, 31 + y, zoom, RESOURCE_GFF_INDEX, GFF_ICON, 18100);
}

void add_load_save_render(void *data, SDL_Renderer *renderer) {
    sprite_render(renderer, background);
    sprite_render(renderer, up_arrow);
    sprite_render(renderer, down_arrow);
    sprite_render(renderer, exit_btn);
    sprite_render(renderer, delete_btn);
    sprite_render(renderer, title);
    sprite_render(renderer, action_btn);
    for (int i = 0; i < 10; i++) {
        sprite_set_frame(bar, 0);
        sprite_set_location(bar, zoom * 45, zoom * (31 + i * 11));
        if (sprite_in_rect(bar, mousex, mousey)) {
            sprite_set_frame(bar, 1);
        }
        if (i == selection) {
            sprite_set_frame(bar, 3);
        }
        sprite_render(renderer, bar);
    }
    //sprite_render(renderer, bar);
}

int add_load_save_handle_mouse_movement(const uint32_t x, const uint32_t y) {
    static uint16_t last_spr = SPRITE_ERROR;
    uint16_t cspr = SPRITE_ERROR;
    mousex = x;
    mousey = y;

    if (sprite_in_rect(action_btn, x, y)) {
        cspr = action_btn;
    }
    if (sprite_in_rect(delete_btn, x, y)) {
        cspr = delete_btn;
    }
    if (sprite_in_rect(exit_btn, x, y)) {
        cspr = exit_btn;
    }
    if (sprite_in_rect(up_arrow, x, y)) {
        cspr = up_arrow;
    }
    if (sprite_in_rect(down_arrow, x, y)) {
        cspr = down_arrow;
    }

    sprite_set_frame(action_btn, 0);
    sprite_set_frame(exit_btn, 0);
    sprite_set_frame(delete_btn, 0);

    if (sprite_get_frame(cspr) < 2) {
        sprite_set_frame(cspr, 1);
    }

    if (last_spr != SPRITE_ERROR && last_spr != cspr) {
        if (sprite_get_frame(last_spr) < 2) {
            sprite_set_frame(last_spr, 0);
        }
    }

    last_spr = cspr;
    return 1; // means I captured the mouse movement
    //return 0; // zero means I did not handle the mouse, so another screen may.
}

int add_load_save_handle_mouse_down(const uint32_t button, const uint32_t x, const uint32_t y) {
    if (sprite_in_rect(action_btn, x, y)) {
        sprite_set_frame(action_btn, 2);
    }
    if (sprite_in_rect(exit_btn, x, y)) {
        sprite_set_frame(exit_btn, 2);
    }
    if (sprite_in_rect(delete_btn, x, y)) {
        sprite_set_frame(delete_btn, 2);
    }
    if (sprite_in_rect(up_arrow, x, y)) {
        sprite_set_frame(up_arrow, 3);
    }
    if (sprite_in_rect(down_arrow, x, y)) {
        sprite_set_frame(down_arrow, 3);
    }
    for (int i = 0; i < 10; i++) {
        sprite_set_location(bar, zoom * 45, zoom * (31 + i * 11));
        if (sprite_in_rect(bar, mousex, mousey)) {
            printf("selection %d\n", i);
            selection = i;
        }
    }
    return 1; // means I captured the mouse click
    //return 0; // zero means I did not handle the mouse click, so another screen may.
}

int add_load_save_handle_mouse_up(const uint32_t button, const uint32_t x, const uint32_t y) {
    if (sprite_in_rect(action_btn, x, y)) {
        sprite_set_frame(action_btn, 0);
        printf("ACTION\n");
    }
    if (sprite_in_rect(exit_btn, x, y)) {
        sprite_set_frame(exit_btn, 0);
        printf("EXIT\n");
    }
    if (sprite_in_rect(delete_btn, x, y)) {
        sprite_set_frame(delete_btn, 0);
        printf("DELETE\n");
    }
    if (sprite_in_rect(up_arrow, x, y)) {
        sprite_set_frame(up_arrow, 0);
        printf("Up\n");
    }
    if (sprite_in_rect(down_arrow, x, y)) {
        sprite_set_frame(down_arrow, 0);
        printf("Down\n");
    }
    return 1; // means I captured the mouse click
    //return 0; // zero means I did not handle the mouse click, so another screen may.
}

void add_load_save_free() {
    sprite_free(background);
    sprite_free(up_arrow);
    sprite_free(down_arrow);
    sprite_free(exit_btn);
    sprite_free(delete_btn);
    sprite_free(action_btn);
    sprite_free(title);
}

sops_t als_screen = {
    .init = add_load_save_init,
    .cleanup = add_load_save_free,
    .render = add_load_save_render,
    .mouse_movement = add_load_save_handle_mouse_movement,
    .mouse_down = add_load_save_handle_mouse_down,
    .mouse_up = add_load_save_handle_mouse_up,
    .data = NULL
};
