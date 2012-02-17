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
#include <assert.h>
#include "spindly.h"
#include "spindly_stream.h"
#include "spindly_phys.h"
#include "hash.h"

/*
 * Internal function for creating and setting up a new stream.
 */

spindly_error_t _spindly_stream_init(struct spindly_phys *phys,
                                     unsigned int prio,
                                     struct spindly_stream **stream,
                                     void *userp,
                                     struct spindly_stream_config *config,
                                     uint32_t peer_streamid)
{
  struct spindly_stream *s;
  int rc;
  spdy_control_frame ctrl_frame;

  /* if there's a given streamid, this stream was created by the peer */
  bool madebypeer = peer_streamid?true:false;

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

  /* init the SPDY protocol handle for this stream */
  rc = spdy_stream_init(&s->spdy, false, false, &phys->zlib_in,
                        &phys->zlib_out);
  if(rc)
    goto fail;

  if(!madebypeer) {
    /* only send a SYN_STREAM if this stream is not the result of a received
       SYN_STREAM from the peer */
    struct spindly_outdata *od;

    /* make it a SYN_STREAM frame.

       "If the server is initiating the stream, the Stream-ID must be even.
       If the client is initiating the stream, the Stream-ID must be odd.

       0 is not a valid Stream-ID."

       NOTES:

       - code currently makes all streams independent
       - doesn't include any NV block yet
       - bumps the physical connection's streamid at the bottom of this
       function
    */

    rc = spdy_control_mk_syn_stream(&ctrl_frame, phys->streamid, 0, prio,
                                    NULL);
    if(rc)
      goto fail;

    /* get an out buffer, TODO: what if drained? */
    od = _spindly_list_first(&phys->pendq);

    /* pack a control frame to the output buffer */
    rc = spdy_control_frame_pack(od->buffer, PHYS_OUTBUFSIZE,
                                 &od->len, &ctrl_frame);

    if(rc)
      goto fail;

    od->stream = s;

    /* add this handle to the outq */
    _spindly_list_add(&phys->outq, &od->node);
  }

  /* append this stream to the list of streams held by the phys handle */
  _spindly_phys_add_stream(phys, s);

  /* store a lookup from the streamid to the stream struct */
  _spindly_hash_store(phys, &phys->streamhash, phys->streamid, s);

  *stream = s;

  /* the control frame was only ever held on the stack */
  spdy_control_frame_destroy(&ctrl_frame);

  if(!madebypeer)
    phys->streamid+=2; /* bump counter last so that it isn't bumped in vain */

  return SPINDLYE_OK;

  fail:

  spdy_control_frame_destroy(&ctrl_frame);

  FREE(phys, s);

  return rc;
}

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
  return _spindly_stream_init(phys, prio, stream, userp, config, false);
}

/*
 * Send nack or ack
 */
static spindly_error_t stream_acknack(struct spindly_stream *s, bool ack)
{
  spindly_error_t rc = SPINDLYE_OK;
  spdy_control_frame ctrl_frame;
  struct spindly_outdata *od;

  assert(s != NULL);

  /* queue up a SYN_REPLY or RST_STREAM message */
  if(ack)
    rc = spdy_control_mk_syn_reply(&ctrl_frame, s->streamid, NULL);
  else
    rc = spdy_control_mk_rst_stream(&ctrl_frame, s->streamid, 0);

  if(rc)
    goto fail;

  /* get an out buffer TODO: what if drained? */
  od = _spindly_list_first(&s->phys->pendq);

  /* pack a control frame to the output buffer */
  rc = spdy_control_frame_pack(od->buffer, PHYS_OUTBUFSIZE,
                               &od->len, &ctrl_frame);
  if(rc)
    goto fail;

  od->stream = s;

  /* add this handle to the outq */
  _spindly_list_add(&s->phys->outq, &od->node);

  fail:
  return rc;
}

/*
 * The STREAM as requested to get opened by the remote is allowed! This
 * function is only used as a response to a SPINDLY_DX_STREAM_REQ.
 */
spindly_error_t spindly_stream_ack(struct spindly_stream *s)
{
  return stream_acknack(s, true);
}

/*
 * The STREAM as requested to get opened by the remote is NOT allowed! This
 * function is only used as a response to a SPINDLY_DX_STREAM_REQ.
 */
spindly_error_t spindly_stream_nack(struct spindly_stream *s)
{
  return stream_acknack(s, false);
}

