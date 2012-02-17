#include "spdy_setup.h"         /* MUST be the first header to include */
#include "spdy_headers.h"
#include "spdy_log.h"
#include "spdy_error.h"
#include "spdy_bytes.h"
#include "spindly_phys.h"

#include <netinet/in.h>

/* Minimum length of a HEADERS frame. */
#define SPDY_HEADERS_MIN_LENGTH 8

int spdy_headers_parse_header(spdy_headers *headers, spdy_data *data)
{
  size_t length = data->data_end - data->cursor;
  if(length < SPDY_HEADERS_MIN_LENGTH) {
    SPDYDEBUG("Not enough data for parsing the header.");
    data->needed = SPDY_HEADERS_MIN_LENGTH - length;
    return SPDY_ERROR_INSUFFICIENT_DATA;
  }

  headers->stream_id = BE_LOAD_32(data->cursor) & 0x7FFFFFFF;
  data->cursor += 4;

  return SPDY_ERROR_NONE;
}

int spdy_headers_parse(spdy_headers *headers,
                       struct spindly_phys *phys,
                       spdy_data *data,
                       uint32_t frame_length)
{
  int ret;
  size_t length = data->data_end - data->cursor;

  if(length < SPDY_HEADERS_MIN_LENGTH) {
    data->needed = SPDY_HEADERS_MIN_LENGTH - length;
    SPDYDEBUG("Not enough data for parsing the frame.");
    return SPDY_ERROR_INSUFFICIENT_DATA;
  }

  if((ret = spdy_headers_parse_header(headers, data)) != SPDY_ERROR_NONE) {
    SPDYDEBUG("Failed to parse header.");
    return ret;
  }

  /* Parse NV block. */
  ret = spdy_nv_block_inflate_parse(headers->nv_block,
                                    data->cursor,
                                    frame_length,
                                    &phys->zlib_in);
  if(ret != SPDY_ERROR_NONE) {
    /* Clean up. */
    SPDYDEBUG("Failed to parse NV block.");
    return ret;
  }
  data->cursor += frame_length - SPDY_HEADERS_MIN_LENGTH;

  return SPDY_ERROR_NONE;
}
