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

#include <spindly.h>

#include "list.h"
#include "hash.h"

#include "spdy_frame.h"

/*
 * We use a set of pre-allocated structs in a linked list to put data in to
 * get sent.
 */
#define PHYS_NUM_OUTDATA 64 /* number of allocated structs by default */
#define PHYS_OUTBUFSIZE 128 /* size of buffer that avoids malloc */

struct spindly_outdata {
  struct list_node node;
  size_t len; /* number of bytes of data provided */
  unsigned char *alloced; /* if not NULL, an allocated pointer with data
                             instead of buffer */
  unsigned char buffer[PHYS_OUTBUFSIZE];
  struct spindly_stream *stream; /* originating stream */
};


struct spindly_indata {
  struct list_node node;
  void *identifier;
  unsigned char *data;
  size_t datalen;
  bool copied; /* set TRUE if data malloced and copied here */
};

struct spindly_phys
{
  spindly_side_t side;
  spindly_spdyver_t protver;

  /* all the streams on this physical connection */
  struct list_head streams;
  int num_streams;           /* how many have been added so far */
  uint32_t streamid;         /* the next streamid to ask for */

  /* list of spindly_stream handles to go over for outgoing traffic */
  struct list_head outq;

  /* 'outgoing' holds the node when *outgoing() has been called until *sent()
     has acknowledged that the entire data is sent. */

  size_t outgoing_tosend;
  struct spindly_outdata *outgoing;

  /* list of spindly_indata with incoming traffic */
  struct list_head inq;
  size_t inq_size; /* total number of bytes in the queue */

  /* list of spindly_outdata nodes that are unused */
  struct list_head pendq;

  /* state variables for the parsing and demuxing of single incoming data
     stream */
  spdy_frame frame;
  spdy_data data;

  unsigned char *parse; /* malloc'ed buffer for parsing incoming data */
  size_t parsealloc; /* size of the malloc'ed parse buffer */
  size_t parselen;   /* length of data used in the parse buffer */

  struct spindly_phys_config *config;

  struct hash streamhash; /* for ID => stream lookup */

  spdy_zlib_context zlib_in;
  spdy_zlib_context zlib_out;

};

/* internal functions */

spindly_error_t _spindly_phys_add_stream(struct spindly_phys *phys,
                                         struct spindly_stream *s);

#endif /* SPINDLY_PHYS_H */
