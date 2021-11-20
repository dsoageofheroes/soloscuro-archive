#include "gfftypes.h"
#include "portrait.h"
#include "gff.h"
#include "gpl.h"
#include "../src/port.h"

#include <stdlib.h>
#include <string.h>

#define MAX_PORTRAITS (256)
#define PORT_MAX (1<<14)
static int loaded = 0;
static sol_sprite_t portraits[MAX_PORTRAITS];

void sol_portrait_load() {
    if (loaded) { return; }
    unsigned char *data;
    unsigned char buf[PORT_MAX];
    unsigned int w, h, id;
    unsigned int *ids = gff_get_id_list(DSLDATA_GFF_INDEX, GFF_PORT);
    unsigned int num_ids = gff_get_resource_length(DSLDATA_GFF_INDEX, GFF_PORT);
    memset(portraits, 0x0, sizeof(sol_sprite_t) * MAX_PORTRAITS);

    for (uint32_t i = 0; i < num_ids; i++) {
        id = ids[i];
        gff_chunk_header_t chunk = gff_find_chunk_header(DSLDATA_GFF_INDEX, GFF_PORT, id);
        if (chunk.length > PORT_MAX) {
            error ("chunk.length (%d) is greater than PORT_MAX (%d)\n", chunk.length, PORT_MAX);
            exit (1);
        }
        gff_read_chunk(DSLDATA_GFF_INDEX, &chunk, buf, chunk.length);
        data = gff_get_portrait(buf, &w, &h);
        portraits[id] = sol_sprite_create_from_data(data, w, h);
        free(data);
    }

    if (ids) { free(ids); }
}

void sol_portrait_display(const uint32_t id, const uint32_t x, const uint32_t y) {
    sol_sprite_set_location(portraits[id], x, y);
    sol_sprite_render(portraits[id]);
}

