#ifndef SPINDLY_STREAM_H
#define SPINDLY_STREAM_H
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
#include "spdy_zlib.h"
#include "spdy_stream.h"

#define STREAM_BUFSIZE 32 /* scratch buffer for generated frame contents */

enum stream_state
{
  STREAM_NEW,                   /* as before the peer has ACKed it */
  STREAM_ACKED,                 /* ACKed by remote or locally */
  STREAM_CLOSED                 /* handle has been closed, can't use it */
};

struct spindly_stream
{
  struct list_node node;
  struct spindly_phys *phys;    /* the physical connection this is associated
                                   with */
  enum stream_state state;
  size_t bytes_pending;         /* number of bytes not yet drained from this
                                   handle */
  uint32_t streamid;            /* SPDY identifier for this stream */
  void *userp;                  /* set in stream_new() */
  unsigned int prio;            /* 0 - 7 */

  int out; /* when this handle is added to the outq, this field will hold the
              hint of what to send */
  size_t outlen; /* number of bytes in 'buffer' that is stored and ready to
                    get sent off */
  struct list_node outnode;

  struct spindly_stream_config *config;

  spdy_stream spdy;
  spdy_zlib_context zlib_in;
  spdy_zlib_context zlib_out;

  unsigned char buffer[STREAM_BUFSIZE];
};

#define PRIO_MAX 7

spindly_error_t _spindly_stream_init(struct spindly_phys *phys,
                                     unsigned int prio,
                                     struct spindly_stream **stream,
                                     void *userp,
                                     struct spindly_stream_config *config,
                                     bool madebypeer);

#endif /* SPINDLY_STREAM_H */
