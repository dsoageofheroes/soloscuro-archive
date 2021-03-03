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

    node->next->prev = node->prev;
    node->prev->next = node->next;

    free(node);
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
    //gff_map_object_t *entry_table = open_files[gff_idx].entry_table;
    int num_objs = gff_map_get_num_objects(gff_idx, map_id);
    //rl->pos = gff_map_get_num_objects(gff_idx, map_id);
    //memset(&rl->objs, 0x0, sizeof(region_object_t) * MAX_REGION_OBJS);

    //for (int i = 0; i < rl->pos; i++) {
    for (int i = 0; i < num_objs; i++) {
        //load_object_from_etab(rl->objs + i, entry_table, i);
        dude_t *dude = entity_create_from_etab(open_files[gff_idx].entry_table, i);
        //rl->objs[i].scmd = gff_map_get_object_scmd(gff_idx, map_id, i, 0);
        dude->sprite.scmd = gff_map_get_object_scmd(gff_idx, map_id, i, 0);
        entity_list_add(list, dude);
    }
}
