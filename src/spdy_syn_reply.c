#include "spdy_setup.h"         /* MUST be the first header to include */

#include <string.h>
#include <netinet/in.h>
#include "spdy_syn_reply.h"
#include "spdy_log.h"
#include "spdy_error.h"
#include "spdy_bytes.h"
#include "spindly_phys.h"


/* Minimum length of a SYN_REPLY frame. */
#define SPDY_SYN_REPLY_MIN_LENGTH 8
/* Minimum length of a SYN_STREAM frame header. (The frame without
 * the NV Block.) */
#define SPDY_SYN_REPLY_HEADER_MIN_LENGTH 6

/**
 * Parse the header of a SYN_REPLY control frame.
 * This function can be used to parse the header of a SYN_REPLY frame
 * before the whole NV block has been received.
 * @param syn_reply - Destination frame.
 * @param data - Data to parse.
 * @see SPDY_SYN_REPLY_HEADER_MIN_LENGTH
 * @return 0 on success, 01 on failure.
 */
int spdy_syn_reply_parse_header(spdy_syn_reply *syn_reply, spdy_data *data)
{
  size_t length = data->data_end - data->cursor;
  if(length < SPDY_SYN_REPLY_HEADER_MIN_LENGTH) {
    SPDYDEBUG("Not enough data for parsing the header.");
    data->needed = SPDY_SYN_REPLY_HEADER_MIN_LENGTH - length;
    return SPDY_ERROR_INSUFFICIENT_DATA;
  }

  /* Read the Stream-ID. */
  syn_reply->stream_id = BE_LOAD_32(data->cursor) & 0x7FFFFFFF;
  /* Skip Stream-ID and 2 bytes of unused space. */
  data->cursor += 6;

  return SPDY_ERROR_NONE;
}

/**
 * Parse a SYN_REPLY control frame.
 * Parses the header of a SYN_REPLY control frame and extracts the
 * NV block.
 * @param syn_reply - Destination frame.
 * @param hash - streamid lookup
 * @param data - Data to parse.
 * @param frame_length - Length of the frame.
 * @see SPDY_SYN_STREAM_MIN_LENGTH
 * @return 0 on success, -1 on failure.
 */
int spdy_syn_reply_parse(spdy_syn_reply *syn_reply,
                         struct spindly_phys *phys,
                         spdy_data *data,
                         uint32_t frame_length)
{
  int ret;
  size_t length = data->data_end - data->cursor;

  if(length < SPDY_SYN_REPLY_MIN_LENGTH) {
    SPDYDEBUG("Not enough data for parsing the stream.");
    data->needed = SPDY_SYN_REPLY_MIN_LENGTH - length;
    return SPDY_ERROR_INSUFFICIENT_DATA;
  }

  /* Parse the frame header. */
  if((ret = spdy_syn_reply_parse_header(syn_reply, data)) != SPDY_ERROR_NONE) {
    return ret;
  }

  /* Init NV block. */
  ret = spdy_nv_block_init(&syn_reply->nv_block);
  if(ret) {
    return ret;
  }

  /* Parse NV block */
  ret = spdy_nv_block_inflate_parse(&syn_reply->nv_block,
                                    data->cursor,
                                    frame_length,
                                    &phys->zlib_in);
  if(ret != SPDY_ERROR_NONE) {
    /* Clean up. */
    SPDYDEBUG("Failed to parse NV block.");
    return ret;
  }
  data->cursor += frame_length - SPDY_SYN_REPLY_HEADER_MIN_LENGTH;

  return SPDY_ERROR_NONE;
}


/*
 * Pack SYN_REPLY into an output buffer for transmitting.
 */
int spdy_syn_reply_pack(unsigned char *out, size_t bufsize,
                        size_t *outsize, spdy_syn_reply *rep)
{
  char buf[2];
  size_t consumed;
  char *deflated;
  size_t deflated_length;
  int rc;

  if(bufsize < 6)
    return SPDY_ERROR_TOO_SMALL_BUFFER;
  BE_STORE_32(out, rep->stream_id);
  out += 4;
  BE_STORE_16(out, 0); /* 16 bits unused */
  out += 2;

  /* create the NV block to include */
  BE_STORE_16(buf, 0); /* 16 bit NV pair counter */

  rc = spdy_zlib_deflate(buf, 2, &consumed, &deflated, &deflated_length);
  if(rc)
    return rc;

  if(bufsize < (10 + deflated_length)) {
    free(deflated);
    return SPDY_ERROR_TOO_SMALL_BUFFER;
  }

  memcpy(out, deflated, deflated_length);
  free(deflated);

  *outsize = 6 + deflated_length;
  return SPDY_ERROR_NONE;
}

/*
 * Destroy/free all data this struct has allocated.
 */
void spdy_syn_reply_destroy(spdy_syn_reply *syn_reply)
{
  spdy_nv_block_destroy(&syn_reply->nv_block);
}
