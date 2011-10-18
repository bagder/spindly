#ifndef SPDY_CONTROL_FRAME_H_
#define SPDY_CONTROL_FRAME_H_ 1

#include <stdint.h>
#include <stdlib.h>

#include "spdy_data.h"
#include "spdy_zlib.h"

/** Minimum length of a control frame.*/
extern const uint8_t SPDY_CONTROL_FRAME_MIN_LENGTH;

/**
 * SPDY control frame types.
 * List of all SPDY control frame types. See section 2.7 in the draft 2
 * specification.
 */
enum SPDY_CTRL_TYPES {
	SPDY_CTRL_SYN_STREAM=1,   /*!< SYN_STREAM frame. */
	SPDY_CTRL_SYN_REPLY=2,    /*!< SYN_REPLY frame. */
	SPDY_CTRL_RST_STREAM=3,   /*!< RST_STREAM frame. */
	SPDY_CTRL_SETTINGS=4,     /*!< SETTINGS frame. */
	SPDY_CTRL_NOOP=5,         /*!< NOOP frame. */
	SPDY_CTRL_PING=6,         /*!< PING frame.*/
	SPDY_CTRL_GOAWAY=7,       /*!< GOAWAY frame. */
	SPDY_CTRL_HEADERS=8,      /*!< HEADERS frame. */
	SPDY_CTRL_WINDOW_UPDATE=9 /*!< WINDOW_UPDATE frame. */
};

/**
 * Control frame
 * - Contains all data (including the data payload) of a data frame.
 */
typedef struct {
	uint16_t version;   /*!< 15 bit version */
	uint16_t type;      /*!< 16 bit type */
	uint8_t flags;      /*!< 8 bit flags */
	uint32_t length;    /*!< 24 bit length */
	/*@null@*/void *type_obj;     /*!< Frame type object */
} spdy_control_frame;

int spdy_control_frame_parse_header(
		spdy_control_frame *frame,
		char *data,
		size_t data_length);
int spdy_control_frame_pack_header(char **out, spdy_control_frame *frame);
int spdy_control_frame_parse(
		spdy_control_frame *frame,
		spdy_data *data,
		spdy_zlib_context *zlib_ctx);
char *spdy_control_frame_get_type_name(int type);

#endif

