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
/*
 * Linked list functions for spindly. The idea is to not do memory functions
 * for list handling.
 */
#include "spdy_setup.h"         /* MUST be the first header to include */

#include <stdio.h>
#include "list.h"

/* init the list head */
void _spindly_list_init(struct list_head *head)
{
  head->first = head->last = NULL;
}

/* add a node last in the list */
void _spindly_list_add(struct list_head *head,
                       struct list_node *entry)
{
  /* store a pointer to the head */
  entry->head = head;

  /* we add this entry at the "top" so it has no next */
  entry->next = NULL;

  /* make our prev point to what the head thinks is last */
  entry->prev = head->last;

  /* and make head's last be us now */
  head->last = entry;

  /* make sure our 'prev' node points to us next */
  if(entry->prev)
    entry->prev->next = entry;
  else
    head->first = entry;
}

/* return the "first" node in the list this head points to */
void *_spindly_list_first(struct list_head *head)
{
  return head->first;
}

/* return the next node in the list */
void *_spindly_list_next(struct list_node *node)
{
  return node->next;
}

/* return the prev node in the list */
void *_spindly_list_prev(struct list_node *node)
{
  return node->prev;
}

/* remove this node from the list */
void _spindly_list_remove(struct list_node *entry)
{
  if(entry->prev)
    entry->prev->next = entry->next;
  else
    entry->head->first = entry->next;

  if(entry->next)
    entry->next->prev = entry->prev;
  else
    entry->head->last = entry->prev;
}

#if 0
/* insert a node before the given 'after' entry */
void _spindly_list_insert(struct list_node *after, /* insert before this */
                          struct list_node *entry)
{
  /* 'after' is next to 'entry' */
  bentry->next = after;

  /* entry's prev is then made to be the prev after current has */
  entry->prev = after->prev;

  /* the node that is now before 'entry' was previously before 'after'
     and must be made to point to 'entry' correctly */
  if(entry->prev)
    entry->prev->next = entry;
  else
    /* there was no node before this, so we make sure we point the head
       pointer to this node */
    after->head->first = entry;

  /* after's prev entry points back to entry */
  after->prev = entry;

  /* after's next entry is still the same as before */

  /* entry's head is the same as after's */
  entry->head = after->head;
}

#endif
