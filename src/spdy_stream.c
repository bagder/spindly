#include "spdy_setup.h"         /* MUST be the first header to include */
#include "spdy_stream.h"
#include "spdy_syn_stream.h"
#include "spdy_syn_reply.h"
#include "spdy_rst_stream.h"
#include "spdy_log.h"
#include "spdy_error.h"

#include <stdlib.h>
#include <string.h>

/**
 * Initialize a spdy_stream to null values.
 * @param stream - Stream to initialize.
 * @param store_received_data - Whether to store received data.
 * @param store_frames - Store frames of the stream.
 * @param in - Compression context used to initialise zlib_ctx_in
 * @param out - Compression context used to initialise zlib_ctx_out
 * @return Errorcode
 */
int spdy_stream_init(spdy_stream *stream,
                     bool store_received_data,
                     bool store_frames,
                     spdy_zlib_context *in, spdy_zlib_context *out)
{
  memset(stream, 0, sizeof(stream));

  stream->state = SPDY_STREAM_IDLE;

  stream->store_received_data = store_received_data;
  stream->store_frames = store_frames;

  /* The C standard doesn't guarantee that NULL == 0x00000000, so we have to
   * NULL the pointers explicitely.
   */
  stream->data_received = NULL;
  stream->data_sent = NULL;
  stream->frames = NULL;

  stream->zlib_ctx_in = in;
  stream->zlib_ctx_out = out;

  return SPDY_ERROR_NONE;
}

int spdy_stream_handle_data(spdy_stream *stream, spdy_data *data)
{
  int ret = SPDY_ERROR_NONE;

  switch (stream->state) {
  case SPDY_STREAM_IDLE:
    /* start parsing frame. */
    /* Allocate memory for the new frame. */
    stream->active_frame = malloc(sizeof(spdy_frame));
    if(!stream->active_frame) {
      return SPDY_ERROR_MALLOC_FAILED;
    }
    spdy_frame_init(stream->active_frame);

    /* Set state to parsing frame. */
    stream->state = SPDY_STREAM_PARSING_FRAME;
    /* fall through */
  case SPDY_STREAM_PARSING_FRAME:
    /* continue parsing frame. */
    ret = spdy_frame_parse(stream->active_frame, data, stream->zlib_ctx_in);
    if(ret != SPDY_ERROR_NONE) {
      SPDYDEBUG("ERROR");
      return ret;
    }
    SPDYDEBUG("NOERROR");
    ret = spdy_stream_handle_frame(stream, stream->active_frame);
    break;
  case SPDY_STREAM_TERMINATED:
    /* we don't want to handle any new data. */
    break;
  default:
    /* can't happen! */
    SPDYDEBUG("Non-reachable branch reached - Uninitialized stream?");
    ret = SPDY_ERROR_INVALID_DATA;
    break;
  }
  return ret;
}

/**
 * Handle frame on stream.
 * @param stream - Target stream.
 * @param frame - Frame to handle.
 * @return Errorcode
 */
int spdy_stream_handle_frame(spdy_stream *stream, spdy_frame *frame)
{
  if(stream->fin_received) {
    SPDYDEBUG("Already received FIN.");
    return SPDY_ERROR_STREAM_FIN;
  } else if(stream->rst_received) {
    SPDYDEBUG("Already received RST.");
    return SPDY_ERROR_STREAM_RST;
  }

  if(!stream->frames) {
    stream->frames = frame;
  } else {
    stream->last_frame->next = frame;
    frame->prev = stream->last_frame;
  }
  stream->last_frame = frame;

  switch (frame->type) {
  case SPDY_DATA_FRAME:
    SPDYDEBUG("DATAFRAME");
    return spdy_stream_handle_data_frame(stream, &frame->frame.data);
  case SPDY_CONTROL_FRAME:
    SPDYDEBUG("CONTROLFRAME");
    return spdy_stream_handle_control_frame(stream, &frame->frame.control);
  default:
    /* Should _never_ happen. */
    SPDYDEBUG("Invalid frame type - This should never be reached!");
    return SPDY_ERROR_INVALID_DATA;
  }

  return SPDY_ERROR_NONE;
}

/**
 * Handle data frame on stream.
 * @param stream - Target stream.
 * @param frame - Data frame to handle.
 * @return Errorcode
 */
int spdy_stream_handle_data_frame(spdy_stream *stream, spdy_data_frame *frame)
{

  /* Store received data? */
  if(stream->store_received_data) {
    /* Reallocate buffer for received data */
    char *data_received_new = realloc(stream->data_received,
                                      stream->data_received_length +
                                      frame->length);
    if(!data_received_new) {
      SPDYDEBUG("Reallocating data_received failed.");
      return SPDY_ERROR_MALLOC_FAILED;
    }
    stream->data_received = data_received_new;
    /* Copy frame data to end of data_received */
    memcpy(stream->data_received + stream->data_received_length,
           frame->data, frame->length);
    stream->data_received_length += frame->length;
  }

  /* Check if FIN was received. */
  stream->fin_received = (frame->flags & SPDY_DATA_FLAG_FIN);

  return SPDY_ERROR_NONE;
}

int spdy_stream_handle_control_frame(spdy_stream *stream,
                                     spdy_control_frame *frame)
{
  (void)stream;
  /* Handle control frame types */
  printf("TYPE: %d\n", frame->type);
  switch (frame->type) {
  case SPDY_CTRL_SYN_STREAM:
    SPDYDEBUG("Handle syn stream");
    break;
  case SPDY_CTRL_SYN_REPLY:
    SPDYDEBUG("Handle syn reply");
    break;
  case SPDY_CTRL_RST_STREAM:
    SPDYDEBUG("Handle rst stream");
    /*                      spdy_rst_stream rst_stream;
       spdy_rst_stream_parse(
       stream->rst_received = 1;
       stream->status_code = frame->->status_code; */

    break;
  default:
    SPDYDEBUG("Unhandled control frame type.");
    return SPDY_ERROR_INVALID_DATA;
  }
  return SPDY_ERROR_NONE;
}

int spdy_stream_handle_syn_stream(spdy_stream *stream,
                                  spdy_syn_stream *syn_stream)
{
  (void)stream;
  (void)syn_stream;
  return SPDY_ERROR_NONE;
}

int spdy_stream_handle_syn_reply(spdy_stream *stream,
                                 spdy_syn_reply *syn_reply)
{
  (void)stream;
  (void)syn_reply;
  return SPDY_ERROR_NONE;
}

int spdy_stream_handle_rst_stream(spdy_stream *stream,
                                  spdy_rst_stream *rst_stream)
{
  (void)stream;
  (void)rst_stream;
  return SPDY_ERROR_NONE;
}
