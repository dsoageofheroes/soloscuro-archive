#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "gff-image.h"
#include "gff.h"
#include "gfftypes.h"
#include "gpl.h"
#include "../ext/spng/spng.h"


#define NUM_PALETTES (256)

static unsigned char* create_initialized_image_rgb(const unsigned int w, const unsigned h);
static int plnr_get_mask(const int bits_to_read);
static int plnr_get_bits(const unsigned char *data, const int bits_read, const int bits_to_read);
static int plnr_get_next(unsigned char* chunk, int bits_per_symbol);
static int intensity_multiplier = 4;
static size_t num_palettes;
static gff_palette_t palettes[NUM_PALETTES];
static gff_palette_t *master_palette;

void gff_image_init() {
    num_palettes = 0;
    master_palette = NULL;
    memset(palettes, 0, sizeof(gff_palette_t) * NUM_PALETTES);
}

#define RES_COUNT (256)
#define PAL_MAX   (1<<12)
gff_palettes_t* gff_read_palettes_type(int idx, int type_id) {
    //gff_chunk_header_t *header = gff_find_chunk_header(
    char pal[PAL_MAX];
    uint32_t res_ids[RES_COUNT];
    uint32_t len = gff_get_resource_length(idx, type_id);
    uint32_t mem_amt = sizeof(gff_palettes_t) + len * sizeof(gff_palette_t);

    gff_palettes_t *ret = malloc(sizeof(gff_palettes_t) + mem_amt);
    memset(ret, 0x0, mem_amt);
    ret->len = len;

    if (len > RES_COUNT) {
        error ("Need to increase res_ids buffer in read_palettes_type\n");
        exit(1);
    }

    gff_get_resource_ids(idx, type_id, res_ids);

    for (int i = 0; i < ret->len; i++) {
        gff_chunk_header_t chunk = gff_find_chunk_header(idx, type_id, res_ids[i]);
        size_t pal_len = gff_read_chunk(idx, &chunk, pal, PAL_MAX);
        if (pal_len != PALETTE_SIZE * 3) {
            error("Encountered an incorrectly size palette!\n");
            exit(1);
        }
        for (int j = 0; j < PALETTE_SIZE; j++) {
            ret->palettes[i].color[j].r = intensity_multiplier * ((unsigned char)(pal[j * 3 + 0]));
            ret->palettes[i].color[j].g = intensity_multiplier * ((unsigned char)(pal[j * 3 + 1]));
            ret->palettes[i].color[j].b = intensity_multiplier * ((unsigned char)(pal[j * 3 + 2]));
        }
    }

    return ret;
}

gff_palettes_t* gff_read_palettes(int idx) {
    return gff_read_palettes_type(idx, GFF_PAL);
}

gff_palette_t* gff_create_palettes(int gff_idx, unsigned int *len) {
    open_files[gff_idx].pals = gff_read_palettes(gff_idx);

    for (int i = 0; i < open_files[gff_idx].pals->len; i++) {
        //palettes[num_palettes] = open_files[gff_idx].pals->palettes[i];
        for (int j = 0; j < PALETTE_SIZE; j++) {
            palettes[num_palettes].color[j].r = open_files[gff_idx].pals->palettes[i].color[j].r;
            palettes[num_palettes].color[j].g = open_files[gff_idx].pals->palettes[i].color[j].g;
            palettes[num_palettes].color[j].b = open_files[gff_idx].pals->palettes[i].color[j].b;
        }
        num_palettes++;
    }

    *len = open_files[gff_idx].pals->len;

    if (gff_idx == gff_get_master()) { 
        master_palette = open_files[gff_idx].pals->palettes;
    }

    return NULL;
}

int gff_get_frame_count(int gff_idx, int type_id, int res_id) {
    char buf[16];
    gff_chunk_header_t chunk = gff_find_chunk_header(gff_idx, type_id, res_id);
    //printf("chunk: {id = %d, length = %d, location = %d}\n", chunk.id, chunk.length, chunk.location);
    gff_read_chunk_piece(gff_idx, &chunk, buf, 16);
    short num_frames = *((unsigned short*)(buf + 4));
    return num_frames;
}

extern int gff_image_load_png(const int gff_index, int type_id, const uint32_t image_id,
        const int frame_id, uint32_t *w, uint32_t *h, unsigned char **data) {
    unsigned char buf[1<<16];
    size_t out_size;
    struct spng_ihdr header;

    *data = NULL;
    gff_chunk_header_t chunk = gff_find_chunk_header(gff_index, type_id, image_id);
    if (chunk.length < 8) { return 0; }
    gff_read_chunk(gff_index, &chunk, buf, 1<<16);
    spng_ctx *ctx = spng_ctx_new(0);
    if (spng_set_png_buffer(ctx, buf, chunk.length)) { goto error; }
    if (spng_decoded_image_size(ctx, SPNG_FMT_RGBA8, &out_size)) { goto error; }
    if (spng_get_ihdr(ctx, &header)) { goto error; }
    *w = header.width;
    *h = header.height;
    *data = malloc(out_size);
    if (spng_decode_image(ctx, *data, out_size, SPNG_FMT_RGBA8, 0)) { goto error; }
    spng_ctx_free(ctx);

    return 1;
error:
    if (*data) {
        free(*data);
        *data = NULL;
    }
    return 0;
}

extern int gff_image_is_png(int gff_idx, int type_id, int res_id, int frame_id) {
    unsigned char buf[1<<16];
    gff_chunk_header_t chunk = gff_find_chunk_header(gff_idx, type_id, res_id);
    if (chunk.length < 8) { return 0; }
    gff_read_chunk(gff_idx, &chunk, buf, 1<<16);
    return buf[0] == 137 && buf[1] == 80 && buf[2] == 78 && buf[3] == 71
        && buf[4] == 13 && buf[5] == 10 && buf[6] == 26 && buf[7] == 10;
}

#define get_distance_to_chunk(a, b) ((unsigned long)a - (unsigned long)b)

int gff_get_frame_width(int gff_idx, int type_id, int res_id, int frame_id) {
    unsigned char buf[1<<16];
    gff_chunk_header_t chunk = gff_find_chunk_header(gff_idx, type_id, res_id);
    gff_read_chunk(gff_idx, &chunk, buf, 1<<16);
    if (chunk.length < (6 + 4 + frame_id*4)) { return -1;}
    unsigned int frame_offset = *((unsigned int*)(buf + 6 + frame_id*4));
    //if (chunk.location + frame_offset > open_files[gff_idx].len) {
        //return -1;
    //}
    if (chunk.length <= (frame_offset + 2)) { return -1; }
    unsigned short width = *(unsigned short*)(buf + frame_offset);
    return width;
}

int gff_get_frame_height(int gff_idx, int type_id, int res_id, int frame_id) {
    unsigned char buf[1<<16];
    gff_chunk_header_t chunk = gff_find_chunk_header(gff_idx, type_id, res_id);
    gff_read_chunk(gff_idx, &chunk, buf, 1<<16);
    if (chunk.length < (6 + 4 + frame_id*4)) { return -1;}
    unsigned int frame_offset = *((unsigned int*)(buf + 6 + frame_id*4));
    //if (chunk.location + frame_offset > open_files[gff_idx].len) {
        //return -1;
    //}
    if (chunk.length <= (frame_offset + 4)) { return -1; }
    unsigned short height = *(unsigned short*)(buf + frame_offset + 2);
    return height;
}

static unsigned char* create_initialized_image_rgb(const unsigned int w, const unsigned h) {
    unsigned char *img = malloc(sizeof(unsigned char) * 4 * w * h);
    if (img == NULL) { return NULL; }
    memset(img, 0, sizeof(unsigned char) * 4 * w * h);
    return img;
}

unsigned char* create_ds1_rgba(unsigned char *chunk, int cpos, const int width, const int height, const gff_palette_t *cpal) {
    int num_rows = 0;
    unsigned char* img = create_initialized_image_rgb(width, height);
    while (num_rows < height) {
        int row_num = *(chunk + cpos++);

        if (row_num == 0xFF) { break; }
        if (row_num >= height) { goto ds1_img_error; }

        unsigned char *img_row = img + (4*row_num*width);// The current row to edit.
        num_rows++;
        while (1) {
            int startx = *(chunk + cpos++);
            int flags = *(chunk + cpos++);
            cpos++;
            int compressed_length = *(chunk + cpos++);

            if (flags & 0x01) { // We are column 256...
                startx += 256;
            }
            unsigned char *row = chunk + cpos;

            // Work on pixels.
            for (int i = 0; i < compressed_length; ) {
                int code = *(row + i++);
                int run_len = code / 2 + 1;
                if ((code %2) == 0) { // Even = row of regular palette entries.
                    for (int j = 0; j < run_len; j++) {
                        int pal_index = row[i++];
                        *(img_row + 4*(startx) + 0) = cpal->color[pal_index].r;
                        *(img_row + 4*(startx) + 1) = cpal->color[pal_index].g;
                        *(img_row + 4*(startx) + 2) = cpal->color[pal_index].b;
                        *(img_row + 4*(startx) + 3) = 0xFF;
                        startx++;
                    }
                } else { // Odd = row of duplicate pallete entries.
                    int repeated = *(row + i++);
                    for (int j = 0; j < run_len; j++) {
                        *(img_row + 4*(startx) + 0) = cpal->color[repeated].r;
                        *(img_row + 4*(startx) + 1) = cpal->color[repeated].g;
                        *(img_row + 4*(startx) + 2) = cpal->color[repeated].b;
                        *(img_row + 4*(startx) + 3) = 0xFF;
                        startx++;
                    }
                }
            }

            // Now calculate the next bit.
            cpos += compressed_length;

            if (flags & 0x80) { // Last run flag
                break;
            }
        }
    }
    return img;
ds1_img_error:
    if (img) { free(img); }
    return NULL;
}

/* PLNR CODE */
static int plnr_get_mask(const int bits_to_read) {
    switch (bits_to_read) {
        case 1: return 0x01;
        case 2: return 0x03;
        case 3: return 0x07;
        case 4: return 0x0f;
        case 5: return 0x1f;
        case 6: return 0x3f;
        case 7: return 0x7f;
        case 8: return 0xff;
    }

    fprintf(stderr, "bit_mask > 8 bits is not implemented!\n");
    return 0xff;
}

static int plnr_get_bits(const unsigned char *data, const int bits_read, const int bits_to_read) {
    int byte_offset = bits_read / 8;
    int bit_offset = 4 - (bits_read % 8);
    int bit_mask = plnr_get_mask(bits_to_read);
    const unsigned char *byte = data + byte_offset;
    //printf("bit_offset = %d, bits_to_read = %d\n", bit_offset, bits_to_read);
    if (bit_offset + bits_to_read <= 8) {
        int first_byte = ((*byte) >> bit_offset) & bit_mask;
        int nbyte = ((first_byte & 0x08) >> 3);
        nbyte |= ((first_byte & 0x04) >> 1);
        nbyte |= ((first_byte & 0x02) << 1);
        nbyte |= ((first_byte & 0x01) << 3);
        return first_byte;
    } else {
        printf("split bits!\n");
    }

    return 0;
}

int plnr_last_value = 0;
int plnr_remaining = 0;
int plnr_num_bits_read = 0;

static int plnr_get_next(unsigned char* image_data, int bits_per_symbol) {
    if (plnr_remaining == 0) {
        int first_slice = plnr_get_bits(image_data, plnr_num_bits_read, bits_per_symbol);
        plnr_num_bits_read += bits_per_symbol;
        if (first_slice == 0) {
            int second_slice = plnr_get_bits(image_data, plnr_num_bits_read, bits_per_symbol);
            plnr_num_bits_read += bits_per_symbol;
            if (second_slice == 0) {
                plnr_last_value = 0;
                plnr_remaining = 1;
            } else {
                // forward the value
                plnr_remaining = second_slice + 2;
            }
        } else {
            plnr_last_value = first_slice;
            plnr_remaining = 1;
        }
    }

    plnr_remaining--;

    return plnr_last_value;
}
/* END PLNR CODE */

/*
static void set_color_by_pallete(unsigned char *img_row, const int x, const int pallete_index) {
    gff_color_t *color = master_palette->color + pallete_index;
    *(img_row + 4*(x) + 0) = color->r;
    *(img_row + 4*(x) + 1) = color->g;
    *(img_row + 4*(x) + 2) = color->b;
    if (color->r == 0 && color->g == 0 && color->b == 0) {
        *(img_row + 4*(x) + 3) = 0x00;
    } else {
        *(img_row + 4*(x) + 3) = 0xFF;
    }
}
*/

int int_byte_swap(const int val) {
    int ret = (val >> 24) & 0xFF;
    ret |= (val >> 8) & 0xFF00;
    ret |= (val << 8) & 0xFF0000;
    ret |= (val << 24) & 0xFF000000;

    return ret;
}

//TODO: PERFORMANCE: we don't *need* to read the chunk every time, just the first.
#define FONT_NUM (1<<8)
unsigned char* gff_create_font_rgba(int gff_idx, int c, int fg_color, int bg_color) {
    uint8_t *pixel_idx = NULL;
    ds_font_t font[FONT_NUM]; // a hack, we need extra data to store the fonts!
    gff_chunk_header_t chunk = gff_find_chunk_header(gff_idx, GFF_FONT, 100);

    if ((sizeof(ds_font_t) * FONT_NUM) < chunk.length) {
        error("ERROR font length > font buf, need to fix!");
        return NULL;
    }
    gff_read_chunk(gff_idx, &chunk, font, chunk.length);

    if (!master_palette || c < 0 || c >= font->num) {return NULL;}

    ds_char_t *ds_char = (ds_char_t*)(((uint8_t*)font) + *(font->char_offset + c));
    pixel_idx = ds_char->data;

    if (font->height <= 0 || ds_char->width <= 0) { return NULL; }

    unsigned char* img = create_initialized_image_rgb(ds_char->width, font->height);
    unsigned char* img_row = img;
    int transparent = 0x00000000; // No color!
    for (int x = 0; x < font->height; x++) {
        for (int y = 0; y < ds_char->width; y++, pixel_idx++) {
            float intensity = font->colors[*pixel_idx] / 255.0;
            if (intensity <= 0.0001) {
                *(int*)(img_row + 4*y) = transparent;
                continue;
            }
            int true_fg_color = int_byte_swap(fg_color);
            int true_bg_color = int_byte_swap(bg_color);
            unsigned char *cp = (unsigned char*)(img_row + 4*y);
            unsigned char *fcp = (unsigned char*)&true_fg_color;
            unsigned char *bcp = (unsigned char*)&true_bg_color;
            *(cp + 0) = *(fcp + 0) * intensity;
            *(cp + 1) = *(fcp + 1) * intensity;
            *(cp + 2) = *(fcp + 2) * intensity;
            *(cp + 3) = *(fcp + 3) * intensity;
            *(cp + 0) += *(bcp + 0) * (1.0 - intensity);
            *(cp + 1) += *(bcp + 1) * (1.0 - intensity);
            *(cp + 2) += *(bcp + 2) * (1.0 - intensity);
            *(cp + 3) += *(bcp + 3) * (1.0 - intensity);
        }
        img_row += 4 * ds_char->width;
    }

    return img;
}

unsigned char* gff_get_frame_rgba_palette_img(gff_image_entry_t *img, int frame_id, const gff_palette_t *pal) {
    uint16_t width, height;
    uint32_t frame_offset;
    char *frame_type;

    if (pal == NULL) {
        printf("palette is NULL! returning no image.\n");
        return NULL;
    }

    if (frame_id < 0 && frame_id >= img->frame_num) {
        printf("frame_id outside of bounds: %d\n", frame_id);
        return NULL;
    }

    frame_offset = *(uint32_t*)(img->data + 6 + frame_id * 4);
    if (frame_offset > img->data_len) { return NULL; }
    width = *(uint16_t*)(img->data + frame_offset);
    height = *(uint16_t*)(img->data + frame_offset + 2);
    frame_type = (img->data + frame_offset + 5);

    //printf("w = %d, h = %d, frame_num = %d, frame_type = '%s'\n", width, height, img->frame_num, frame_type);
    if (strncmp(frame_type, "PLNR", 4) == 0) {
        printf("PLNR: Not fully implemented!\n");
        //int bits_per_symbol = *(chunk + frame_offset + 9);
        int bits_per_symbol = *(img->data + frame_offset + 9);
        int dictionary_size = 1 << bits_per_symbol;
        //if (len < frame_offset + 10 + dictionary_size) {
        if (img->data_len < frame_offset + 10 + dictionary_size) {
            fprintf(stderr, "2ERROR: unable to load frame (%d),  it is too small (%u.)\n", frame_id, img->data_len);
        }
        unsigned char *pixel_value_dictionary = malloc(dictionary_size);
        for (int i = 0; i < dictionary_size; i++) {
            //pixel_value_dictionary[i] = *(unsigned char*)(chunk + frame_offset + 10 + i);
            pixel_value_dictionary[i] = *(unsigned char*)(img->data + frame_offset + 10 + i);
            //printf("pvd[%d] = %d\n", i, pixel_value_dictionary[i]);
        }
        int codestart = frame_offset + 10 + dictionary_size;
        //unsigned char *data = chunk + codestart;
        unsigned char *data = (unsigned char*)img->data + codestart;
        plnr_last_value = 0;
        plnr_remaining = 0;
        plnr_num_bits_read = 0;
        unsigned char* img = create_initialized_image_rgb(width, height);
        for (int j = 0; j < height; j++) {
            //printf("[%d]: ", j);
            int row_offset = j * 4 * width;
            for (int k = 0; k < width; k++) {
                int palette_index = plnr_get_next(data, bits_per_symbol);
                gff_color_t color = pal->color[pixel_value_dictionary[palette_index]];
                *(img + row_offset + 4*(k) + 0) = color.r;
                *(img + row_offset + 4*(k) + 1) = color.g;
                *(img + row_offset + 4*(k) + 2) = color.b;
                if (color.r == 0 && color.g == 0 && color.b == 0) {
                    *(img + row_offset + 4*(k) + 3) = 0x00;
                } else {
                    *(img + row_offset + 4*(k) + 3) = 0xFF;
                }
            }
        }
        return img;
        /*
    */
    //int image_start = frame_offset + 10 + dictionary_size;
    } else if (strncmp(frame_type, "PLAN", 4) == 0) {
        printf("PLAN not implemented!\n");
    } else {
        //printf("DS1 frame detected.\n");
        unsigned char* ret_img = create_ds1_rgba((unsigned char*)img->data, frame_offset + 4, width, height, pal);
        if (ret_img == NULL) {
            fprintf(stderr, "Error creating frame: %d\n", frame_id);
        }
        return ret_img;
    }
    return NULL;
}

unsigned char* gff_get_frame_rgba_palette(int gff_idx, int type_id, int res_id, int frame_id, const gff_palette_t *pal) {
    gff_image_entry_t *img;
    unsigned char *data;

    gff_chunk_header_t chunk = gff_find_chunk_header(gff_idx, type_id, res_id);
    if (chunk.length < 1) { return NULL; }
    img = (gff_image_entry_t*) malloc(sizeof(gff_image_entry_t*) * chunk.length);
    gff_read_chunk(gff_idx, &chunk, &(img->data), chunk.length);
    img->data_len = chunk.length;
    img->frame_num = *(uint16_t*)(img->data + 4);

    data = (unsigned char*)gff_get_frame_rgba_palette_img(img, frame_id, pal);
    free(img);
    return data;
}

unsigned char* gff_get_frame_rgba_with_palette(int gff_index, int type_id, int res_id, int frame_id, int palette_id) {
    gff_palette_t *cpal = NULL;
    if (palette_id < 0 || palette_id >= num_palettes) {
        cpal = master_palette;
    } else {
        //cpal = open_files[gff_index].pals->palettes + palette_id;
        cpal = palettes + palette_id;
    }

    return gff_get_frame_rgba_palette(gff_index, type_id, res_id, frame_id, cpal);
}

unsigned char* gff_get_portrait(unsigned char* bmp_table, unsigned int *width, unsigned int *height) {
    unsigned char *ret = NULL;
    unsigned int frame_offset = *((unsigned int*)(bmp_table + 6 ));
    *width = *(unsigned short*)(bmp_table + frame_offset);
    *height = *(unsigned short*)(bmp_table + frame_offset + 2);
    char *frame_type = ((char*)(bmp_table + frame_offset + 5));
    if (strncmp(frame_type, "PLNR", 4) == 0) {
        error("get_portrait: PLNR is not supported!\n");
    } else if (strncmp(frame_type, "PLAN", 4) == 0) {
        error("get_portrait: PLAN is not supported!\n");
    } else {
        //unsigned char* ret_img = create_ds1_rgba(bmp_table, frame_offset + 4, width, height, cpal);
        ret = create_ds1_rgba(bmp_table, frame_offset + 4, *width, *height, master_palette);
    }
    return ret;
}

int gff_get_number_of_palettes() {
    return num_palettes;
}
