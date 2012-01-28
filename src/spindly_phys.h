#ifndef SPINDLY_PHYS_H
#define SPINDLY_PHYS_H
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

#include "list.h"

struct spindly_indata {
  struct list_node node;
  void *identifier;
  unsigned char *data;
  size_t datalen;
};

struct spindly_phys
{
  spindly_side_t side;
  spindly_spdyver_t protver;

  /* all the streams on this physical connection */
  struct list_head streams;
  int num_streams;             /* how many have been added so far */
  uint32_t streamid;            /* the next streamid to ask for */

  /* list of spindly_stream handles to go over for outgoing traffic */
  struct list_head outq;

  /* list of spindly_indata with incoming traffic */
  struct list_head inq;

  struct spindly_phys_config *config;
};

/* internal functions */

spindly_error_t _spindly_phys_add_stream(struct spindly_phys *phys,
                                         struct spindly_stream *s);

#endif /* SPINDLY_PHYS_H */
