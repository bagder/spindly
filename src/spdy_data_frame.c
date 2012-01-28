#include "spdy_setup.h"         /* MUST be the first header to include */
#include "spdy_data_frame.h"
#include "spdy_log.h"
#include "spdy_error.h"
#include "spdy_bytes.h"

#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>

/* Minimum length of a data frame. */
#define SPDY_DATA_FRAME_MIN_LENGTH 8

int spdy_data_frame_init(spdy_data_frame *frame)
{
  frame->stream_id = 0;
  return SPDY_ERROR_NONE;
}

/**
 * Parse the header of a data frame. This needs 'stream_id' to be cleared to
 * consider this as the first call.
 *
 * @param frame - Target data frame.
 * @param data - Data to parse.
 * @see spdy_data_frame
 * @return Errorcode
 */
int spdy_data_frame_parse_header(spdy_data_frame *frame, spdy_data *data)
{

  /* Check if the frame header has already been parsed. */
  if(!frame->stream_id) {
    size_t length = data->data_end - data->cursor;
    if(length < SPDY_DATA_FRAME_MIN_LENGTH) {
      SPDYDEBUG("Insufficient data for data frame.");
      data->needed = SPDY_DATA_FRAME_MIN_LENGTH - length;
      return SPDY_ERROR_INSUFFICIENT_DATA;
    }

    /* Read stream id. (AND removes the first type bit.) */
    frame->stream_id = BE_LOAD_32(data->cursor) & 0x7FFFFFFF;
    data->cursor += 4;
    frame->flags = data->cursor[0];
    frame->length = BE_LOAD_32(data->cursor) & 0x00FFFFFF;
    data->cursor += 4;
    frame->data = NULL;         /* no frame payload yet */
  }
  return SPDY_ERROR_NONE;
}

/**
 * Parse a data frame
 * @param frame - Target data frame.
 * @param data - Data to parse.
 * @see spdy_data_frame
 * @return Errorcode
 */
int spdy_data_frame_parse(spdy_data_frame *frame, spdy_data *data)
{
  int ret;
  size_t length;
  ret = spdy_data_frame_parse_header(frame, data);
  if(ret != SPDY_ERROR_NONE) {
    return ret;
  }

  length = data->data_end - data->cursor;
  if(frame->length > length) {
    data->needed = frame->length - length;
    SPDYDEBUG("Insufficient data for data frame.");
    return SPDY_ERROR_INSUFFICIENT_DATA;
  }

  frame->data = malloc(frame->length);
  if(!frame->data) {
    SPDYDEBUG("Frame payload malloc failed.");
    return SPDY_ERROR_MALLOC_FAILED;
  }
  memcpy(frame->data, data->cursor, frame->length);
  data->cursor += frame->length;

  return SPDY_ERROR_NONE;
}

/**
 * Pack the data frame into a buffer for transmitting.
 * @param out Target buffer
 * @param bufsize Size of the target buffer
 * @param outlen Length of the data written by this function
 * @param frame Frame to pack
 * @see spdy_data_frame
 * @return Errorcode
 */
int spdy_data_frame_pack_header(char *out, size_t bufsize,
                                size_t *outlen, spdy_data_frame *frame)
{
  if(bufsize < 8)
    return SPDY_ERROR_TOO_SMALL_BUFFER;

  BE_STORE_32(out, (frame->stream_id & 0x8FFFFFFF));
  out += 4;
  BE_STORE_32(out, frame->length);
  /* The flags are set after the length is written, because
   * otherwise the flags would get overwritten by the length. */
  out[0] = frame->flags;
  *outlen = 8;
  return SPDY_ERROR_NONE;
}

/*
 * Destroy/free all data this struct has allocated.
 */
void spdy_data_frame_destroy(spdy_data_frame *frame)
{
  if(frame->data) {
    free(frame->data);
    frame->data = NULL;
  }
}
