#include "spdy_control_frame.h"
#include "spdy_syn_stream.h"
#include "spdy_syn_reply.h"
#include "spdy_rst_stream.h"
#include "spdy_log.h"
#include "spdy_error.h"

#include <netinet/in.h>
#include <stdlib.h>

// Minimum length of a control frame.
const uint8_t SPDY_CONTROL_FRAME_MIN_LENGTH = 8;

/**
 * Parse the header of a control frame.
 * @param frame - Target control frame.
 * @param data - Data to parse.
 * @param data_length - Length of data to parse.
 * @see spdy_control_frame
 * @todo Evaluate how to store data in the frame.
 * @return 0 on success, -1 on failure.
 */
int spdy_control_frame_parse_header(
		spdy_control_frame *frame,
		char *data,
		size_t data_length) {
	if(data_length < SPDY_CONTROL_FRAME_MIN_LENGTH) {
		SPDYDEBUG("Insufficient data for control frame.");
		return SPDY_ERROR_INSUFFICIENT_DATA;
	}
	// Read SPDY version. (AND is there to remove the first bit
	// which is used as frame type identifier.
	frame->version = ntohs(*((uint16_t*) data)) & 0x7FFF;
	data += 2;
	frame->type = ntohs(*((uint16_t*) data));
	data += 2;
	// Read one byte
	frame->flags = data[0];
	// Read four byte, including the flags byte and removing it with the AND.
	frame->length = ntohl(*((uint32_t*)data)) & 0x00FFFFFF;
	return 0;
}

/**
 * Parse a control frame.
 * @param frame - Target control fame.
 * @param data - Data to parse.
 * @param zlib_ctx - zlib context to use.
 * @see spdy_control_frame
 * @return Errorcode
 */
int spdy_control_frame_parse(
		spdy_control_frame *frame,
		spdy_data *data,
		spdy_zlib_context *zlib_ctx) {
	int ret;
	ret = spdy_control_frame_parse_header(frame, data->data, data->length);
	if(ret != SPDY_ERROR_NONE) {
		SPDYDEBUG("Control frame parse header failed.");
		return ret;
	}
	// Remove the header length from data_length.
	data->length -= SPDY_CONTROL_FRAME_MIN_LENGTH;
	data->data += SPDY_CONTROL_FRAME_MIN_LENGTH;
	data->used += SPDY_CONTROL_FRAME_MIN_LENGTH;

	if(frame->length > data->length) {
		data->needed = frame->length - data->length;
		SPDYDEBUG("Insufficient data for frame..");
		return SPDY_ERROR_INSUFFICIENT_DATA;
	}

	switch(frame->type) {
		// SYN_STREAM HANDLING
		case SPDY_CTRL_SYN_STREAM:
			frame->type_obj = malloc(sizeof(spdy_syn_stream));
			if(!frame->type_obj) {
				SPDYDEBUG("Failed to allocate space for SYN_STREAM.");
				return SPDY_ERROR_MALLOC_FAILED;
			}
					// TODO TODO TODO
					// Using frame length instead of data length, so that if the
					// buffer is larger as needed, it won't be handled by zlib
					// or anything similar.
			ret = spdy_syn_stream_parse(
					frame->type_obj,
					data,
					zlib_ctx);
			if(ret != SPDY_ERROR_NONE) {
				free(frame->type_obj);
				SPDYDEBUG("SYN_STREAM parsing failed.");
				return ret;
			}
			break;

			// SYN_REPLY HANDLING
		case SPDY_CTRL_SYN_REPLY:
			frame->type_obj = malloc(sizeof(spdy_syn_reply));
			if(!frame->type_obj) {
				SPDYDEBUG("Failed to allocate space for SYN_REPLY.");
				return SPDY_ERROR_MALLOC_FAILED;
			}
			ret = spdy_syn_reply_parse(
					frame->type_obj,
					data,
					zlib_ctx);
			if(ret != SPDY_ERROR_NONE) {
				free(frame->type_obj);
				SPDYDEBUG("SYN_REPLY parsing failed.");
				return ret;
			}
			break;

		// RST_STREAM HANDLING
		case SPDY_CTRL_RST_STREAM:
			frame->type_obj = malloc(sizeof(spdy_rst_stream));
			if(!frame->type_obj) {
				SPDYDEBUG("Failed to allocate space for RST_STREAM.");
				return SPDY_ERROR_MALLOC_FAILED;
			}
			ret = spdy_rst_stream_parse(
					frame->type_obj,
					data->data,
					frame->length);
			if(ret != SPDY_ERROR_NONE) {
				free(frame->type_obj);
				SPDYDEBUG("RST_STREAM parsing failed.");
				return ret;
			}
			break;
	}
	return SPDY_ERROR_NONE;
}

/**
 * Pack the control frame into a buffer for transmitting.
 * @param out Target buffer.
 * @param frame Frame to pack.
 * @see spdy_control_frame
 * @return SPDY_ERRORS
 */
int spdy_control_frame_pack_header(char **out, spdy_control_frame *frame) {
	*out = malloc(sizeof(char)*8);
	char *dat = *out;
	if(!dat) {
		SPDYDEBUG("Allocation of destination buffer failed.");
		return SPDY_ERROR_MALLOC_FAILED;
	}
	// The OR sets the first bit to true, indicating that this is a
	// control frame.
	*(uint16_t*)dat = htons(frame->version | 0x8000);
	dat += 2;
	*(uint16_t*)dat = htons(frame->type);
	dat += 2;
	*(uint32_t*)dat = htonl(frame->length);
	// The flags are set after the length is written, because elsewise
	// the flags would get overwritten by the length.
	dat[0] = frame->flags;
	return 0;
}

/**
 * Returns the name of the given control frame type.
 * @param type - Type of which the name is needed.
 * @return String with type name
 */
char *spdy_control_frame_get_type_name(int type) {
	switch(type) {
		case SPDY_CTRL_SYN_STREAM:
			return "SYN_STREAM";
		case SPDY_CTRL_SYN_REPLY:
			return "SYN_REPLY";
		case SPDY_CTRL_RST_STREAM:
			return "RST_STREAM";
		case SPDY_CTRL_SETTINGS:
			return "SETTINGS";
		case SPDY_CTRL_NOOP:
			return "NOOP";
		case SPDY_CTRL_PING:
			return "PING";
		case SPDY_CTRL_GOAWAY:
			return "GOAWAY";
		case SPDY_CTRL_HEADERS:
			return "HEADERS";
		case SPDY_CTRL_WINDOW_UPDATE:
			return "WINDOW_UPDATE";
		default:
			return "UNKNOWN";
	}
}

