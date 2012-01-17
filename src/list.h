#ifndef SPINDLY_LIST_H
#define SPINDLY_LIST_H

struct list_head {
    struct list_node *last;
    struct list_node *first;
};

struct list_node {
    struct list_node *next;
    struct list_node *prev;
    struct list_head *head;
};

void _spindly_list_init(struct list_head *head);

/* add a node last in the list */
void _spindly_list_add(struct list_head *head,
                       struct list_node *entry);

/* return the "first" node in the list this head points to */
void *_spindly_list_first(struct list_head *head);

/* return the next node in the list */
void *_spindly_list_next(struct list_node *node);

/* return the prev node in the list */
void *_spindly_list_prev(struct list_node *node);

/* remove this node from the list */
void _spindly_list_remove(struct list_node *entry);

#endif /* SPINDLY_LIST_H */
