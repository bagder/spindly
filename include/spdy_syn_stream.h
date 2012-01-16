#ifndef SPDY_SYN_STREAM_H_
#define SPDY_SYN_STREAM_H_ 1

#include "spdy_data.h"
#include "spdy_nv_block.h"
#include "spdy_zlib.h"

#include <stdint.h>
#include <stdlib.h>

/** Minimum length of a SYN_STREAM frame. */
extern const uint8_t SPDY_SYN_STREAM_MIN_LENGTH;
/** Minimum length of a SYN_STREAM frame header. */
extern const uint8_t SPDY_SYN_STREAM_HEADER_MIN_LENGTH;

/**
 * Flags for SYN_STREAM frames.
 */
enum SPDY_SYN_STREAM_FLAGS
{
  SPDY_SYN_STREAM_FLAG_FIN = 0x01,      /*!< FLAG_FIN */
  SPDY_SYN_STREAM_FLAG_UNIDIRECTIONAL = 0x02    /*!< FLAG_UNIDIRECTIONAL */
};

/**
 * SYN_STREAM control frame
 */
typedef struct
{
  uint16_t stream_id;           /*!< 31 bit stream id */
  uint16_t associated_to;       /*!< 31 bit assocaited to stream id */
  uint8_t priority;             /*!< 2 bit priority */
  spdy_nv_block nv_block;       /*!< Name/Value block */
} spdy_syn_stream;

int spdy_syn_stream_parse_header(spdy_syn_stream *syn_stream, spdy_data *data);
int spdy_syn_stream_parse(spdy_syn_stream *syn_stream,
                          spdy_data *data,
                          uint32_t frame_length, spdy_zlib_context *zlib_ctx);

void spdy_syn_stream_destroy(spdy_syn_stream *syn_stream);

#endif
