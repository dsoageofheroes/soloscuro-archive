#include "gpl.h"
#include "entity-list.h"
#include "gff.h"
#include "gfftypes.h"
#include "statics.h"
#include <stdlib.h>
#include <stdio.h>

extern sol_status_t sol_entity_list_create(sol_entity_list_t** ret) {
    if (!ret) { return SOL_NULL_ARGUMENT; }

    *ret = (sol_entity_list_t*) calloc(1, sizeof(sol_entity_list_t));

    return *ret == NULL ? SOL_MEMORY_ERROR : SOL_SUCCESS;
}

extern sol_status_t sol_entity_list_clear(sol_entity_list_t *list) {
    sol_status_t status;

    if (!list) { return SOL_NULL_ARGUMENT; }

    while (list && list->head) {
        if ((status = sol_entity_list_remove(list, list->head)) != SOL_SUCCESS) {
            return status;
        }
    }

    return SOL_SUCCESS;
}

extern sol_status_t sol_entity_list_free(sol_entity_list_t *list) {
    sol_status_t status;
    if (!list) { return SOL_NULL_ARGUMENT; }

    status = sol_entity_list_clear(list);
    if (status != SOL_SUCCESS) { return SOL_NULL_ARGUMENT; }
    free(list);

    return SOL_SUCCESS;
}

extern sol_status_t sol_entity_list_free_all(sol_entity_list_t *list) {
    sol_status_t status;

    if (!list) { return SOL_NULL_ARGUMENT; }

    while (list && list->head) {
        sol_entity_free(list->head->entity);
        if ((status = sol_entity_list_remove(list, list->head)) != SOL_SUCCESS) {
            return status;
        }
    }

    free(list);

    return SOL_SUCCESS;
}

static sol_entity_list_node_t* create_node(sol_entity_t *entity, sol_entity_list_node_t *next, sol_entity_list_node_t *prev) {
    sol_entity_list_node_t *node = (sol_entity_list_node_t*) malloc(sizeof(sol_entity_list_node_t));

    node->entity = entity;
    node->next = next;
    node->prev = prev;

    return node;
}

extern sol_status_t sol_entity_list_add_by_init(sol_entity_list_t *list, sol_entity_t *entity, sol_entity_list_node_t **node) {
    sol_entity_list_node_t *rover;
    sol_entity_list_node_t *tnode;

    if (!list || !entity) { return SOL_NULL_ARGUMENT; }
   
    tnode = create_node(entity, NULL, NULL);
    if (tnode == NULL) { return SOL_MEMORY_ERROR; }

    if (!list->head) {
        list->head = tnode;
        if (node) {
            *node = tnode;
        }
        return SOL_SUCCESS;
    }

    rover = list->head;
    while (rover->next != NULL 
        && (rover->next->entity->stats.combat.initiative < tnode->entity->stats.combat.initiative)) {
        rover = rover->next;
    }

    if (rover->next == NULL) {
        rover->next = tnode;
        tnode->prev = rover;
        if (node) {
            *node = tnode;
        }
        return SOL_SUCCESS;
    }

    tnode->prev = rover;
    tnode->next = rover->next;
    rover->next->prev = tnode;
    rover->next = tnode;

    if (node) {
        *node = tnode;
    }

    return SOL_SUCCESS;
}

extern sol_status_t sol_entity_list_add(sol_entity_list_t *list, sol_entity_t *entity, sol_entity_list_node_t **ret_node) {
    if (!list || !entity) { return SOL_NULL_ARGUMENT; }
    sol_entity_list_node_t *node;
   
    node = create_node(entity, list->head, NULL);
    if (!node) { return SOL_MEMORY_ERROR; }

    if (list->head) {
        list->head->prev = node;
    }

    if (ret_node) {
        *ret_node = node;
    }

    list->head = node;

    return SOL_SUCCESS;
}

extern sol_status_t sol_entity_list_remove(sol_entity_list_t *list, sol_entity_list_node_t *node) {
    if (node == NULL || list == NULL) { return SOL_NULL_ARGUMENT; }

    if (node == list->head) {
        list->head = node->next;
        if (list->head) {
            list->head->prev = NULL;
        }
        free(node);
        return SOL_SUCCESS;
    }

    if (node->next) {
        node->next->prev = node->prev;
    }
    node->prev->next = node->next;

    free(node);
    return SOL_SUCCESS;
}

extern sol_status_t sol_entity_list_remove_entity(sol_entity_list_t *list, sol_entity_t *entity) {
    sol_status_t status;
    sol_entity_list_node_t *node;

    if ((status = sol_entity_list_find(list, entity, &node)) != SOL_SUCCESS) {
        return status;
    }

    return sol_entity_list_remove(list, node);
}

extern sol_status_t sol_entity_list_find(sol_entity_list_t *list, sol_entity_t *entity, sol_entity_list_node_t **node) {
    sol_entity_list_node_t* rover = list->head;

    if (!entity || !list) { return SOL_NULL_ARGUMENT; }

    while(rover) {
        if (rover->entity == entity) { // Warning: pointer comparison
            if (node) {
                *node = rover;
            }
            return SOL_SUCCESS;
        }
        rover = rover->next;
    }

    return SOL_NOT_FOUND;
}

extern sol_status_t sol_entity_list_load_etab(sol_entity_list_t *list, sol_static_list_t *ssl, const int gff_idx, const int map_id) {
    if (!list) { return SOL_NULL_ARGUMENT; }
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
        sol_dude_t *dude;
        sol_entity_create_from_etab(open_files[gff_idx].entry_table, i, &dude);
        //dude->anim.scmd = gff_map_get_object_scmd(gff_idx, map_id, i, 0);
        gff_map_fill_scmd_info(dude, gff_idx, map_id, i, 0);
        gff_map_load_scmd(dude);
        sol_entity_list_add(list, dude, NULL);
        //if ((status = sol_animate_shift_entity(list, entity_list_add(list, dude)))) {
            //return status;
        //}
        if (dude->anim.scmd != NULL && !(dude->anim.scmd->flags & SCMD_LAST)) {
            sol_entity_animation_list_add(&dude->actions, EA_SCMD, dude, NULL, NULL, 30);
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

    return SOL_SUCCESS;
}
