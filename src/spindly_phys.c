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

#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include "spindly.h"
#include "spindly_phys.h"
#include "spindly_stream.h"
#include "spdy_error.h"

/*
 * Create a handle for a single duplex physical connection, SIDE is either
 * client or server - what side the handle is made to handle. PROTVER is the
 * specific SPDY protocol version.
 *
 * TODO: provide a means to replace the memory functions
 */
struct spindly_phys *spindly_phys_init(spindly_side_t side,
                                       spindly_spdyver_t protver,
                                       struct spindly_phys_config *config)
{
  struct spindly_phys *phys;

  /* this is the first malloc, it should use the malloc function provided in
     the config struct if set, but probably cannot use the MALLOC macro */
  phys = malloc(sizeof(struct spindly_phys));
  if(!phys)
    goto fail;
  phys->config = config;
  phys->side = side;
  phys->protver = protver;
  phys->num_streams = 0;
  phys->streamid = 0;

  _spindly_list_init(&phys->streams);
  _spindly_list_init(&phys->outq); /* the list of handles to go over for
                                      outgoing traffic */

  /* init receiver variables  */
  spdy_frame_init(&phys->frame);
  spdy_data_use(&phys->data, NULL, 0);

  return phys;

fail:
  if(phys)
    free(phys);

  return NULL;
}

spindly_error_t _spindly_phys_add_stream(struct spindly_phys *phys,
                                         struct spindly_stream *s)
{
  _spindly_list_add(&phys->streams, &s->node);
  s->streamid = ++phys->streamid;
  phys->num_streams++;
  return SPINDLYE_OK;
}

/*
 * Returns info (pointer and length) with data that PHYS holds that is
 * available to send over the transport medium immediately.
 */
spindly_error_t spindly_phys_outgoing(struct spindly_phys *phys,
                                      unsigned char **data,
                                      size_t *len)
{
  struct list_node *n = _spindly_list_first(&phys->outq);
  if(n) {
    struct spindly_stream *s= (struct spindly_stream *)
      ((char *)n - offsetof(struct spindly_stream, outnode));

    /* iterate over the attached streams and return binary data */
    switch(s->out) {
    case SPDY_CTRL_SYN_STREAM:
      *data = s->buffer;
      *len = s->outlen;
      /* remove this node from the outgoing queue */
      _spindly_list_remove(&s->outnode);
      break;
    }
  }
  else {
    *data = NULL;
    *len = 0;
  }

  return SPINDLYE_OK;
}

/*
 * when the application has read data off the transport, this function is
 * called to tell Spindly about more data that has arrived. As spindly doesn't
 * read any network data by itself, it needs to get the data passed into it by
 * the application.
 *
 * After data has been fed into the handle, call spindly_phys_demux() to make
 * it demux the incoming data.
 *
 */

spindly_error_t spindly_phys_incoming(struct spindly_phys *phys,
                                      unsigned char *data, size_t datalen,
                                      void *identifier)
{
  struct spindly_indata *in = MALLOC(phys, sizeof(*in));

  if(!in)
    return SPINDLYE_NOMEM;

  in->identifier = identifier;
  in->datalen = datalen;
  in->data = data;

  phys->inq_size += datalen;

  /* add this to the phys' incoming queue */
  _spindly_list_add(&phys->inq, &in->node);

  return SPINDLYE_OK;
}

/*
 * 'more' will be set to true/false if there was more data added
 */

static int parse_append(struct spindly_phys *phys, bool *more)
{
  /* the existing data is too small, merge it with the next in the malloc'ed
     buffer */
  struct list_node *n = _spindly_list_first(&phys->inq);
  struct list_node *next = _spindly_list_next(n);
  struct spindly_indata *in= (struct spindly_indata *)next;
  spdy_data *data = &phys->data;
  size_t needed;
  size_t copylen;

  if(in) {
    *more = true;

    needed = data->needed + in->datalen;
    copylen = data->data_end - data->cursor;

    if(phys->parsealloc < needed) {
      char *newp = realloc(phys->parse, needed);
      if(!newp)
        return SPINDLYE_NOMEM;
      phys->parsealloc = needed;
    }

    /* the buffer might already be used by the first chunk, so we either copy
     * new data to it or we move the trailing piece to the start of the
     * buffer */
    memmove(phys->parse, data->cursor, copylen);
    phys->parselen = copylen;

    /* append the entire next chunk */
    memcpy(&phys->parse[copylen], in->data, in->datalen);

    /* now store the combined data amount */
    spdy_data_use(&phys->data, phys->parse, needed);
  }
  else
    *more = false;

  return SPINDLYE_OK;
}

/*
 * Returns information about incoming data, split up for consumption.
 * Subsequent calls will return the next result and so on until there's
 * nothing left to demux - until spindly_phys_incoming() is called again to
 * feed it with more data.
 *
 * When this function returns that there is no more message, it may still hold
 * trailing data that forms the beginning of the subsequent message.
 *
 * 'ptr' will point to a struct dedicated to the particular message.
 */
spindly_error_t spindly_phys_demux(struct spindly_phys *phys,
                                   spindly_demux_t *msg, void **ptr)
{
  struct list_node *n = _spindly_list_first(&phys->inq);
  struct spindly_indata *in= (struct spindly_indata *)n;
  int rc = SPINDLYE_OK;

  *ptr = NULL;
  *msg = SPINDLY_DX_NONE;

  do {

    if(phys->data.data_end <= phys->data.cursor)
      /* if the previously stored data is all consumed then get the
         current queue data */
      spdy_data_use(&phys->data, in->data, in->datalen);

    /* Parse data. The parsing function wants a full frame to be in a
       contiguous buffer so unless it is, we must create a full frame from the
       input we have.

       TODO: how can we know which zlib context to use?
    */
    rc = spdy_frame_parse(&phys->frame, &phys->data, NULL);

    if(rc == SPDY_ERROR_NONE) {
      /* TODO: convert to a 'msg' and suitable struct for 'ptr' and return */

      if(phys->frame.type == SPDY_CONTROL_FRAME) {
        switch(phys->frame.frame.control.type) {
        case SPDY_CTRL_SYN_STREAM:
          break;
        case SPDY_CTRL_SYN_REPLY:
          break;
        case SPDY_CTRL_RST_STREAM:
          break;
        case SPDY_CTRL_SETTINGS:
          break;
        case SPDY_CTRL_NOOP:
          break;
        case SPDY_CTRL_PING:
          break;
        case SPDY_CTRL_GOAWAY:
          break;
        case SPDY_CTRL_HEADERS:
          break;
        case SPDY_CTRL_WINDOW_UPDATE:
          break;
        }
      }
      else { /* data */
        *msg = SPINDLY_DX_DATA;
      }

      /* TODO: if the complete inq node was consumed, call the callback */

      return SPINDLYE_OK;
    }

    else if(rc == SPDY_ERROR_INSUFFICIENT_DATA) {
      /* if there's too little data to parse, merge the buffer with the next
       * in the queue and loop and parse the bigger one
       */
      bool more;
      rc = parse_append(phys, &more);

      if(!more)
        /* there's no more right now */
        return SPINDLYE_OK;
    }
  } while(!rc);

  return rc;
}

#if 0 /* not yet implemented */

/*
 * Tell Spindly how many bytes of the data that has been sent and should be
 * considered consumed. The PHYS will then contain updated information of
 * amount of remaining data to send etc.
 */
spindly_error_t spindly_phys_sent(struct spindly_phys *phys, size_t len)
{

}

/*
 * Change one or more settings associated with the connection. This will
 * result in a SPINDLY_DX_SETTINGS message to end up on the remote side.
 *
 * TODO: figure out how to pass in 'settings' the best way
 */
spindly_error_t spindly_phys_settings(struct spindly_phys *phys,
                                      void *settings)
{

}

#endif

/*
 * Cleanup the entire connection.
 */
void spindly_phys_cleanup(struct spindly_phys *phys)
{
  if(phys) {
    FREE(phys, phys);
  }
}
