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
  size_t bytes_pending;         /* number of bytes not yet drained from this
                                   handle */
  size_t write_index;           /* where to write new data to */
  size_t read_index;            /* where to read data from */
  void *userp;                  /* set in stream_new() */
};

#define PRIO_MAX 7

/*
 * Creates a request for a new stream and muxes the request into the output
 * connection, creates a STREAM handle for the new stream and returns the
 * RESULT. The CUSTOMP pointer will be associated with the STREAM to allow the
 * application to identify it.
 */

spindly_error_t spindly_stream_new(struct spindly_phys *phys,
                                   unsigned int prio,
                                   struct spindly_stream **stream,
                                   void *userp)
{
  struct spindly_stream *s;
  if(!phys || prio> PRIO_MAX)
    return SPINDLYE_INVAL;

  s = CALLOC(phys, sizeof(struct spindly_stream));
  if(!s)
    return SPINDLYE_NOMEM;

  s->phys = phys;
  s->stream_state = STREAM_NEW;
  s->userp = userp;

  *stream = s;

  return SPINDLYE_OK;
}
