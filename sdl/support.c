#include <stdio.h>
#include <stdlib.h>
#include "../src/dsl.h"
#include "../src/ds-region.h"
#include "../src/gff.h"
#include "../src/gff-map.h"
#include "../src/gff-image.h"
#include "../src/spells.h"

#define RES_MAX (1<<12)

unsigned char bmp_header[] = {// All values are little-endian
    0x42, 0x4D,             // Signature 'BM'
    0xaa, 0x00, 0x00, 0x00, // Size: 170 bytes
    0x00, 0x00,             // Unused
    0x00, 0x00,             // Unused
    0x8a, 0x00, 0x00, 0x00, // Offset to image data

    0x7c, 0x00, 0x00, 0x00, // DIB header size (124 bytes)
    0x04, 0x00, 0x00, 0x00, // Width (4px)
    0x02, 0x00, 0x00, 0x00, // Height (2px)
    0x01, 0x00,             // Planes (1)
    0x20, 0x00,             // Bits per pixel (32)
    0x03, 0x00, 0x00, 0x00, // Format (bitfield = use bitfields | no compression)
    0x20, 0x00, 0x00, 0x00, // Image raw size (32 bytes)
    0x13, 0x0B, 0x00, 0x00, // Horizontal print resolution (2835 = 72dpi * 39.3701)
    0x13, 0x0B, 0x00, 0x00, // Vertical print resolution (2835 = 72dpi * 39.3701)
    0x00, 0x00, 0x00, 0x00, // Colors in palette (none)
    0x00, 0x00, 0x00, 0x00, // Important colors (0 = all)
    0xFF, 0x00, 0x00, 0x00, // B bitmask (000000FF)
    0x00, 0xFF, 0x00, 0x00, // G bitmask (0000FF00)
    0x00, 0x00, 0xFF, 0x00, // R bitmask (00FF0000)
    0x00, 0x00, 0x00, 0xFF, // A bitmask (FF000000)
    0x42, 0x47, 0x52, 0x73, // sRGB color space
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Unused R, G, B entries for color space
    0x00, 0x00, 0x00, 0x00, // Unused Gamma X entry for color space
    0x00, 0x00, 0x00, 0x00, // Unused Gamma Y entry for color space
    0x00, 0x00, 0x00, 0x00, // Unused Gamma Z entry for color space

    0x00, 0x00, 0x00, 0x00, // Unknown
    0x00, 0x00, 0x00, 0x00, // Unknown
    0x00, 0x00, 0x00, 0x00, // Unknown
    0x00, 0x00, 0x00, 0x00, // Unknown
};

static void write_image(const char *base_path, const int gff_idx, const int type_id, const int res_id) {
    gff_palette_t *pal = open_files[RESOURCE_GFF_INDEX].pals->palettes;
    gff_image_entry_t *cimg = NULL;
    char filename[1<<10];
    char type[5];
    uint32_t *bmp_ptr = (uint32_t*)(bmp_header + 2);

    if (type_id != GFF_BMP && type_id != GFF_ICON && type_id != GFF_PORT) { return; }

    if (type_id == GFF_BMP && open_files[gff_idx].pals->len) {
        pal = open_files[gff_idx].pals->palettes;
    }

    type[4] = '\0';
    type[3] = (type_id >> 24) & 0xFF;
    type[2] = (type_id >> 16) & 0xFF;
    type[1] = (type_id >> 8) & 0xFF;
    type[0] = (type_id >> 0) & 0xFF;

    gff_chunk_header_t chunk = gff_find_chunk_header(gff_idx, type_id, res_id);
    cimg = (gff_image_entry_t*) malloc(sizeof(gff_image_entry_t*) * chunk.length);
    gff_read_chunk(gff_idx, &chunk, &(cimg->data), chunk.length);
    cimg->data_len = chunk.length;
    cimg->frame_num = *(uint16_t*)(cimg->data + 4);
    printf("%d:%s:%d: has %d frames\n", gff_idx, type, res_id, cimg->frame_num);
    for (int cframe = 0; cframe < cimg->frame_num; cframe++) {
        uint32_t *data = (uint32_t*)get_frame_rgba_palette_img(cimg, cframe, pal);
        uint16_t w = get_frame_width(gff_idx, type_id, res_id, cframe);
        uint16_t h = get_frame_height(gff_idx, type_id, res_id, cframe);
        snprintf(filename, 1<<10, "%s/%d-%s-res%d-frame%d.bmp", base_path, gff_idx, type, res_id, cframe);
        //printf("%s  (%p)\n", filename, data);
        FILE *file = fopen(filename, "w");
        bmp_ptr[0] = sizeof(bmp_header) + 32 * w * h; // size
        bmp_ptr[4] = w;
        bmp_ptr[5] = h;
        fwrite(bmp_header, sizeof(bmp_header), 1, file);
        fwrite(data, 32 * w * h, 1, file);
        fclose(file);
        free(data);
    }
    free(cimg);
}

void export_all_images(const char *base_path) {
    uint32_t res_ids[RES_MAX];
    for (int i = 0; i < NUM_FILES; i++) {
        if (open_files[i].file) {
            for (int j = 0; j < gff_get_number_of_types(i); j++) {
                int type;
                int res_max = gff_get_resource_length(i, type = gff_get_type_id(i, j));
                //printf("SIZE %s:[%d:%d] has %d\n", open_files[i].filename, i, type, res_max);
                gff_get_resource_ids(i, type, res_ids);
                //printf("SIZE %s:[%d] has %d\n", open_files[i].filename, j, res_max);
                for (int k = 0; k < res_max; k++) {
                    write_image(base_path, i, type, res_ids[k]);
                }
            }
            //if (open_files[i].pals->len) {
                //pal = open_files[i].pals->palettes;
            //}
        }
    }
}
