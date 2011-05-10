#ifndef SPDY_FRAME_H_
#define SPDY_FRAME_H_ 1

#include "spdy_data.h"
#include "spdy_control_frame.h"

#include <stdint.h>
#include <stdlib.h>

/**
 * Frame type enum
 * Contains the identifiers for the different frame types.
 */
enum SPDY_FRAME_TYPE {
	SPDY_DATA_FRAME=0,   /*!< SPDY Data Frame */
	SPDY_CONTROL_FRAME=1 /*!< SPDY Control Frame */
};

/**
 * Frame
 * Indicates the type of the frame and keeps a pointer to a
 * control or data frame.
 */
typedef struct {
	enum SPDY_FRAME_TYPE type; /*!< Type of the frame */
	void *frame;               /*!< Frame */
} spdy_frame;

int spdy_frame_parse_header(
		spdy_frame *frame,
		char *data,
		size_t data_length);
int spdy_frame_parse(
		spdy_frame *frame,
		spdy_data *data,
		spdy_zlib_context *zlib_ctx);

#endif
