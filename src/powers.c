#include <stdlib.h>
#include "powers.h"
#include "wizard.h"
#include "gff.h"
#include "gfftypes.h"

extern power_list_t* power_list_create() {
    return calloc(1, sizeof(power_list_t*));
}

extern void power_list_add(power_list_t *pl, power_t *pw) {
    if (!pl || !pw) { return; }
    power_instance_t *to_add = calloc(1, sizeof(power_instance_t));

    to_add->stats = pw;
    to_add->next = pl->head;
    if (pl->head) {
        pl->head->prev = to_add;
    }
    pl->head = to_add;
}

extern void power_list_free_instance(power_list_t *pl, power_instance_t *pi) {
    if (!pl || !pi) { return; }
    if (pl->head == pi) {
        pl->head = pi->next;
    }
    if (pi->stats->description) {
        free(pi->stats->description);
    }
    free(pi->stats);
    free(pi);
}

extern void power_list_free(power_list_t *pl) {
    if (!pl) { return; }

    while (pl->head) {
        power_list_free_instance(pl, pl->head);
    }

    free(pl);
}

extern void powers_set_cast(power_t *powers, const uint16_t id) {
}

extern void powers_set_icon(power_t *powers, const uint16_t id) {
}

extern void powers_set_thrown(power_t *powers, const uint16_t id) {
}

extern void powers_set_hit(power_t *powers, const uint16_t id) {
}

extern void powers_init() {
    wizard_setup_powers();
}

extern void powers_cleanup() {
    wizard_cleanup();
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
