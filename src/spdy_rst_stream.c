#include "spdy_rst_stream.h"
#include "spdy_log.h"
#include "spdy_error.h"

#include <netinet/in.h>

/* Length of a RST_STREAM frame. (No minimum length - the length
 * of a RST_STREAM frame is always 8. */
const uint8_t SPDY_RST_STREAM_LENGTH = 8;

/**
 * Parse a RST_STREAM control frame.
 * @param rst_stream - Destination frame.
 * @param data - Data to parse.
 * @param data_length - Length of data.
 * @see spdy_rst_stream
 * @see SPDY_RST_STREAM_LENGTH
 * @return 0 on success, -1 on failure.
 */
int spdy_rst_stream_parse(spdy_rst_stream *rst_stream, char *data, size_t data_length) {
	if(data_length != SPDY_RST_STREAM_LENGTH) {
		SPDYDEBUG("Not enough data for parsing the header.");
		return SPDY_ERROR_INSUFFICIENT_DATA;
	}

	/* Read the Stream-ID. */
	rst_stream->stream_id = ntohl(*((uint32_t*)data)) & 0x7FFFFFFF;
	/* Read the status code. */
	rst_stream->status_code = ntohl(*((uint32_t*)data));

	return 0;
}

