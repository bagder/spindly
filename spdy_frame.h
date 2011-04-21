#ifndef SPDY_FRAME_H_
#define SPDY_FRAME_H_

#include <stdint.h>

enum SPDY_FRAME_TYPE {
	SPDY_DATA_FRAME=0,   /* SPDY Data Frame */
	SPDY_CONTROL_FRAME=1 /* SPDY Control Frame */
}

typedef struct {
	uint16_t version;   /* 15 bit version */
	uint16_t type;      /* 16 bit type */
	uint8_t flags;      /* 8 bit flags */
	uint32_t length;    /* 24 bit length */
	unsigned char *data;/* Frame payload */
} spdy_control_frame;

typedef struct {
	uint32_t stream_id; /* 31 bit stream id */
	uint8_t  flags;     /* 8 bit flags */
	uint32_t length;    /* 24 bit length */
	unsigned char *data;/* Frame payload */
} spdy_data_frame;

#endif
