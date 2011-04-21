#include "spdy_control_frame.h"

#include <netinet/in.h>

/**
 * Parse the header of a control frame.
 * @param frame Target control frame.
 * @param data Data to parse.
 * @see spdy_control_frame
 * @todo Evaluate how to store data in the frame.
 * @return 0 on success, -1 on failure.
 */
int spdy_control_frame_parse_header(spdy_control_frame *frame, char *data) {
	// Read frame version. (AND is there to remove the first bit
	// which is used as frame type identifier.
	frame->version = ntohs(*((uint16_t*) data)) & 0x7FFF;
	data += 2;
	frame->type = ntohs(*((uint16_t*) data));
	data += 2;
	// Read one byte
	frame->flags = data[0];
	// Read four byte, including the flags byte and removing it with the AND.
	frame->length = ntohl(*((uint32_t*)data)) & 0x00FFFFFF;
	frame->data = data + 4;
	return 0;
}

