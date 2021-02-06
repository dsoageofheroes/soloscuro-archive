#ifndef GFF_COMMON_H
#define GFF_COMMON_H

#include <stdint.h>

typedef struct {
    int32_t   id;
    uint32_t  location;
    uint32_t  length;
} gff_chunk_header_t;

#endif
