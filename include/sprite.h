#ifndef SOL_SPRITE_H
#define SOL_SPRITE_H

#include <stdint.h>
#include <stdlib.h>

#include "gff-image.h"
#include "status.h"

#define SPRITE_ERROR (0xFFFF)
#define sol_sprite_t uint32_t

typedef struct sol_sprite_info_s {
    uint16_t x, y, w, h, num_frames, current_frame;
} sol_sprite_info_t;

typedef enum sol_font_e {
    FONT_YELLOW,
    FONT_YELLOW_BRIGHT,
    FONT_GREY,
    FONT_GREYLIGHT,
    FONT_BLACK,
    FONT_BLACKDARK,
    FONT_RED,
    FONT_REDDARK,
    FONT_BLUE,
    NUM_FONTS
} sol_font_t;

typedef struct sol_dim_s {
    uint16_t x, y, w, h;
} sol_dim_t;

extern sol_status_t sol_sprite_init();
extern sol_status_t sol_sprite_new(gff_palette_t *pal,
        const int offsetx, const int offsety, const float zoom,
        const int gff_idx, const int type_id, const int res_id,
        sol_sprite_t *spr);
extern sol_status_t sol_sprite_append(sol_sprite_t sprite_id, gff_palette_t *pal,
        const int offsetx, const int offsety, const float zoom,
        const int gff_idx, const int type_id, const int res_id);
extern sol_status_t sol_sprite_create_from_data(unsigned char *data, const uint32_t w, const uint32_t h, sol_sprite_t *spr);
extern sol_status_t sol_sprite_free(const sol_sprite_t id);

extern sol_status_t sol_sprite_get_info(const sol_sprite_t id, sol_sprite_info_t *loc);

extern sol_status_t sol_sprite_set_location(const sol_sprite_t id, const uint32_t x, const uint32_t y);
extern sol_status_t sol_sprite_render(const sol_sprite_t id);
extern sol_status_t sol_sprite_set_alpha(const sol_sprite_t id, const uint8_t alpha);
extern sol_status_t sol_sprite_set_frame(const sol_sprite_t id, const uint16_t frame);
extern sol_status_t sol_sprite_set_frame_keep_loc(const sol_sprite_t id, const uint16_t frame);
extern sol_status_t sol_sprite_increment_frame(const sol_sprite_t id, const int16_t amt);
extern sol_status_t sol_sprite_in_rect(const uint16_t id, const uint32_t x, const uint32_t y);
extern sol_status_t sol_sprite_center_spr(const int dest, const int src);
extern sol_status_t sol_sprite_center(const int id, const int x, const int y, const int w, const int h);
extern sol_status_t sol_sprite_render_box(const uint16_t sprite_id, const uint16_t x,
    const uint16_t y, const uint16_t w, const uint16_t h);
extern sol_status_t sol_sprite_render_flip(const uint16_t sprite_id, const int horizontal_flip, const int vertical_flip);
extern sol_status_t sol_sprite_set_color_mod(const uint16_t sprite_id, const uint8_t r, const uint8_t g, const uint8_t b);
extern sol_status_t sol_sprite_print(const sol_sprite_t sprite_id);
struct animate_sprite_s;
extern sol_status_t sol_sprite_load(struct animate_sprite_s *anim, gff_palette_t *pal, const int gff_index,
                const int type, const uint32_t id, const int num_load);

// These should be in font.h
extern sol_status_t sol_print_line_len(const sol_font_t font, const char *text, size_t x, size_t y, const uint32_t len);
extern sol_status_t sol_font_render_center(sol_font_t font, const char *str, const uint16_t x, const uint16_t y, const uint16_t w);

#endif
