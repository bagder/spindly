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
 * @see spdy_frame
 * @return Errorcode
 */
int spdy_frame_parse_header(
		spdy_frame *frame,
		spdy_data *data) {

	/**
	 * Read the type bit
	 * (The mask equals 0x10000000, filtering all but the first bit. Then
	 * we shift it over by 7 digits, giving us a char with a value which
	 * either is 0 or 1.)
	 */
	frame->type = (data->cursor[0] & 0x80)>>7;
	frame->frame = NULL;
	return SPDY_ERROR_NONE;
}

/**
 * Parse a frame.
 * @param frame - Target frame.
 * @param data - Data to parse.
 * @param zlib_ctx - zlib context to use.
 * @see spdy_frame
 * @return Errorcode
 */
int spdy_frame_parse(
		spdy_frame *frame,
		spdy_data *data,
		spdy_zlib_context *zlib_ctx) {
	int ret;
	ret = spdy_frame_parse_header(frame, data);
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
			spdy_control_frame_init(frame->frame);
			ret = spdy_control_frame_parse(
					frame->frame,
					data,
					zlib_ctx);
			if(ret != SPDY_ERROR_NONE) {
				SPDYDEBUG("Control frame parse failed.");
				return ret;
			}
			break;
		case SPDY_DATA_FRAME:
			frame->frame = malloc(sizeof(spdy_data_frame));
			if(!frame->frame) {
				SPDYDEBUG("Data frame malloc failed.");
				return SPDY_ERROR_MALLOC_FAILED;
			}

			ret = spdy_data_frame_parse(
					frame->frame,
					data);
			if(ret != SPDY_ERROR_NONE) {
				SPDYDEBUG("Data frame parse failed.");
				return ret;
			}
			break;
		default:
			SPDYDEBUG("UNSUPPORTED");
			break;
	}
	return SPDY_ERROR_NONE;
}

