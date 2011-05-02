#include "spdy_frame.h"
#include "spdy_control_frame.h"
#include "spdy_data_frame.h"
#include "spdy_error.h"
#include "spdy_log.h"

#include <stdlib.h>

/**
 * Parse the header of a frame.
 * @param frame - Target frame.
 * @param data - Data to parse.
 * @param data_length - Length of data.
 * @see spdy_frame
 * @see spdy_control_frame
 * @see spdy_data_frame
 * @return 0 on success, -1 on failure.
 */
int spdy_frame_parse_header(
		spdy_frame *frame,
		char *data,
		size_t data_length) {
	// Read type bit
	frame->type = (data[0] & 0x80)>>7;
	frame->frame = NULL;
	int ret;
	switch(frame->type) {
		case SPDY_DATA_FRAME:
			// Allocate space for data frame.
			frame->frame = malloc(sizeof(spdy_data_frame));
			if(!frame->frame) {
				SPDYDEBUG("Allocating of space for data frame failed.");
				return SPDY_ERROR_MALLOC_FAILED;
			}
			if((ret = spdy_data_frame_parse_header(frame->frame, data)) != SPDY_ERROR_NONE) {
				free(frame->frame);
				frame->frame = NULL;
				return ret;
			}
			break;
		case SPDY_CONTROL_FRAME:
			// Allocate space for control frame.
			frame->frame = malloc(sizeof(spdy_control_frame));
			if(!frame->frame) {
				SPDYDEBUG("Allocation of space for control frame failed.");
				return SPDY_ERROR_MALLOC_FAILED;
			}
			// Parse frame header.
			if((ret = spdy_control_frame_parse_header(frame->frame, data, data_length)) != SPDY_ERROR_NONE) {
				free(frame->frame);
				frame->frame = NULL;
				return ret;
			}
			break;
		default:
			// This should never happen.
			return SPDY_ERROR_INVALID_DATA;
	}
	return 0;
}

/**
 * Parse a frame.
 * @param frame - Target frame.
 * @param data - Data to parse.
 * @param data_length - Length of data to parse.
 * @param zlib_ctx - zlib context to use.
 * @see spdy_frame
 * @return Errorcode
 */
int spdy_frame_parse(
		spdy_frame *frame,
		char *data,
		size_t data_length,
		spdy_zlib_context *zlib_ctx) {
	int ret;
	ret = spdy_frame_parse_header(frame, data, data_length);
	if(ret != SPDY_ERROR_NONE) {
		SPDYDEBUG("Frame parse header failed.");
		return ret;
	}
	switch(frame->type) {
		case SPDY_CONTROL_FRAME:
			frame->frame = malloc(sizeof(spdy_control_frame));
			if(!frame->frame) {
				SPDYDEBUG("Control frame malloc failed.");
				return SPDY_ERROR_MALLOC_FAILED;
			}

			ret = spdy_control_frame_parse(frame->frame, data, data_length, zlib_ctx);
			if(ret != SPDY_ERROR_NONE) {
				SPDYDEBUG("Control frame parse failed.");
				return ret;
			}
			break;
		case SPDY_DATA_FRAME:
			SPDYDEBUG("WHOT");
			break;
		default:
			SPDYDEBUG("UNSUPPORTED");
			break;
	}
	return SPDY_ERROR_NONE;
}

