#include "spdy_syn_reply.h"
#include "spdy_log.h"
#include "spdy_error.h"

#include <netinet/in.h>

/* Minimum length of a SYN_REPLY frame. */
const uint8_t SPDY_SYN_REPLY_MIN_LENGTH = 8;
/* Minimum length of a SYN_STREAM frame header. (The frame without
 * the NV Block.) */
const uint8_t SPDY_SYN_REPLY_HEADER_MIN_LENGTH = 6;

/**
 * Parse the header of a SYN_REPLY control frame.
 * This function can be used to parse the header of a SYN_REPLY frame
 * before the whole NV block has been received.
 * @param syn_reply - Destination frame.
 * @param data - Data to parse.
 * @param data_length - Length of data.
 * @see SPDY_SYN_REPLY_HEADER_MIN_LENGTH
 * @return 0 on success, 01 on failure.
 */
int spdy_syn_reply_parse_header(spdy_syn_reply *syn_reply, char *data, size_t data_length) {
	if(data_length < SPDY_SYN_REPLY_HEADER_MIN_LENGTH) {
		SPDYDEBUG("Not enough data for parsing the header.");
		return SPDY_ERROR_INSUFFICIENT_DATA;
	}

	// Read the Stream-ID.
	syn_reply->stream_id = ntohl(*((uint32_t*)data)) & 0x7FFFFFFF;
	// Skip Stream-ID and 2 bytes of unused space.
	data += 6;

	return 0;
}

/**
 * Parse a SYN_REPLY control frame.
 * Parses the header of a SYN_REPLY control frame and extracts the
 * NV block.
 * @param syn_reply - Destination frame.
 * @param data - Data to parse.
 * @param zlib_ctx - The zlib context to use.
 * @see SPDY_SYN_STREAM_MIN_LENGTH
 * @return 0 on success, -1 on failure.
 */
int spdy_syn_reply_parse(
		spdy_syn_reply *syn_reply,
		spdy_data *data,
		spdy_zlib_context *zlib_ctx) {
	int ret;
	if(data->length < SPDY_SYN_REPLY_MIN_LENGTH) {
		SPDYDEBUG("Not enough data for parsing the stream.");
		return SPDY_ERROR_INSUFFICIENT_DATA;
	}

	// Parse the frame header.
	if((ret = spdy_syn_reply_parse_header(syn_reply, data->data, data->length)) != SPDY_ERROR_NONE)
	{
		SPDYDEBUG("Failed to parse header.");
		return ret;
	}

	// Skip the (already parsed) header.
	data->data += SPDY_SYN_REPLY_HEADER_MIN_LENGTH;
	data->length -= SPDY_SYN_REPLY_HEADER_MIN_LENGTH;

	// Parse NV block.
	if((ret = spdy_nv_block_inflate_parse(
					syn_reply->nv_block,
					data->data,
					data->length,
					zlib_ctx)) != SPDY_ERROR_NONE) {
		// Clean up.
		SPDYDEBUG("Failed to parse NV block.");
		return ret;
	}
	data->used += data->length;

	return 0;
}

