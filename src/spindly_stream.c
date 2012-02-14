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
 *
 */

spindly_error_t spindly_stream_new(struct spindly_phys *phys,
                                   unsigned int prio,
                                   struct spindly_stream **stream,
                                   void *userp,
                                   struct spindly_stream_config *config)
{
  struct spindly_stream *s;
  int rc;
  spdy_control_frame ctrl_frame;

  if(!phys || prio> PRIO_MAX)
    return SPINDLYE_INVAL;

  s = CALLOC(phys, sizeof(struct spindly_stream));
  if(!s)
    return SPINDLYE_NOMEM;

  /* create a control frame */
  spdy_control_frame_init(&ctrl_frame);

  s->prio = prio;
  s->phys = phys;
  s->state = STREAM_NEW;
  s->userp = userp;
  s->config = config;

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

  /* mark the current action */
  s->out = SPDY_CTRL_SYN_STREAM;

  /* make it a SYN_STREAM frame.

     NOTES:

     - code currently makes all streams independent
     - doesn't include any NV block yet
     - bumps the physical connection's streamid at the bottom of this
       function
  */
  rc = spdy_control_mk_syn_stream(&ctrl_frame, phys->streamid, 0, prio, NULL);
  if(rc)
    goto fail;

  /* pack a control frame to the output buffer */
  rc = spdy_control_frame_pack(s->buffer, sizeof(s->buffer),
                               &s->outlen, &ctrl_frame);
  if(rc)
    goto fail;

  /* add this handle to the outq */
  _spindly_list_add(&phys->outq, &s->outnode);

  /* append this stream to the list of streams held by the phys handle */
  _spindly_phys_add_stream(phys, s);

  *stream = s;

  /* the control frame was only ever held on the stack */
  spdy_control_frame_destroy(&ctrl_frame);

  phys->streamid++; /* bump the counter last so that it isn't bumped in vain */

  return SPINDLYE_OK;

  fail:

  spdy_control_frame_destroy(&ctrl_frame);

  spdy_zlib_inflate_end(&s->zlib_in);
  spdy_zlib_inflate_end(&s->zlib_out);

  FREE(phys, s);

  return rc;
}
