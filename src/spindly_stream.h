#ifndef SPINDLY_STREAM_H
#define SPINDLY_STREAM_H
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

#include "list.h"
#include "spdy_zlib.h"
#include "spdy_frame.h"
#include "spdy_data_frame.h"
#include "spdy_control_frame.h"

enum stream_state
{
  STREAM_NEW,                   /* as before the peer has ACKed it */
  STREAM_ACKED,                 /* ACKed by remote or locally */
  STREAM_CLOSED                 /* handle has been closed, can't use it */
};

enum spdy_stream_states
{
  SPDY_STREAM_IDLE,
  SPDY_STREAM_PARSING_FRAME,
  SPDY_STREAM_TERMINATED
};

/**
 * SPDY Stream
 * This structure keeps the whole state of a SPDY stream.
 */
typedef struct
{
  /** State: */
  enum spdy_stream_states state;
  /** Configuration: **/
  bool store_received_data;
  bool store_frames;
  /** Stream data: **/
  uint32_t stream_id;
  uint32_t associated_to;
  bool unidirectional;
  uint32_t data_received_length;
  uint32_t data_sent_length;
  char *data_received;
  char *data_sent;
  bool fin_received;
  bool fin_sent;
  bool rst_received;
  bool rst_sent;
  uint32_t status_code;
  uint32_t frames_count;
  spdy_frame *frames;
  spdy_frame *last_frame;
  spdy_zlib_context *zlib_ctx_in;
  spdy_zlib_context *zlib_ctx_out;
  /** Temporary data: */
  spdy_frame *active_frame;
} spdy_stream;

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
  unsigned int prio;            /* 0 - 7 */

  struct spindly_stream_config *config;

  spdy_stream spdy;
};

#define PRIO_MAX 7


spindly_error_t _spindly_stream_init0(spdy_stream *stream,
                     bool store_received_data,
                     bool store_frames,
                     spdy_zlib_context *in, spdy_zlib_context *out);

spindly_error_t _spindly_stream_init(struct spindly_phys *phys,
                                     unsigned int prio,
                                     struct spindly_stream **stream,
                                     void *userp,
                                     struct spindly_stream_config *config,
                                     uint32_t streamid);

#endif /* SPINDLY_STREAM_H */
