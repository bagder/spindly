#include "spdy_data_frame.h"
#include "spdy_log.h"
#include "spdy_error.h"

#include <netinet/in.h>
#include <stdlib.h>

/**
 * Parse the header of a data frame.
 * @param frame Target data frame.
 * @param data Data to parse.
 * @see spdy_data_frame
 * @return 0 on success, -1 on failure.
 */
int spdy_data_frame_parse_header(spdy_data_frame *frame, char *data) {
	// Read stream id. (AND removes the first type bit.)
	frame->stream_id = ntohl(*((uint32_t*)data)) & 0x7FFFFFFF;
	data += 4;
	frame->flags = data[0];
	frame->length = ntohl(*((uint32_t*)data)) & 0x00FFFFFF;
	frame->data = data+4;
	return 0;
}

/**
 * Pack the data frame into a buffer for transmitting.
 * @param out Target buffer.
 * @param frame Frame to pack.
 * @see spdy_data_frame
 * @return 0 on success, -1 on failure.
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
	return 0;
}

