#ifndef SPINDLY_STREAM_H
#define SPINDLY_STREAM_H

#include "list.h"
#include "spdy_zlib.h"
#include "spdy_stream.h"

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

  spdy_stream spdy;
  spdy_zlib_context zlib_in;
  spdy_zlib_context zlib_out;
};

#define PRIO_MAX 7

#endif /* SPINDLY_STREAM_H */
