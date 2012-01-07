#include "spdy_setup.h" /* MUST be the first header to include */
#include "spdy_syn_stream.h"
#include "spdy_log.h"
#include "spdy_error.h"
#include "spdy_bytes.h"

#include <netinet/in.h>

/* Minimum length of a SYN_STREAM frame. */
const uint8_t SPDY_SYN_STREAM_MIN_LENGTH = 12;
/* Minimum length of a SYN_STREAM frame header. (The frame without
 * the NV block.) */
const uint8_t SPDY_SYN_STREAM_HEADER_MIN_LENGTH = 10;

/**
 * Parse the header of a SYN_STREAM control frame.
 * This function can be used to parse the header of a SYN_STREAM frame
 * before the whole NV block has been recevied. (Minimum of bytes needed
 * is stored in SPDY_SYN_STREAM_HEADER_MIN_LENGTH.)
 * @param syn_stream - Destination frame.
 * @param data - Data to parse.
 * @param data_length - Length of data.
 * @see SPDY_SYN_STREAM_HEADER_MIN_LENGTH
 * @return Errorcode
 */
int spdy_syn_stream_parse_header(spdy_syn_stream *syn_stream, spdy_data *data) {
	size_t length = data->data_end - data->cursor;
	if(length < SPDY_SYN_STREAM_HEADER_MIN_LENGTH) {
		SPDYDEBUG("Not enough data for parsing the header.");
		data->needed = SPDY_SYN_STREAM_HEADER_MIN_LENGTH - length;
		return SPDY_ERROR_INSUFFICIENT_DATA;
	}

	/* Read the Stream-ID. */
	syn_stream->stream_id = BE_LOAD_32(data->cursor) & 0x7FFFFFFF;
	data->cursor += 4;
	/* Read the 'Associated-To-Stream-ID'. */
	syn_stream->associated_to = BE_LOAD_32(data->cursor) & 0x7FFFFFFF;
	data->cursor += 4;
	/* Read the two priority bits. */
	syn_stream->priority = (data->cursor[0] & 0xC0) >> 6;
	/* Skip the unused block. */
	data->cursor += 2;

	return SPDY_ERROR_NONE;
}

/**
 * Parse a SYN_STREAM control frame.
 * Parses the header of a SYN_STREAM control frame and extracts the
 * NV block.
 * @param syn_stream - Destination frame.
 * @param data - Data to parse.
 * @param frame_length - Length of the frame.
 * @param zlib_ctx - The zlib context to use.
 * @see spdy_control_frame
 * @see SPDY_SYN_STREAM_MIN_LENGTH
 * @return 0 on success, -1 on failure.
 */
int spdy_syn_stream_parse(
		spdy_syn_stream *syn_stream,
		spdy_data *data,
		uint32_t frame_length,
		spdy_zlib_context *zlib_ctx) {
	int ret;
	size_t length = data->data_end - data->cursor;
	if(length < frame_length) {
		data->needed = frame_length - length;
		SPDYDEBUG("Not enough data for parsing the stream.");
		return SPDY_ERROR_INSUFFICIENT_DATA;
	}
	/* TODO: Optimize the double length check away. */
	if(length < SPDY_SYN_STREAM_MIN_LENGTH) {
		data->needed = SPDY_SYN_STREAM_MIN_LENGTH - length;
		SPDYDEBUG("Not enough data for parsing the stream.");
		return SPDY_ERROR_INSUFFICIENT_DATA;
	}

	/* Parse the frame header. */
	if((ret = spdy_syn_stream_parse_header(
					syn_stream,
					data)) != SPDY_ERROR_NONE) {
		SPDYDEBUG("Failed to parse header.");
		return ret;
	}

	/* Create NV block. */
	ret = spdy_nv_block_create(&syn_stream->nv_block);
	if(ret) {
		return ret;
	}

	/* Parse NV block. */
	if((ret = spdy_nv_block_inflate_parse(
					syn_stream->nv_block,
					data->cursor,
					frame_length,
					zlib_ctx)) != SPDY_ERROR_NONE) {
		/* Clean up. */
		SPDYDEBUG("Failed to parse NV block.");
		return ret;
	}
	data->cursor += frame_length-SPDY_SYN_STREAM_HEADER_MIN_LENGTH;

	return SPDY_ERROR_NONE;
}

