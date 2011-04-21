#include "spdy_control_frame.h"

#include <netinet/in.h>
#include <stdlib.h>

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

/**
 * Pack the control frame into a buffer for transmitting.
 * @param out Target buffer.
 * @param frame Frame to pack.
 * @see spdy_control_frame
 * @return 0 on success, -1 on failure.
 */
int spdy_control_frame_pack_header(char **out, spdy_control_frame *frame) {
	*out = malloc(sizeof(char)*8);
	char *dat = *out;
	if(!dat) {
		return -1;
	}
	(void)frame;
	*(uint16_t*)dat = htons(frame->version | 0x8000);
	dat += 2;
	*(uint16_t*)dat = htons(frame->type);
	dat += 2;
	*(uint32_t*)dat = htonl(frame->length);
	dat[0] = frame->flags;
	return 0;
}

