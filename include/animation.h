#ifndef ANIMATION_H
#define ANIMATION_H

#include <stdint.h>
#include "ssi-scmd.h"
#include "status.h"
#include "sprite.h"

// O why did SSI choose this over 0... Many tears...
#define SPRITE_ERROR (0xFFFF)

typedef struct animate_sprite_s {
    sol_sprite_t spr;
    uint16_t flags;
    uint16_t delay;
    uint16_t pos;
    uint16_t x, y, w, h;
    uint16_t destx, desty;
    uint16_t load_frame; // frame to select on load
    uint32_t bmp_id;
    int16_t xoffset, yoffset;      // offset for (x, y)
    float movex, movey, left_over; // see animate_tick for left_over
    scmd_t *scmd;
    struct {
        int gff_idx, res_id, index;
    } scmd_info;

} animate_sprite_t;

struct sol_entity_s;
struct sol_entity_action_s;
struct sol_entity_list_node_s;
struct sol_entity_list_s;

extern sol_status_t sol_animate_shift_entity(struct sol_entity_list_s *list, struct sol_entity_list_node_s *en);
extern sol_status_t sol_animate_sprite_tick(struct sol_entity_action_s *action, struct sol_entity_s *entity );

#endif
