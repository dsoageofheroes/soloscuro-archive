#include "entity-list.h"
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
