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
  int rc;

  if(!phys || prio> PRIO_MAX)
    return SPINDLYE_INVAL;

  s = CALLOC(phys, sizeof(struct spindly_stream));
  if(!s)
    return SPINDLYE_NOMEM;

  s->phys = phys;
  s->state = STREAM_NEW;
  s->userp = userp;

  /* create zlib contexts for incoming and outgoing data */
  rc = spdy_zlib_inflate_init(&s->zlib_in);
  if(rc)
    goto fail;

  rc = spdy_zlib_inflate_init(&s->zlib_out);
  if(rc)
    goto fail;

  /* init the SPDY protocol handle for this stream */
  rc = spdy_stream_init(&s->spdy, false, false, &s->zlib_in, &s->zlib_out);
  if(rc)
    goto fail;

  /* now create an outgoing SPDY frame to request this stream to get acked
     by the remote */

  /* append this stream to the list of streams held by the phys handle */
  _spindly_phys_add_stream(phys, s);

  *stream = s;

  return SPINDLYE_OK;

  fail:
  spdy_zlib_inflate_end(&s->zlib_in);
  spdy_zlib_inflate_end(&s->zlib_out);

  return rc;
}
