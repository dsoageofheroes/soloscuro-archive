#include "gff-char.h"
#include "gfftypes.h"
#include "gff-common.h"
#include "gff.h"

int gff_char_delete(const int id) {
    int16_t entry = 0;
    gff_chunk_header_t chunk = gff_find_chunk_header(CHARSAVE_GFF_INDEX, GFF_CACT, id);

    size_t amt = gff_read_chunk(CHARSAVE_GFF_INDEX, &chunk, &entry, sizeof(entry));
    if (amt > 1) {
        entry = 0x0000;
        return (gff_write_chunk(CHARSAVE_GFF_INDEX, chunk, (char*) &entry) > 1);
    }

    return 0; // FAILED
}
