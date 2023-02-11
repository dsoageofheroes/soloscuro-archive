#ifndef ENTITY_LIST_H
#define ENTITY_LIST_H

#include "entity.h"
#include "statics.h"

typedef struct sol_entity_list_node_s {
    sol_entity_t *entity;
    struct sol_entity_list_node_s *next, *prev;
} sol_entity_list_node_t;

typedef struct sol_entity_list_s {
    sol_entity_list_node_t *head;
} sol_entity_list_t;

#define sol_entity_list_init(a) a.head = NULL
#define sol_entity_list_empty(a) (a.head == NULL)
#define sol_entity_list_for_each(el, obj) \
    obj = (el && el->head) ? el->head->entity : NULL; \
    for (sol_entity_list_node_t *__el_rover = el->head; __el_rover;\
        __el_rover = __el_rover->next, obj = __el_rover ? __el_rover->entity : NULL)

#endif

extern sol_status_t sol_entity_list_create(sol_entity_list_t **ret);
extern sol_status_t sol_entity_list_clear(sol_entity_list_t *list); // just clear the list (don't free)
extern sol_status_t sol_entity_list_free(sol_entity_list_t *list); // just the list
extern sol_status_t sol_entity_list_free_all(sol_entity_list_t *list); // the list and the entities
extern sol_status_t sol_entity_list_load_etab(sol_entity_list_t *list, sol_static_list_t *ssl, const int gff_idx, const int map_id);
extern sol_status_t sol_entity_list_remove(sol_entity_list_t *list, sol_entity_list_node_t *node);
extern sol_status_t sol_entity_list_remove_entity(sol_entity_list_t *list, sol_entity_t *entity);

extern sol_status_t sol_entity_list_add(sol_entity_list_t *list, sol_entity_t *entity, sol_entity_list_node_t **node);
extern sol_status_t sol_entity_list_add_by_init(sol_entity_list_t *list, sol_entity_t *entity, sol_entity_list_node_t **node);
extern sol_status_t sol_entity_list_find(sol_entity_list_t *list, sol_entity_t *entity, sol_entity_list_node_t **node);
