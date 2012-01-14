#include "spdy_setup.h" /* MUST be the first header to include */
#include "spdy_control_frame.h"
#include "spdy_syn_stream.h"
#include "spdy_syn_reply.h"
#include "spdy_rst_stream.h"
#include "spdy_headers.h"
#include "spdy_log.h"
#include "spdy_error.h"
#include "spdy_bytes.h"

#include <netinet/in.h>
#include <stdlib.h>

/** Minimum length of a control frame. */
const uint8_t SPDY_CONTROL_FRAME_MIN_LENGTH = 8;

int spdy_control_frame_init(spdy_control_frame *frame) {
	frame->_header_parsed = 0;
	return SPDY_ERROR_NONE;
}

/**
 * Parse the header of a control frame.
 * @param frame - Target control frame.
 * @param data - Data to parse.
 * @see spdy_control_frame
 * @todo Evaluate how to store data in the frame.
 * @return 0 on success, -1 on failure.
 */
int spdy_control_frame_parse_header(
		spdy_control_frame *frame,
		spdy_data *data) {
	size_t length;
	/* Check if the header has already been parsed. */
	if(frame->_header_parsed)
		return SPDY_ERROR_NONE;

	length = data->data_end - data->cursor;
	if(length < SPDY_CONTROL_FRAME_MIN_LENGTH) {
		SPDYDEBUG("Insufficient data for control frame.");
		data->needed = SPDY_CONTROL_FRAME_MIN_LENGTH - length;
		return SPDY_ERROR_INSUFFICIENT_DATA;
	}
	/* Read SPDY version. (AND is there to remove the first bit
	 * which is used as frame type identifier. */
	frame->version = BE_LOAD_16(data->cursor) & 0x7FFF;
	data->cursor += 2;
	frame->type = BE_LOAD_16(data->cursor);
	data->cursor += 2;
	/* Read one byte */
	frame->flags = (uint8_t)data->cursor[0];
	/* Read four byte, including the flags byte and removing it with the AND. */
	frame->length = BE_LOAD_32(data->cursor) & 0x00FFFFFF;
	data->cursor += 4;
	frame->_header_parsed = 1;
	return SPDY_ERROR_NONE;
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
	size_t length;
	if(!frame->_header_parsed) {
		ret = spdy_control_frame_parse_header(frame, data);
		if(ret != SPDY_ERROR_NONE) {
			SPDYDEBUG("Control frame parse header failed.");
			return ret;
		}
	}

	/* TODO: Check if control_frame_min_length is contained in length or not */
	length = data->data_end - data->cursor;
	if(frame->length > length) {
		data->needed = frame->length - length;
		SPDYDEBUG("Insufficient data for control frame.");
		return SPDY_ERROR_INSUFFICIENT_DATA;
	}

	switch(frame->type) {
		case SPDY_CTRL_SYN_STREAM:
			ret = spdy_syn_stream_parse(
					&frame->obj.syn_stream,
					data,
					frame->length,
					zlib_ctx);
			if(ret != SPDY_ERROR_NONE) {
				SPDYDEBUG("SYN_STREAM parsing failed.");
				return ret;
			}
			break;

		case SPDY_CTRL_SYN_REPLY:
			ret = spdy_syn_reply_parse(
					&frame->obj.syn_reply,
					data,
					frame->length,
					zlib_ctx);
			if(ret != SPDY_ERROR_NONE) {
				SPDYDEBUG("SYN_REPLY parsing failed.");
				return ret;
			}
			break;

		case SPDY_CTRL_RST_STREAM:
			ret = spdy_rst_stream_parse(
					&frame->obj.rst_stream,
					data->cursor,
					frame->length);
			if(ret != SPDY_ERROR_NONE) {
				SPDYDEBUG("RST_STREAM parsing failed.");
				return ret;
			}
			break;

		case SPDY_CTRL_HEADERS:
			ret = spdy_headers_parse(
					&frame->obj.headers,
					data,
					frame->length,
					zlib_ctx);
			if(ret != SPDY_ERROR_NONE) {
				SPDYDEBUG("HEADERS parsing failed.");
				return SPDY_ERROR_INVALID_DATA;
			}
			break;
	}
	return SPDY_ERROR_NONE;
}

/**
 * Pack the control frame into an output buffer for transmitting.
 * @param out Target buffer
 * @param buffer Length of target buffer
 * @param outsize Pointer to length of the output data
 * @param frame Frame to pack
 * @see spdy_control_frame
 * @return SPDY_ERRORS
 */
int spdy_control_frame_pack_header(char *out, size_t bufsize,
                                   size_t *outsize,
                                   spdy_control_frame *frame)
{
        if(bufsize < 8)
          return SPDY_ERROR_TOO_SMALL_BUFFER;

	/* The OR sets the first bit to true, indicating that this is a
	 * control frame. */
	BE_STORE_16(out, (frame->version | 0x8000));
	out += 2;
	BE_STORE_16(out, frame->type);
	out += 2;
	BE_STORE_32(out, frame->length);
	/* The flags are set after the length is written, because elsewise
	 * the flags would get overwritten by the length. */
	out[0] = frame->flags;

        *outsize = 8;
	return SPDY_ERROR_NONE;
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

void spdy_control_frame_destroy(spdy_control_frame *frame)
{
  switch(frame->type) {
    case SPDY_CTRL_SYN_STREAM:
      spdy_syn_stream_destroy(&frame->obj.syn_stream);
      break;
  default:
    break;
  }
}
