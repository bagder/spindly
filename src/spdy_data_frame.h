#ifndef SPDY_DATA_FRAME_H_
#define SPDY_DATA_FRAME_H_

#include <stdint.h>

/**
 * Flags of data frames.
 */
enum SPDY_DATA_FLAGS {
	SPDY_DATA_FLAG_FIN = 0x01 /*!< FLAG_FIN */
};

/**
 * Data frame
 * Contains all data (including the data payload) of a data frame.
 */
typedef struct {
	uint32_t stream_id;  /*!< 31 bit stream_id */
	uint8_t flags;       /*!< 8 bit flags */
	uint32_t length;     /*!< 24 bit length */
	char *data;          /*!< Frame payload */
} spdy_data_frame;

int spdy_data_frame_parse_header(spdy_data_frame *frame, char *data);
int spdy_data_frame_pack_header(char **out, spdy_data_frame *frame);

#endif

