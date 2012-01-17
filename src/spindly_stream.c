/*
 * Home of the spindly_phys_*() functions.
 */
#include "spdy_setup.h"         /* MUST be the first header to include */

#include <stdlib.h>
#include "spindly.h"
#include "spindly_stream.h"
#include "spindly_phys.h"

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
  s->state = STREAM_NEW;
  s->userp = userp;

  _spindly_phys_add_stream(phys, s);

  *stream = s;

  return SPINDLYE_OK;
}
