#include "spdy_stream.h"
#include "spdy_syn_stream.h"
#include "spdy_syn_reply.h"
#include "spdy_rst_stream.h"
#include "spdy_log.h"
#include "spdy_error.h"

#include <stdlib.h>
#include <string.h>

int spdy_stream_init(
		spdy_stream *stream,
		_Bool store_received_data,
		_Bool store_frames) {
	stream->store_received_data = store_received_data;
	stream->store_frames = store_frames;
	stream->stream_id = 0;
	stream->associated_to = 0;
	stream->unidirectional = 0;
	stream->data_received_length = 0;
	stream->data_sent_length = 0;
	stream->data_received = NULL;
	stream->data_sent = NULL;
	stream->fin_received = 0;
	stream->fin_sent = 0;
	stream->rst_received = 0;
	stream->rst_sent = 0;
	stream->frames_count = 0;
	stream->frames = NULL;

	return 0;
}

int spdy_stream_handle_frame(spdy_stream *stream, spdy_frame *frame) {
	if(stream->fin_received) {
		SPDYDEBUG("Already received FIN.");
		return SPDY_ERROR_STREAM_FIN;
	} else if(stream->rst_received) {
		SPDYDEBUG("Already received RST.");
		return SPDY_ERROR_STREAM_RST;
	}
	if(stream->store_frames) {
		// Reallocate space for frames pointer.
		spdy_frame **frames_new = realloc(
				stream->frames,
				sizeof(spdy_stream*)*(stream->frames_count+1));

		// If allocation failed, we return but keep the frames pointer untouched.
		if(!frames_new) {
			SPDYDEBUG("Reallocating space for frames pointer failed.");
			return SPDY_ERROR_MALLOC_FAILED;
		}
		stream->frames = frames_new;
		stream->frames[stream->frames_count] = frame;
	}

	// Keep the number of frames used in the stream.
	stream->frames_count++;

	switch(frame->type) {
		case SPDY_DATA_FRAME:
			return spdy_stream_handle_data_frame(stream, frame->frame);
			break;
		case SPDY_CONTROL_FRAME:
			break;
		default:
			// Should _never_ happen.
			return SPDY_ERROR_INVALID_DATA;
	}

	return 0;
}

int spdy_stream_handle_data_frame(
		spdy_stream *stream,
		spdy_data_frame *frame) {

	// Store received data?
	if(stream->store_received_data) {
		// Reallocate buffer for received data
		char *data_received_new = realloc(
				stream->data_received,
				sizeof(char)*(stream->data_received_length+frame->length));
		if(!data_received_new) {
			SPDYDEBUG("Reallocating data_received failed.");
			return SPDY_ERROR_MALLOC_FAILED;
		}
		stream->data_received = data_received_new;
		// Copy frame data to end of data_received
		memcpy(
				stream->data_received+stream->data_received_length,
				frame->data,
				frame->length);
		stream->data_received_length += frame->length;
	}

	// Check if FIN was received.
	stream->fin_received = (frame->flags & SPDY_DATA_FLAG_FIN);

	return 0;
}

int spdy_stream_handle_control_frame(
		spdy_stream *stream,
		spdy_control_frame *frame) {
	int ret;
	(void)stream;
	// Handle control frame types
	switch(frame->type) {
		case SPDY_CTRL_SYN_STREAM:
			frame->type_obj = malloc(sizeof(spdy_syn_stream));
			if(!frame->type_obj) {
				SPDYDEBUG("Failed to allocate space for SYN_STREAM.");
				return SPDY_ERROR_MALLOC_FAILED;
			}
	/*			ret = spdy_syn_stream_parse(
					frame->type_obj,
					frame->length,
					frame->data,
					frame->length);*/
			(void)ret;
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
	return 0;
}

