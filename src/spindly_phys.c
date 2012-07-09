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
#include <assert.h>
#include "spindly.h"
#include "spindly_phys.h"
#include "spindly_stream.h"
#include "spdy_error.h"

static spindly_error_t remove_inqnode(struct spindly_phys *phys,
                                      struct spindly_indata *chunk);

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
  int rc;
  struct spindly_outdata *od;

  /* this is the first malloc, it should use the malloc function provided in
     the config struct if set, but probably cannot use the MALLOC macro */
  phys = malloc(sizeof(struct spindly_phys));
  if(!phys)
    goto fail;
  phys->config = config;
  phys->side = side;
  phys->protver = protver;
  phys->num_streams = 0;
  phys->streamid = side == SPINDLY_SIDE_CLIENT?1:2;
  phys->outgoing = NULL;
  phys->outgoing_tosend = 0;

  /* create zlib contexts for incoming and outgoing data */
  rc = spdy_zlib_inflate_init(&phys->zlib_in);
  if(rc)
    goto fail;

  rc = spdy_zlib_inflate_init(&phys->zlib_out);
  if(rc)
    goto fail;

  _spindly_list_init(&phys->streams);
  _spindly_list_init(&phys->outq);
  _spindly_list_init(&phys->inq);
  _spindly_list_init(&phys->pendq);

  /* now add all outdata nodes to the pending queue */
  for(rc=0; rc < PHYS_NUM_OUTDATA; rc++) {
    od = CALLOC(phys, sizeof(struct spindly_outdata));
    if(!od)
      goto fail;

    _spindly_list_add(&phys->pendq, &od->node);
  }

  /* init receiver variables  */
  spdy_frame_init(&phys->frame);
  spdy_data_use(&phys->data, NULL, 0);

  /* for stream-ID to stream struct lookups */
  _spindly_hash_init(&phys->streamhash, phys);

  return phys;

fail:
  spdy_zlib_inflate_end(&phys->zlib_in);
  spdy_zlib_inflate_end(&phys->zlib_out);

  /* TODO: clean up the pendq list */

  if(phys)
    free(phys);

  return NULL;
}

spindly_error_t _spindly_phys_add_stream(struct spindly_phys *phys,
                                         struct spindly_stream *s)
{
  _spindly_list_add(&phys->streams, &s->node);
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
  struct spindly_outdata *od;

  if(phys->outgoing)
    /* data returned previously has not yet been reported to have been sent
       off */
    return SPINDLYE_INVAL;

  od = _spindly_list_first(&phys->outq);
  if(od) {
    *data = od->buffer;
    *len = od->len;
    /* remove this node from the outgoing queue */
    _spindly_list_remove(&od->node);

    phys->outgoing = od;
    phys->outgoing_tosend = *len; /* send this to be done */
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
                                      int flags,
                                      void *identifier)
{
  struct spindly_indata *in = MALLOC(phys, sizeof(*in));

  if(!in)
    return SPINDLYE_NOMEM;

  in->identifier = identifier;
  if(flags & SPINDLY_INCOMING_COPY) {
    in->data = MALLOC(phys, datalen);
    if(!in->data) {
      FREE(phys, in);
      return SPINDLYE_NOMEM;
    }
    memcpy(in->data, data, datalen);
    in->datalen = datalen;
  }
  else {
    in->datalen = datalen;
    in->data = data;
  }

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

    /* now remove both the copied nodes from the list */
    remove_inqnode(phys, (struct spindly_indata *)n);
    remove_inqnode(phys, (struct spindly_indata *)next);

    /* now store the combined data amount */
    spdy_data_use(&phys->data, phys->parse, needed);
  }
  else
    *more = false;

  return SPINDLYE_OK;
}

/* TODO: if the complete inq node was consumed::
   1 - call the callback.
   2 - possibly free the ->data
   3 - remove the node from the list
*/
static spindly_error_t remove_inqnode(struct spindly_phys *phys,
                                      struct spindly_indata *chunk)
{
  /* call the completetion callback? */
  (void)phys;

  /* free the data if it was previously copied into the node */
  if(chunk->copied)
    FREE(phys, chunk->data);

  /* remove the node from the linked list */
  _spindly_list_remove(&chunk->node);

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
 * 'ptr' must point to the correct struct, read the first 'type' field of that
 * to know how to interpret the rest!
 */
spindly_error_t spindly_phys_demux(struct spindly_phys *phys,
                                   struct spindly_demux *ptr)
{
  struct list_node *n = _spindly_list_first(&phys->inq);
  struct spindly_indata *in= (struct spindly_indata *)n;
  int rc = SPINDLYE_OK;

  assert(ptr != NULL);

  ptr->type = SPINDLY_DX_NONE;

  do {

    if(phys->data.data_end <= phys->data.cursor)
      /* if the previously stored data is all consumed then get the
         current queue data */
      spdy_data_use(&phys->data, in->data, in->datalen);

    /*
     * Parse data. The parsing function wants a full frame to be in a
     * contiguous buffer so unless it is, we must create a full frame from the
     * input we have.
     */
    rc = spdy_frame_parse(&phys->frame, phys, &phys->data);

    if(rc == SPDY_ERROR_NONE) {
      if(phys->frame.type == SPDY_CONTROL_FRAME) {
        spdy_syn_stream *syn;
        spdy_syn_reply *rep;
        struct spindly_stream *stream;
        struct hashnode *n;

        switch(phys->frame.frame.control.type) {
        case SPDY_CTRL_SYN_STREAM:
          /*
           * At this point there's a syn_stream struct that needs to be
           * converted to a full spinly_stream struct!
           *
           * phys->frame.frame.control.obj.syn_stream
           */
          syn = &phys->frame.frame.control.obj.syn_stream;

          rc = _spindly_stream_init(phys, syn->priority, &stream, NULL,
                                    NULL, syn->stream_id);
          if(rc)
            /* TODO: how do we deal with a failure here? */
            break;

          ptr->type = SPINDLY_DX_STREAM_REQ;
          ptr->msg.stream.stream = stream;
          break;
        case SPDY_CTRL_SYN_REPLY:
          /*
           * At this point there's a syn_reply struct that needs to be
           * converted to a full spinly_stream struct!
           *
           * phys->frame.frame.control.obj.syn_reply
           */
          rep = &phys->frame.frame.control.obj.syn_reply;

          n = _spindly_hash_get(&phys->streamhash, rep->stream_id);
          if(!n)
            /* received a SYN_REPLY for an unknown streamid */
            rc = SPINDLYE_PROTOCOL;
          else {
            stream = n->ptr;
            stream->state = STREAM_ACKED;
            ptr->type = SPINDLY_DX_STREAM_ACK;
            ptr->msg.stream.stream = stream;
          }
          break;
        case SPDY_CTRL_RST_STREAM:
          assert(0); /* not implemented yet! */
          break;
        case SPDY_CTRL_SETTINGS:
          assert(0); /* not implemented yet! */
          break;
        case SPDY_CTRL_NOOP:
          assert(0); /* not implemented yet! */
          break;
        case SPDY_CTRL_PING:
          assert(0); /* not implemented yet! */
          break;
        case SPDY_CTRL_GOAWAY:
          assert(0); /* not implemented yet! */
          break;
        case SPDY_CTRL_HEADERS:
          assert(0); /* not implemented yet! */
          break;
        case SPDY_CTRL_WINDOW_UPDATE:
          assert(0); /* not implemented yet! */
          break;
        default:
          assert(0); /* internal error */
          break;
        }
        spdy_frame_destroy(&phys->frame);
      }
      else { /* data */
        ptr->type = SPINDLY_DX_DATA;
      }

      if(phys->data.cursor == phys->data.data_end)
        /* the complete inq node was consumed */
        remove_inqnode(phys, in);

      return SPINDLYE_OK;
    }

    else if(rc == SPDY_ERROR_INSUFFICIENT_DATA) {
      /* if there's too little data to parse, merge the buffer with the next
       * in the queue and loop and parse the bigger one
       */
      bool more;
      rc = parse_append(phys, &more);
      if(rc)
        break;

      if(!more)
        /* there's no more right now */
        return SPINDLYE_OK;
    }
  } while(!rc);

  return rc;
}

/*
 * Tell Spindly how many bytes of the data that has been sent and should be
 * considered consumed. The PHYS will then contain updated information of
 * amount of remaining data to send etc.
 */
spindly_error_t spindly_phys_sent(struct spindly_phys *phys, size_t len)
{
  struct spindly_outdata *od = phys->outgoing;

  if(len > phys->outgoing_tosend)
    /* a larger value that outstanding means badness and we rather tell the
       user than adapt in silence */
    return SPINDLYE_INVAL;

  phys->outgoing_tosend -= len;

  if(phys->outgoing_tosend == 0) {
    phys->outgoing = NULL;

    /* add this node back to the pending queue */
    _spindly_list_add(&phys->pendq, &od->node);
  }
  return SPINDLYE_OK;
}

#if 0 /* not yet implemented */

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
  /* TODO: move over all attached streams and clean them up as well */

  if(phys) {
    FREE(phys, phys);
  }
}
