#ifndef SPDY_SYN_STREAM_H_
#define SPDY_SYN_STREAM_H_ 1

#include "spdy_nv_block.h"
#include "spdy_zlib.h"

#include <stdint.h>
#include <stdlib.h>

// Minimum length of a SYN_STREAM frame.
extern const uint8_t SPDY_SYN_STREAM_MIN_LENGTH;
// Minimum length of a SYN_STREAM frame header.
extern const uint8_t SPDY_SYN_STREAM_HEADER_MIN_LENGTH;

/**
 * SYN_STREAM control frame
 */
typedef struct {
	uint16_t stream_id;      /*!< 31 bit stream id */
	uint16_t associated_to;  /*!< 31 bit assocaited to stream id */
	uint8_t priority;        /*!< 2 bit priority */
	spdy_nv_block *nv_block; /*!< Name/Value block */
} spdy_syn_stream;

int spdy_syn_stream_parse_header(spdy_syn_stream *syn_stream, char *data, size_t data_length);
int spdy_syn_stream_parse(spdy_syn_stream *syn_stream, char *data, size_t data_length, spdy_zlib_context *zlib_ctx);

#endif

