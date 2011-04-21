#include "spdy_frame.h"

#include <stdlib.h>

/**
 * Parse the header of a frame.
 * @param frame Target frame.
 * @param data Data to parse.
 * @see spdy_frame
 * @see spdy_control_frame
 * @see spdy_data_frame
 * @see SPDY_FRAME_TYPE
 * @return 0 on success, -1 on failure.
 */
int spdy_frame_parse_header(spdy_frame *frame, char *data) {
	// Read type bit
	frame->type = (data[0] & 0x80)>>7;
	switch(frame->type) {
		case SPDY_DATA_FRAME:
			// Allocate space for data frame.
			frame->frame = malloc(sizeof(spdy_data_frame));
			if(!frame->frame) {
				return -1;
			}
			break;
		case SPDY_CONTROL_FRAME:
			// Allocate space for control frame.
			frame->frame = malloc(sizeof(spdy_control_frame));
			if(!frame->frame) {
				return -1;
			}
			break;
		default:
			// This should never happen.
			return -1;
	}
	return 0;
}

