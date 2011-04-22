#ifndef SPDY_CONTROL_FRAME_H_
#define SPDY_CONTROL_FRAME_H_ 1

#include <stdint.h>

/**
 * SPDY control frame types.
 * List of all SPDY control frame types.
 */
enum SPDY_CTRL_TYPES {
	SPDY_CTRL_SYN_STREAM=1,   /*!< SYN_STREAM */
	SPDY_CTRL_SYN_REPLY=2,    /*!< SYN_REPLY */
	SPDY_CTRL_RST_STREAM=3,   /*!< RST_STREAM */
	SPDY_CTRL_SETTINGS=4,     /*!< SETTINGS */
	SPDY_CTRL_NOOP=5,         /*!< NOOP */
	SPDY_CTRL_PING=6,         /*!< PING */
	SPDY_CTRL_GOAWAY=7,       /*!< GOAWAY */
	SPDY_CTRL_HEADERS=8,      /*!< HEADERS */
	SPDY_CTRL_WINDOW_UPDATE=9 /*!< WINDOW_UPDATE */
};

/**
 * Control frame
 * Contains all data (including the data payload) of a data frame.
 */
typedef struct {
	uint16_t version;   /*!< 15 bit version */
	uint16_t type;      /*!< 16 bit type */
	uint8_t flags;      /*!< 8 bit flags */
	uint32_t length;    /*!< 24 bit length */
	char *data;         /*!< Frame payload */
} spdy_control_frame;

int spdy_control_frame_parse_header(spdy_control_frame *frame, char *data);
int spdy_control_frame_pack_header(char **out, spdy_control_frame *frame);
char *spdy_control_frame_get_type_name(int type);

#endif

