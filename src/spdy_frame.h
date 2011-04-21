#ifndef SPDY_FRAME_H_
#define SPDY_FRAME_H_ 1

#include <stdint.h>

/**
 * Frame type enum
 * Contains the identifiers for the different frame types.
 */
enum SPDY_FRAME_TYPE {
	SPDY_DATA_FRAME=0,   /*!< SPDY Data Frame */
	SPDY_CONTROL_FRAME=1 /*!< SPDY Control Frame */
};

typedef struct {
	enum SPDY_FRAME_TYPE type; /*!< Type of the frame */
	void *frame;               /*!< Frame */
} spdy_frame;

/**
 * Control frame
 * Contains all data (including the data payload) of a data frame.
 */
typedef struct {
	uint16_t version;   /*!< 15 bit version */
	uint16_t type;      /*!< 16 bit type */
	uint8_t flags;      /*!< 8 bit flags */
	uint32_t length;    /*!< 24 bit length */
	unsigned char *data;/*!< Frame payload */
} spdy_control_frame;

/**
 * Data frame
 * Contains all data (including the data payload) of a control frame.
 */
typedef struct {
	uint32_t stream_id; /*!< 31 bit stream id */
	uint8_t  flags;     /*!< 8 bit flags */
	uint32_t length;    /*!< 24 bit length */
	unsigned char *data;/*!< Frame payload */
} spdy_data_frame;

int spdy_frame_parse_header(spdy_frame *frame, char *data);

#endif
