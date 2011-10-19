#ifndef SPDY_STREAM_H_
#define SPDY_STREAM_H_ 1

#include "spdy_frame.h"
#include "spdy_data_frame.h"
#include "spdy_control_frame.h"


/**
 * SPDY Stream
 * This structure keeps the whole state of a SPDY stream.
 */
typedef struct {
	/** Configuration: **/
	_Bool store_received_data;
	_Bool store_frames;
	/** Stream data: **/
	uint32_t stream_id;
	uint32_t associated_to;
	_Bool unidirectional;
	uint32_t data_received_length;
	uint32_t data_sent_length;
	char *data_received;
	char *data_sent;
	_Bool fin_received;
	_Bool fin_sent;
	_Bool rst_received;
	_Bool rst_sent;
	uint32_t status_code;
	uint32_t frames_count;
	spdy_frame *frames;
	spdy_frame *last_frame;
} spdy_stream;

int spdy_stream_init(
		spdy_stream *stream,
		_Bool store_received_data,
		_Bool store_frames);
int spdy_stream_handle_frame(spdy_stream *stream, spdy_frame *frame);
int spdy_stream_handle_data_frame(
		spdy_stream *stream,
		spdy_data_frame *frame);
int spdy_stream_handle_control_frame(
		spdy_stream *stream,
		spdy_control_frame *frame);
#endif

