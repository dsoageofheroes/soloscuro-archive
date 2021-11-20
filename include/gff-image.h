#ifndef GFF_IMAGE_H
#define GFF_IMAGE_H

#include "gff-common.h"

#define PALETTE_SIZE (256)

typedef struct _gff_color_t {
    unsigned short r, g, b;
} gff_color_t;

typedef struct _gff_palette_t {
    gff_color_t color[PALETTE_SIZE];
} gff_palette_t;

typedef struct gff_palettes_s {
    uint16_t len;
    gff_palette_t palettes[];
} gff_palettes_t;

typedef struct gff_image_entry_s {
    uint16_t frame_num;
    uint32_t data_len;
    char data[];
} gff_image_entry_t;

typedef struct gff_frame_entry_s {
    uint16_t width, height;
    char rgba[];
} gff_frame_entry_t;

extern void            gff_image_init();
extern gff_palette_t*  gff_create_palettes(int gff_index, unsigned int *len);
extern gff_palettes_t* gff_read_palettes(int gff_idx);
extern gff_palettes_t* gff_read_palettes_type(int gff_idx, int res_id);
extern int             gff_get_frame_count(int gff_index, int type_id, int res_id);
extern int             gff_get_frame_width(int gff_index, int type_id, int res_id, int frame_id);
extern int             gff_get_frame_height(int gff_index, int type_id, int res_id, int frame_id);
extern int             gff_image_is_png(int gff_idx, int type_id, int res_id, int frame_id);
extern unsigned char*  gff_get_frame_rgba(int gff_index, int type_id, int res_id, int frame_id);
extern unsigned char*  gff_get_frame_rgba_with_palette(int gff_index, int type_id, int res_id, int frame_id, int palette_id);
extern unsigned char*  gff_get_frame_rgba_palette(int gff_index, int type_id, int res_id, int frame_id, const gff_palette_t *pal);
extern unsigned char*  gff_get_frame_rgba_palette_img(gff_image_entry_t *img, int frame_id, const gff_palette_t *pal);
extern int             gff_image_load_png(const int gff_index, int type_id, const uint32_t image_id,
        const int frame_id, uint32_t *w, uint32_t *h, unsigned char **data);
extern unsigned char*  gff_get_portrait(unsigned char* bmp_table, unsigned int *width, unsigned int *height);
extern unsigned char*  gff_create_font_rgba(int gff_index, int c, int fg_color, int bg_color);
extern int             gff_get_number_of_palettes();

#endif
