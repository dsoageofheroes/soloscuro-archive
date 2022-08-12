#include "gpl.h"
#include "entity-list.h"
#include "gff.h"
#include "gfftypes.h"
#include "statics.h"
#include <stdlib.h>
#include <stdio.h>

extern entity_list_t* entity_list_create() {
    return (entity_list_t*) calloc(1, sizeof(entity_list_t));
}

extern void entity_list_clear(entity_list_t *list) {
    while (list && list->head) {
        entity_list_remove(list, list->head);
    }
}

extern void entity_list_free(entity_list_t *list) {
    entity_list_clear(list);
    free(list);
}

extern void entity_list_free_all(entity_list_t *list) {
    while (list && list->head) {
        entity_free(list->head->entity);
        entity_list_remove(list, list->head);
    }
    free(list);
}

static entity_list_node_t* create_node(struct entity_s *entity, entity_list_node_t *next, entity_list_node_t *prev) {
    entity_list_node_t *node = (entity_list_node_t*) malloc(sizeof(entity_list_node_t));

    node->entity = entity;
    node->next = next;
    node->prev = prev;

    return node;
}

extern entity_list_node_t* entity_list_add_by_init(entity_list_t *list, struct entity_s *entity) {
    entity_list_node_t *rover;
    entity_list_node_t *node = create_node(entity, NULL, NULL);

    if (!list->head) {
        list->head = node;
        return node;
    }

    rover = list->head;
    while (rover->next != NULL 
        && (rover->next->entity->stats.combat.initiative < node->entity->stats.combat.initiative)) {
        rover = rover->next;
    }

    if (rover->next == NULL) {
        rover->next = node;
        node->prev = rover;
        return node;
    }

    node->prev = rover;
    node->next = rover->next;
    rover->next->prev = node;
    rover->next = node;

    return node;
}

extern entity_list_node_t* entity_list_add(entity_list_t *list, entity_t *entity) {
    if (!list) { return NULL; }
    entity_list_node_t *node = create_node(entity, list->head, NULL);

    if (list->head) {
        list->head->prev = node;
    }

    return list->head = node;
}

extern int entity_list_remove(entity_list_t *list, entity_list_node_t *node) {
    if (node == NULL || list == NULL) { return 0; }

    if (node == list->head) {
        list->head = node->next;
        if (list->head) {
            list->head->prev = NULL;
        }
        free(node);
        return 1;
    }

    if (node->next) {
        node->next->prev = node->prev;
    }
    node->prev->next = node->next;

    free(node);
    return 1;
}

extern int entity_list_remove_entity(entity_list_t *list, struct entity_s *entity) {
    entity_list_remove(list, entity_list_find(list, entity));
}

entity_list_node_t* entity_list_find(entity_list_t *list, entity_t *entity) {
    entity_list_node_t* rover = list->head;

    while(rover) {
        if (rover->entity == entity) { // Warning: pointer comparison
            return rover;
        }
        rover = rover->next;
    }

    return NULL;
}

extern void entity_list_load_etab(entity_list_t *list, sol_static_list_t *ssl, const int gff_idx, const int map_id) {
    if (!list) { return; }
    sol_static_t s;

    if (!open_files[gff_idx].entry_table) {
        gff_chunk_header_t chunk = gff_find_chunk_header(gff_idx, GFF_ETAB, map_id);
        open_files[gff_idx].entry_table = (gff_map_object_t*) malloc(chunk.length);
        if (!open_files[gff_idx].entry_table) {
            error ("unable to malloc for entry table!\n");
            exit(1);
        }
        gff_read_chunk(gff_idx, &chunk, open_files[gff_idx].entry_table, chunk.length);
        open_files[gff_idx].num_objects = chunk.length / sizeof(gff_map_object_t);
    }

    int num_objs = gff_map_get_num_objects(gff_idx, map_id);

    for (int i = 0; i < num_objs; i++) {
        dude_t *dude = entity_create_from_etab(open_files[gff_idx].entry_table, i);
        //dude->anim.scmd = gff_map_get_object_scmd(gff_idx, map_id, i, 0);
        gff_map_fill_scmd_info(dude, gff_idx, map_id, i, 0);
        gff_map_load_scmd(dude);
        animation_shift_entity(list, entity_list_add(list, dude));
        if (dude->anim.scmd != NULL && !(dude->anim.scmd->flags & SCMD_LAST)) {
            entity_animation_list_add(&dude->actions, EA_SCMD, dude, NULL, NULL, 30);
            // Animations are continued in the entity action list
        } else {
            // NEED TO ADD STATICS HERE!
            sol_static_from_entity(dude, &s);
            //printf("have a non animating: (%d, %d, %d): %d + (%d, %d)\n",
                    //dude->mapx, dude->mapy, dude->mapz, dude->anim.bmp_id,
                    //dude->anim.xoffset, dude->anim.yoffset
                    //);
            sol_static_list_add(ssl, &s);
        }
    }
}
