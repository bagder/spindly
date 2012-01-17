#ifndef SPINDLY_LIST_H
#define SPINDLY_LIST_H
/***************************************************************************
 *  Project      _           _ _
 *     ___ _ __ (_)_ __   __| | |_   _
 *    / __| '_ \| | '_ \ / _` | | | | |
 *    \__ \ |_) | | | | | (_| | | |_| |
 *    |___/ .__/|_|_| |_|\__,_|_|\__, |
 *        |_|                    |___/
 *
 * Copyright (C) 2012, Daniel Stenberg <daniel@haxx.se>
 *
 * This software is licensed as described in the file LICENSE, which you
 * should have received as part of this distribution. The terms are also
 * available at http://spindly.haxx.se/license.html
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY
 * KIND, either express or implied.
 *
 ***************************************************************************/

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
