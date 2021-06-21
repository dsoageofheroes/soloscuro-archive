#include "dsl.h"
#include "entity-list.h"
#include "gff.h"
#include "gfftypes.h"
#include <stdlib.h>
#include <stdio.h>

entity_list_t* entity_list_create() {
    return calloc(1, sizeof(entity_list_t));
}

void entity_list_free(entity_list_t *list) {
    while (*list) {
        entity_list_remove(list, *list);
    }
    free(list);
}

void entity_list_free_all(entity_list_t *list) {
    while (*list) {
        entity_free((*list)->entity);
        entity_list_remove(list, *list);
    }
    free(list);
}

void entity_list_add(entity_list_t *list, entity_t *entity) {
    if (!list) { return; }
    entity_list_node_t *node = malloc(sizeof(entity_list_node_t));

    node->entity = entity;
    node->next = *list;
    node->prev = NULL;

    if (*list) {
        (*list)->prev = node;
    }

    *list = node;
}

void entity_list_remove(entity_list_t *list, entity_list_node_t *node) {
    if (node == NULL || list == NULL) { return; }

    if (node == *list) {
        *list = node->next;
        if (*list) {
            (*list)->prev = NULL;
        }
        free(node);
        return;
    }

    if (node->next) {
        node->next->prev = node->prev;
    }
    node->prev->next = node->next;

    free(node);
}

entity_list_node_t* entity_list_find(entity_list_t *list, entity_t *entity) {
    entity_list_node_t* rover = *list;

    while(rover) {
        if (rover->entity == entity) {
            return rover;
        }
        rover = rover->next;
    }

    return NULL;
}

void entity_list_load_etab(entity_list_t *list, const int gff_idx, const int map_id) {
    if (!list) { return; }

    if (!open_files[gff_idx].entry_table) {
        gff_chunk_header_t chunk = gff_find_chunk_header(gff_idx, GFF_ETAB, map_id);
        open_files[gff_idx].entry_table = malloc(chunk.length);
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
        entity_list_add(list, dude);
    }
}
