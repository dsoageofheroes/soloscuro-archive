#ifndef GFF_IMAGE_H
#define GFF_IMAGE_H

#define PALETTE_SIZE (256)

typedef struct _gff_color_t {
    unsigned short r, g, b;
} gff_color_t;

typedef struct _gff_palette_t {
    gff_color_t color[PALETTE_SIZE];
} gff_palette_t;

void gff_image_init();
gff_palette_t* create_palettes(int gff_index, unsigned int *len);
int get_frame_count(int gff_index, int type_id, int res_id);
int get_frame_width(int gff_index, int type_id, int res_id, int frame_id);
int get_frame_height(int gff_index, int type_id, int res_id, int frame_id);
unsigned char* get_frame_rgba(int gff_index, int type_id, int res_id, int frame_id);
unsigned char* get_frame_rgba_with_palette(int gff_index, int type_id, int res_id, int frame_id, int palette_id);
unsigned char* get_portrait(unsigned char* bmp_table, unsigned int *width, unsigned int *height);
unsigned char* create_font_rgba(int gff_index, int c, int fg_color, int bg_color);
extern int gff_get_number_of_palettes();

#endif
