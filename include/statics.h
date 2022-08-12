/* SOL_STATICS are objects that do not animate.
 * These are separated simply to help with division of items.
 * These could be entities, or "null" entities
 */
#ifndef SOL_STATICS_H
#define SOL_STATICS_H

#include "status.h"
#include "entity.h"

typedef struct sol_static_s {
    int16_t ds_id;     // This is the darksun/GPL id
    uint16_t mapx;     // object's x position in the region
    uint16_t mapy;     // object's y position in the region
    int16_t mapz;      // object's z position in the region
    int8_t  flags;
    animate_sprite_t anim;
} sol_static_t;

typedef struct sol_static_list_s {
    size_t        pos, len;
    sol_static_t *statics;
} sol_static_list_t;

extern sol_status_t sol_static_from_entity(entity_t *entity, sol_static_t *s);
extern sol_status_t sol_static_list_init(sol_static_list_t *ssl);
extern sol_status_t sol_static_list_cleanup(sol_static_list_t *ssl);
extern sol_status_t sol_static_list_add(sol_static_list_t *ssl, sol_static_t *s);
extern sol_status_t sol_static_list_display(sol_static_list_t *ssl);

#endif
