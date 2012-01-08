#include "spdy_setup.h" /* MUST be the first header to include */
#include "spdy_data_frame.h"
#include "spdy_log.h"
#include "spdy_error.h"
#include "spdy_bytes.h"

#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>

/* Minimum length of a data frame. */
const uint8_t SPDY_DATA_FRAME_MIN_LENGTH = 8;

/**
 * Parse the header of a data frame.
 * @param frame - Target data frame.
 * @param data - Data to parse.
 * @see spdy_data_frame
 * @return Errorcode
 */
int spdy_data_frame_parse_header(
		spdy_data_frame *frame,
		spdy_data *data) {

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
int spdy_data_frame_parse(
		spdy_data_frame *frame,
		spdy_data *data) {
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

	frame->data = malloc(sizeof(char) * frame->length);
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
 * @param out Target buffer.
 * @param frame Frame to pack.
 * @see spdy_data_frame
 * @return Errorcode
 */
int spdy_data_frame_pack_header(char **out, spdy_data_frame *frame) {
	char *dat;
	*out = malloc(sizeof(char)*8);
	dat = *out;
	if(!dat) {
		SPDYDEBUG("Allocation of destination buffer failed.");
		return SPDY_ERROR_MALLOC_FAILED;
	}
	BE_STORE_32(dat, (frame->stream_id & 0x8FFFFFFF));
	dat += 4;
	BE_STORE_32(dat, frame->length);
	/* The flags are set after the length is written, because
	 * otherwise the flags would get overwritten by the length. */
	dat[0] = frame->flags;
	return SPDY_ERROR_NONE;
}

