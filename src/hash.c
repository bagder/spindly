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
 *
 * NOTE: this implementation is quickly implemented and primarily set the
 * internal API. It uses a linked list for all entries which is very slow and
 * needs to be replaced with a faster lookup mechanism later.
 */
#include "spdy_setup.h"         /* MUST be the first header to include */

#include <stdio.h>
#include <assert.h>

#include "spindly_phys.h"
#include "hash.h"

/*
  init hash

  store pointed for [id]

  get pointer from [id]

  remove [id] from hash

  destroy the entire hash
*/
void _spindly_hash_init(struct hash *h, struct spindly_phys *phys)
{
  _spindly_list_init(&h->lhead);
  h->phys = phys;
}

struct hashnode * _spindly_hash_get(struct hash *h, uint32_t id)
{
  struct hashnode *n;

  assert(h != NULL);
  assert(id != 0);

  n = _spindly_list_first(&h->lhead);
  while(n) {
    if(n->id == id)
      return n;
    n = _spindly_list_next(&n->node);
  }
  return NULL;
}

spindly_error_t _spindly_hash_store(struct spindly_phys *phys,
                                    struct hash *h, uint32_t id, void *ptr)
{
  struct hashnode *n = _spindly_hash_get(h, id);
  if(n)
    /* the [id] already exists in hash! */
    return SPINDLYE_INVAL;

  /* make a new node */
  n = (struct hashnode *)MALLOC(phys, sizeof(struct hashnode));
  if(!n)
    return SPINDLYE_NOMEM;

  /* fill it in with data */
  n->id = id;
  n->ptr = ptr;

  /* add the new node */
  _spindly_list_add(&h->lhead, &n->node);

  return SPINDLYE_OK;
}

spindly_error_t _spindly_hash_remove(struct spindly_phys *phys,
                                     struct hash *h, uint32_t id)
{
  struct hashnode *n = _spindly_hash_get(h, id);
  if(!n)
    /* the [id] doesn't exist in hash! */
    return SPINDLYE_INVAL;

  /* remove the node and free its memory */
  _spindly_list_remove(&n->node);
  FREE(phys, n);

  return SPINDLYE_OK;
}

spindly_error_t _spindly_hash_destroy(struct spindly_phys *phys,
                                      struct hash *h)
{
  struct hashnode *n = _spindly_list_first(&h->lhead);
  while(n) {
    struct hashnode *f = _spindly_list_next(&n->node);
    _spindly_list_remove(&n->node);
    FREE(phys, n);
    n = f;
  }
  return SPINDLYE_OK;
}
