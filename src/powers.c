#include <stdlib.h>
#include "powers.h"
#include "gff.h"
#include "gfftypes.h"

extern void powers_set_cast(power_t *powers, const uint16_t id) {
}

extern void powers_set_icon(power_t *powers, const uint16_t id) {
}

extern void powers_set_thrown(power_t *powers, const uint16_t id) {
}

extern void powers_set_hit(power_t *powers, const uint16_t id) {
}

extern size_t select_by_game(const size_t ds1, const size_t ds2, const size_t dso) {
    switch(gff_get_game_type()) {
        case DARKSUN_1: return ds1;
        case DARKSUN_2: return ds2;
        case DARKSUN_ONLINE: return dso;
        default: break;
    }
    return dso; // By default, dso is always right.
}

extern char* spin_read_description(const uint16_t id) {
    char *description = NULL;

    gff_chunk_header_t chunk = gff_find_chunk_header(RESOURCE_GFF_INDEX, GFF_SPIN, id);
    if (chunk.length <= 0) { return NULL; }
    description = calloc(1, chunk.length + 1);
    gff_read_chunk(RESOURCE_GFF_INDEX, &chunk, description, chunk.length);

    return description;
}
