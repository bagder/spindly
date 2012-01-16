/*
 * Home of the spindly_phys_*() functions.
 */
#include "spdy_setup.h"         /* MUST be the first header to include */

#include "spindly.h"

enum stream_state
{
  STREAM_NEW,                   /* as before the peer has ACKed it */
  STREAM_ACKED,                 /* ACKed by remote or locally */
  STREAM_CLOSED                 /* handle has been closed, can't use it */
};

struct spindly_stream
{
  struct spindly_phys *phys;    /* the physical connection this is associated
                                   with */
  enum stream_state;
  size_t bytes_pending;         /* number of bytes not yet drained from this handle */
  size_t write_index;           /* where to write new data to */
  size_t read_index;            /* where to read data from */
};
