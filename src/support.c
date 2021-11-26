#include <stdio.h>
#include <stdlib.h>
#include "gpl.h"
#include "region.h"
#include "gff.h"
#include "gff-map.h"
#include "gff-xmi.h"
#include "gff-image.h"
#include "wizard.h"

#define XMIDI_CONVERT_NOCONVERSION      0x00
#define XMIDI_CONVERT_MT32_TO_GM        0x01
#define XMIDI_CONVERT_MT32_TO_GS        0x02
#define XMIDI_CONVERT_MT32_TO_GS127     0x03 /* This one is broken, don't use */
#define XMIDI_CONVERT_MT32_TO_GS127DRUM 0x04 /* This one is broken, don't use */
#define XMIDI_CONVERT_GS127_TO_GS       0x05

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
        uint32_t *data = (uint32_t*)gff_get_frame_rgba_palette_img(cimg, cframe, pal);
        uint16_t w = gff_get_frame_width(gff_idx, type_id, res_id, cframe);
        uint16_t h = gff_get_frame_height(gff_idx, type_id, res_id, cframe);
	printf("    frame %d is %d x %x\n", cframe, w, h);
        snprintf(filename, 1<<10, "%s/%d-%s-res%d-frame%d.bmp", base_path, gff_idx, type, res_id, cframe);
        printf("%s  (%p)\n", filename, data);
        FILE *file = fopen(filename, "w+");
        bmp_ptr[0] = sizeof(bmp_header) + 32 * w * h; // size
        bmp_ptr[4] = w;
        bmp_ptr[5] = h;
        fwrite(bmp_header, sizeof(bmp_header), 1, file);
        fwrite(data, 4 * w * h, 1, file);
        fclose(file);
        free(data);
    }
    free(cimg);
}

void export_all_images(const char *base_path) {
    uint32_t res_ids[RES_MAX];
    printf("START EXPORT ALL IMAGES\n");
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

static void write_rdff(int idx, const char *base_path) {
    uint32_t res_ids[RES_MAX];
    ds1_item_t item;
    int res_max = gff_get_resource_length(idx, GFF_RDFF);
    char buf[128];

    snprintf(buf, 128, "%s/items.csv", base_path);
    FILE *f = fopen(buf, "wb+");
    fprintf(f, "id, quantity, next, value, pack_index, item_index, icon, charges, data0, slot, name, bonus, priority, special\n");
    gff_get_resource_ids(idx, GFF_RDFF, res_ids);
    for (int i = 0; i < res_max; i++) {
        if (ssi_item_load(&item, res_ids[i])) {
            fprintf(f, "%d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d (%s), %d, %d, %d\n",
                item.id,
                item.quantity,
                item.next,
                item.value,
                item.pack_index,
                item.item_index,
                item.icon,
                item.charges,
                item.data0,
                item.slot,
                item.name_idx,
                ssi_item_name(item.name_idx),
                item.bonus,
                item.priority,
                item.special);
        }
    }
    fclose(f);
}

static void write_it1r(int idx, const char *base_path) {
    //int res_max = gff_get_resource_length(idx, GFF_IT1R);
    //printf("THere are %d it1rs\n", res_max);
    size_t pos = 0;
    const ds_item1r_t * it1r = ssi_get_item1r(pos);
    char buf[128];

    snprintf(buf, 128, "%s/it1r.csv", base_path);
    FILE *f = fopen(buf, "wb+");
    fprintf(f, "weapon_type, data0, damage_type, weight, data1, base_hp, material, placement, range"
               ", num_attacks, sides, dice, mod, flags, legal_class, base_AC, data2\n");
    while (it1r) {
        fprintf(f, "%d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d\n",
            it1r->weapon_type,
            it1r->data0,
            it1r->damage_type,
            it1r->weight,
            it1r->data1,
            it1r->base_hp,
            it1r->material,
            it1r->placement,
            it1r->range,
            it1r->num_attacks,
            it1r->sides,
            it1r->dice,
            it1r->mod,
            it1r->flags,
            it1r->legal_class,
            it1r->base_AC,
            it1r->data2);
        /*
    */
        pos++;
        it1r = ssi_get_item1r(pos);
    }
    fclose(f);
}

void export_all_items(const char *base_path) {
    //uint32_t res_ids[RES_MAX];
    for (int i = 0; i < NUM_FILES; i++) {
        if (open_files[i].file) {
            for (int j = 0; j < gff_get_number_of_types(i); j++) {
                int type = gff_get_type_id(i, j);
                //int res_max = gff_get_resource_length(i, type = gff_get_type_id(i, j));
                //printf("type = %d, res_max = %d\n", type, res_max);
                switch(type) {
                    case GFF_RDFF:
                        write_rdff(i, base_path);
                        break;
                    case GFF_IT1R:
                        write_it1r(i, base_path);
                        break;
                }
                //printf("SIZE %s:[%d:%d] has %d\n", open_files[i].filename, i, type, res_max);
                //gff_get_resource_ids(i, type, res_ids);
                //printf("SIZE %s:[%d] has %d\n", open_files[i].filename, j, res_max);
                //for (int k = 0; k < res_max; k++) {
                    //write_image(base_path, i, type, res_ids[k]);
                //}
            }
            //if (open_files[i].pals->len) {
                //pal = open_files[i].pals->palettes;
            //}
        }
    }
}

static void write_xmis(const char *base_path, const int gff_idx, const int type_id, const int res_id) {
    char filename[1<<10];
    //char type[5];
    char *data = NULL;
    unsigned int midi_len;

    gff_chunk_header_t chunk = gff_find_chunk_header(gff_idx, type_id, res_id);
    data = malloc(chunk.length);
    gff_read_chunk(gff_idx, &chunk, data, chunk.length);

    snprintf(filename, 1<<10, "%s%03d.xmi", base_path, res_id);
    FILE *file = fopen(filename, "wb+");
    fwrite(data, 1, chunk.length, file);
    fclose(file);

    unsigned char *midi = gff_xmi_to_midi_type((unsigned char*)data, chunk.length, &midi_len, XMIDI_CONVERT_NOCONVERSION);
    snprintf(filename, 1<<10, "%s%03d-no-conversion.midi", base_path, res_id);
    file = fopen(filename, "wb+");
    fwrite(midi, 1, midi_len, file);
    fclose(file);
    free(midi);

    midi = gff_xmi_to_midi_type((unsigned char*)data, chunk.length, &midi_len, XMIDI_CONVERT_MT32_TO_GM);
    snprintf(filename, 1<<10, "%s%03d-mt32-to-gm.midi", base_path, res_id);
    file = fopen(filename, "wb+");
    fwrite(midi, 1, midi_len, file);
    fclose(file);
    free(midi);

    midi = gff_xmi_to_midi_type((unsigned char*)data, chunk.length, &midi_len, XMIDI_CONVERT_MT32_TO_GS);
    snprintf(filename, 1<<10, "%s%03d-mt32-to-gs.midi", base_path, res_id);
    file = fopen(filename, "wb+");
    fwrite(midi, 1, midi_len, file);
    fclose(file);
    free(midi);

    midi = gff_xmi_to_midi_type((unsigned char*)data, chunk.length, &midi_len, XMIDI_CONVERT_GS127_TO_GS);
    snprintf(filename, 1<<10, "%s%03d-gs127-to-gs.midi", base_path, res_id);
    file = fopen(filename, "wb+");
    fwrite(midi, 1, midi_len, file);
    fclose(file);
    free(midi);

    free(data);
}

void export_all_xmis(const char *base_path) {
    char pseq[128];
    char cseq[128];
    char gseq[128];
    char lseq[128];
    uint32_t res_ids[RES_MAX];

    snprintf(pseq, 127, "%s/pseq", base_path);
    snprintf(cseq, 127, "%s/cseq", base_path);
    snprintf(gseq, 127, "%s/gseq", base_path);
    snprintf(lseq, 127, "%s/lseq", base_path);
    for (int i = 0; i < NUM_FILES; i++) {
        if (open_files[i].file) {
            for (int j = 0; j < gff_get_number_of_types(i); j++) {
                int type = gff_get_type_id(i, j);
                int res_max = gff_get_resource_length(i, type = gff_get_type_id(i, j));
                //printf("SIZE %s:[%d:%d] has %d\n", open_files[i].filename, i, type, res_max);
                gff_get_resource_ids(i, type, res_ids);
                //printf("SIZE %s:[%d] has %d\n", open_files[i].filename, j, res_max);
                for (int k = 0; k < res_max; k++) {
                    switch(type) {
                        case GFF_PSEQ:
                            write_xmis(pseq, i, type, res_ids[k]);
                            break;
                        case GFF_CSEQ:
                            write_xmis(cseq, i, type, res_ids[k]);
                            break;
                        case GFF_LSEQ:
                            write_xmis(lseq, i, type, res_ids[k]);
                            break;
                        case GFF_GSEQ:
                            write_xmis(gseq, i, type, res_ids[k]);
                            break;
                    }
                }
            }
        }
    }
}
