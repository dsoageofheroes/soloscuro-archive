#include "gpl.h"
#include "entity-list.h"
#include "gff.h"
#include "gfftypes.h"
#include <stdlib.h>
#include <stdio.h>

extern entity_list_t* entity_list_create() {
    return (entity_list_t*) calloc(1, sizeof(entity_list_t));
}

extern void entity_list_free(entity_list_t *list) {
    while (list && list->head) {
        entity_list_remove(list, list->head);
    }
    free(list);
}

extern void entity_list_free_all(entity_list_t *list) {
    while (list->head) {
        entity_free(list->head->entity);
        entity_list_remove(list, list->head);
    }
    free(list);
}

extern entity_list_node_t* entity_list_add(entity_list_t *list, entity_t *entity) {
    if (!list) { return NULL; }
    entity_list_node_t *node = (entity_list_node_t*) malloc(sizeof(entity_list_node_t));

    node->entity = entity;
    node->next = list->head;
    node->prev = NULL;

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

extern void entity_list_load_etab(entity_list_t *list, const int gff_idx, const int map_id) {
    if (!list) { return; }

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
        dude->anim.scmd = gff_map_get_object_scmd(gff_idx, map_id, i, 0);
        animation_shift_entity(list, entity_list_add(list, dude));
        if (dude->anim.scmd != NULL && !(dude->anim.scmd->flags & SCMD_LAST)) {
            warn("Need to setup animations!\n");
        }
    }
}
