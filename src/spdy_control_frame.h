#ifndef SPDY_CONTROL_FRAME_H_
#define SPDY_CONTROL_FRAME_H_ 1

#include <stdint.h>

/**
 * Control frame
 * Contains all data (including the data payload) of a data frame.
 */
typedef struct {
	uint16_t version;   /*!< 15 bit version */
	uint16_t type;      /*!< 16 bit type */
	uint8_t flags;      /*!< 8 bit flags */
	uint32_t length;    /*!< 24 bit length */
	char *data;/*!< Frame payload */
} spdy_control_frame;

int spdy_control_frame_parse_header(spdy_control_frame *frame, char *data);
int spdy_control_frame_pack_header(char **out, spdy_control_frame *frame);

#endif

