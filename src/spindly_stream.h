#ifndef SPINDLY_STREAM_H
#define SPINDLY_STREAM_H

#include "list.h"

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
  size_t write_index;           /* where to write new data to */
  size_t read_index;            /* where to read data from */
  void *userp;                  /* set in stream_new() */
};

#define PRIO_MAX 7

#endif /* SPINDLY_STREAM_H */
