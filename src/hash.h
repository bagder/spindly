#ifndef SPINDLY_HASH_H
#define SPINDLY_HASH_H
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
 * Hash functions for spindly
 */

#include "spindly.h"
#include "list.h"

/* initial implementation uses a stupid linked list to scan through! */
struct hash {
  struct list_head lhead;
  struct spindly_phys *phys;
};

struct hashnode {
  struct list_node node;
  uint32_t id;
  void *ptr;
};

void _spindly_hash_init(struct hash *h, struct spindly_phys *phys);
struct hashnode * _spindly_hash_get(struct hash *h, uint32_t id);
spindly_error_t _spindly_hash_store(struct spindly_phys *phys,
                        struct hash *h, uint32_t id, void *ptr);
spindly_error_t _spindly_hash_remove(struct spindly_phys *phys,
                         struct hash *h, uint32_t id);
spindly_error_t _spindly_hash_destroy(struct spindly_phys *phys,
                          struct hash *h);

#endif /* SPINDLY_HASH_H */
