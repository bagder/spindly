#ifndef SPDY_RST_STREAM_H_
#define SPDY_RST_STREAM_H_ 1

#include <stdint.h>
#include <stdlib.h>

/**
 * SPDY Status codes as used in RST_STREAM frames.
 */
enum SPDY_STATUS_CODES
{
  SPDY_PROTOCOL_ERROR = 1,      /*!< PROTOCOL_ERROR */
  SPDY_INVALID_STREAM = 2,      /*!< INVALID_STREAM */
  SPDY_REFUSED_STREAM = 3,      /*!< REFUSED_STREAM */
  SPDY_UNSUPPORTED_VERSION = 4, /*!< UNSUPPORTED_VERSION */
  SPDY_CANCEL = 5,              /*!< CANCEL */
  SPDY_INTERNAL_ERROR = 6,      /*!< INTERNAL_ERROR */
  SPDY_FLOW_CONTROL_ERROR = 7   /*!< FLOW_CONTROL_ERROR */
};

/**
 * RST_STREAM control frame
 */
typedef struct
{
  uint32_t stream_id;           /*!< 31 bit stream id */
  uint32_t status_code;         /*!< 32 bit status code */
} spdy_rst_stream;

int spdy_rst_stream_parse(spdy_rst_stream *rst_stream, unsigned char *data,
                          size_t data_length);

#endif
