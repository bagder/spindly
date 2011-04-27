#include "spdy_syn_stream.h"
#include "spdy_log.h"
#include "spdy_error.h"

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
 * @return 0 on success, -1 on failure.
 */
int spdy_syn_stream_parse_header(spdy_syn_stream *syn_stream, char *data, size_t data_length) {
	if(data_length < SPDY_SYN_STREAM_HEADER_MIN_LENGTH) {
		SPDYDEBUG("Not enough data for parsing the header.");
		return SPDY_ERROR_INSUFFICIENT_DATA;
	}

	// Read the Stream-ID.
	syn_stream->stream_id = ntohl(*((uint32_t*)data)) & 0x7FFFFFFF;
	data += 4;
	// Read the 'Associated-To-Stream-ID'.
	syn_stream->associated_to = ntohl(*((uint32_t*)data)) & 0x7FFFFFFF;
	data += 4;
	// Read the two priority bits.
	syn_stream->priority = (data[0] & 0xC0) >> 6;
	// Skip the unused block.
	data += 2;

	return 0;
}

/**
 * Parse a SYN_STREAM control frame.
 * Parses the header of a SYN_STREAM control frame and extracts the
 * NV block.
 * @param syn_stream - Destination frame.
 * @param data - Data to parse.
 * @param data_length - Length of data.
 * @param zlib_ctx - The zlib context to use.
 * @see spdy_control_frame
 * @see SPDY_SYN_STREAM_MIN_LENGTH
 * @return 0 on success, -1 on failure.
 */
int spdy_syn_stream_parse(spdy_syn_stream *syn_stream, char *data, size_t data_length, spdy_zlib_context *zlib_ctx) {
	int ret;
	if(data_length < SPDY_SYN_STREAM_MIN_LENGTH) {
		SPDYDEBUG("Not enough data for parsing the stream.");
		return SPDY_ERROR_INSUFFICIENT_DATA;
	}

	// Parse the frame header.
	if((ret = spdy_syn_stream_parse_header(
					syn_stream,
					data,
					data_length)) != SPDY_ERROR_NONE) {
		SPDYDEBUG("Failed to parse header.");
		return ret;
	}

	// Skip the (already parsed) header.
	data += SPDY_SYN_STREAM_HEADER_MIN_LENGTH;
	data_length -= SPDY_SYN_STREAM_HEADER_MIN_LENGTH;

	// Inflate NV block.
	char *inflate = NULL;
	size_t inflate_size = 0;
	if((ret = spdy_zlib_inflate(
					zlib_ctx,
					data,
					data_length,
					&inflate,
					&inflate_size)) < 0) {
		SPDYDEBUG("Failed to inflate data.");
		return ret;
	}

	// Allocate space for NV block.
	syn_stream->nv_block = malloc(sizeof(spdy_nv_block));
	if(!syn_stream->nv_block) {
		// Inflate gets allocated in spdy_zlib_inflate.
		free(inflate);
		SPDYDEBUG("Failed to allocate memory for nv_block.");
		return SPDY_ERROR_MALLOC_FAILED;
	}

	// Parse NV block.
	if((ret = spdy_nv_block_parse(
					syn_stream->nv_block,
					inflate,
					inflate_size)) < 0) {
		// Clean up.
		free(inflate);
		free(syn_stream->nv_block);
		SPDYDEBUG("Failed to parse NV block.");
		return ret;
	}

	// Only needed during parsing of NV block.
	free(inflate);

	return 0;
}

