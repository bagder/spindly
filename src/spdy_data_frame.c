#include "spdy_data_frame.h"
#include "spdy_log.h"
#include "spdy_error.h"

#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>

// Minimum length of a data frame.
const uint8_t SPDY_DATA_FRAME_MIN_LENGTH = 8;

/**
 * Parse the header of a data frame.
 * @param frame - Target data frame.
 * @param data - Data to parse.
 * @param data_length - Length of data.
 * @see spdy_data_frame
 * @return Errorcode
 */
int spdy_data_frame_parse_header(
		spdy_data_frame *frame,
		char *data,
		size_t data_length) {
	if(data_length < SPDY_DATA_FRAME_MIN_LENGTH) {
		SPDYDEBUG("Insufficient data for data frame.");
		return SPDY_ERROR_INSUFFICIENT_DATA;
	}
	// Read stream id. (AND removes the first type bit.)
	frame->stream_id = ntohl(*((uint32_t*)data)) & 0x7FFFFFFF;
	data += 4;
	frame->flags = data[0];
	frame->length = ntohl(*((uint32_t*)data)) & 0x00FFFFFF;
	return SPDY_ERROR_NONE;
}

/**
 * Parse a data frame
 * @param frame - Target data frame.
 * @param data - Data to parse.
 * @see spdy_data_frame
 * @return Errorcode
 */
int spdy_data_frame_parse(
		spdy_data_frame *frame,
		spdy_data *data) {
	int ret;
	ret = spdy_data_frame_parse_header(frame, data->data, data->length);
	if(ret != SPDY_ERROR_NONE) {
		SPDYDEBUG("Data frame parse header failed.");
		return ret;
	}

	data->length -= SPDY_DATA_FRAME_MIN_LENGTH;
	data->data += SPDY_DATA_FRAME_MIN_LENGTH;
	data->used += SPDY_DATA_FRAME_MIN_LENGTH;

	if(frame->length > data->length) {
		data->needed = frame->length - data->length;
		SPDYDEBUG("Insufficient data for data frame.");
		return SPDY_ERROR_INSUFFICIENT_DATA;
	}

	frame->data = malloc(sizeof(char) * frame->length);
	if(!frame->data) {
		SPDYDEBUG("Frame payload malloc failed.");
		return SPDY_ERROR_MALLOC_FAILED;
	}
	memcpy(frame->data, data->data, frame->length);
	data->length -= frame->length;
	data->data += frame->length;
	data->used += frame->length;

	return SPDY_ERROR_NONE;
}

/**
 * Pack the data frame into a buffer for transmitting.
 * @param out Target buffer.
 * @param frame Frame to pack.
 * @see spdy_data_frame
 * @return Errorcode
 */
int spdy_data_frame_pack_header(char **out, spdy_data_frame *frame) {
	*out = malloc(sizeof(char)*8);
	char *dat = *out;
	if(!dat) {
		SPDYDEBUG("Allocation of destination buffer failed.");
		return SPDY_ERROR_MALLOC_FAILED;
	}
	*(uint32_t*)dat = htonl(frame->stream_id & 0x8FFFFFFF);
	dat += 4;
	*(uint32_t*)dat = htonl(frame->length);
	// The flags are set after the legnth is writte, because
	// elsewise the flags would get overwritten by the length.
	dat[0] = frame->flags;
	return SPDY_ERROR_NONE;
}

