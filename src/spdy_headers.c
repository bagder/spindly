#include "spdy_headers.h"
#include "spdy_log.h"
#include "spdy_error.h"

#include <netinet/in.h>

/* Minimum length of a HEADERS frame. */
const uint8_t SPDY_HEADERS_MIN_LENGTH  = 8;

int spdy_headers_parse_header(
		spdy_headers *headers,
		char *data,
		size_t data_length) {
	if(data_length < SPDY_HEADERS_MIN_LENGTH) {
		SPDYDEBUG("Not enough data for parsing the header.");
		return SPDY_ERROR_INSUFFICIENT_DATA;
	}

	headers->stream_id = ntohl(*((uint32_t*)data)) & 0x7FFFFFFF;

	return SPDY_ERROR_NONE;
}

int spdy_headers_parse(
		spdy_headers *headers,
		spdy_data *data,
		spdy_zlib_context *zlib_ctx) {
	int ret;
	if(data->length < SPDY_HEADERS_MIN_LENGTH) {
		data->needed = SPDY_HEADERS_MIN_LENGTH;
		SPDYDEBUG("Not enough data for parsing the frame.");
		return SPDY_ERROR_INSUFFICIENT_DATA;
	}

	if((ret = spdy_headers_parse_header(
					headers,
					data->data,
					data->length)) != SPDY_ERROR_NONE) {
		SPDYDEBUG("Failed to parse header.");
		return ret;
	}

	// Skip the (already parsed) header.
	data->data += SPDY_HEADERS_MIN_LENGTH;
	data->length -= SPDY_HEADERS_MIN_LENGTH;
	data->used += SPDY_HEADERS_MIN_LENGTH;

	// Parse NV block.
	if((ret = spdy_nv_block_inflate_parse(
					headers->nv_block,
					data->data,
					data->length,
					zlib_ctx)) != SPDY_ERROR_NONE) {
		// Clean up.
		SPDYDEBUG("Failed to parse NV block.");
		return ret;
	}
	data->used += data->length;

	return SPDY_ERROR_NONE;
}

