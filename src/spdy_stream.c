#include "spdy_stream.h"
#include "spdy_syn_stream.h"
#include "spdy_syn_reply.h"
#include "spdy_rst_stream.h"
#include "spdy_log.h"
#include "spdy_error.h"

#include <stdlib.h>
#include <string.h>

/**
 * Initialize a spdy_stream to null values.
 * @param stream - Stream to initialize.
 * @param store_received_data - Whether to store received data.
 * @param store_frames - Store frames of the stream.
 * @return Errorcode
 */
int spdy_stream_init(
		spdy_stream *stream,
		_Bool store_received_data,
		_Bool store_frames) {
	memset(&stream, 0, sizeof(stream));

	stream->store_received_data = store_received_data;
	stream->store_frames = store_frames;

	/* The C standard doesn't guarantee that 0 == 0x00000000, so we have to
	 * NULL the pointers explicitely.
	 */
	stream->data_received = NULL;
	stream->data_sent = NULL;
	stream->frames = NULL;

	return SPDY_ERROR_NONE;
}

/**
 * Handle frame on stream.
 * @param stream - Target stream.
 * @param frame - Frame to handle.
 * @return Errorcode
 */
int spdy_stream_handle_frame(spdy_stream *stream, spdy_frame *frame) {
	if(stream->fin_received) {
		SPDYDEBUG("Already received FIN.");
		return SPDY_ERROR_STREAM_FIN;
	} else if(stream->rst_received) {
		SPDYDEBUG("Already received RST.");
		return SPDY_ERROR_STREAM_RST;
	}

	if(!stream->frames) {
		stream->frames = frame;
	} else {
		stream->last_frame->next = frame;
		frame->prev = stream->last_frame;
	}
	stream->last_frame = frame;

	switch(frame->type) {
		case SPDY_DATA_FRAME:
			return spdy_stream_handle_data_frame(stream, frame->frame.data);
			break;
		case SPDY_CONTROL_FRAME:
			return spdy_stream_handle_control_frame(stream, frame->frame.control);
			break;
		default:
			/* Should _never_ happen. */
			return SPDY_ERROR_INVALID_DATA;
	}

	return SPDY_ERROR_NONE;
}

/**
 * Handle data frame on stream.
 * @param stream - Target stream.
 * @param frame - Data frame to handle.
 * @return Errorcode
 */
int spdy_stream_handle_data_frame(
		spdy_stream *stream,
		spdy_data_frame *frame) {

	/* Store received data? */
	if(stream->store_received_data) {
		/* Reallocate buffer for received data */
		char *data_received_new = realloc(
				stream->data_received,
				sizeof(char)*(stream->data_received_length+frame->length));
		if(!data_received_new) {
			SPDYDEBUG("Reallocating data_received failed.");
			return SPDY_ERROR_MALLOC_FAILED;
		}
		stream->data_received = data_received_new;
		/* Copy frame data to end of data_received */
		memcpy(
				stream->data_received+stream->data_received_length,
				frame->data,
				frame->length);
		stream->data_received_length += frame->length;
	}

	/* Check if FIN was received. */
	stream->fin_received = (frame->flags & SPDY_DATA_FLAG_FIN);

	return SPDY_ERROR_NONE;
}

int spdy_stream_handle_control_frame(
		spdy_stream *stream,
		spdy_control_frame *frame) {
	(void)stream;
	/* Handle control frame types */
	switch(frame->type) {
		case SPDY_CTRL_SYN_STREAM:
			break;
		case SPDY_CTRL_SYN_REPLY:
			break;
		case SPDY_CTRL_RST_STREAM:
	/*			spdy_rst_stream rst_stream;
			spdy_rst_stream_parse(
			stream->rst_received = 1;
			stream->status_code = frame->->status_code;*/

			break;
		default:
			SPDYDEBUG("Unhandled control frame type.");
			return SPDY_ERROR_INVALID_DATA;
	}
	return SPDY_ERROR_NONE;
}

