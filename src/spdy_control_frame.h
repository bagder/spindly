#ifndef SPDY_CONTROL_FRAME_H_
#define SPDY_CONTROL_FRAME_H_ 1

#include <stdint.h>
#include <stdlib.h>

#include "spdy_data.h"
#include "spdy_zlib.h"
#include "spdy_syn_stream.h"
#include "spdy_syn_reply.h"
#include "spdy_rst_stream.h"
#include "spdy_headers.h"

struct spindly_phys;

/**
 * SPDY control frame types.
 * List of all SPDY control frame types. See section 2.7 in the draft 2
 * specification.
 */
enum SPDY_CTRL_TYPES
{
  SPDY_CTRL_SYN_STREAM = 1,     /*!< SYN_STREAM frame. */
  SPDY_CTRL_SYN_REPLY = 2,      /*!< SYN_REPLY frame. */
  SPDY_CTRL_RST_STREAM = 3,     /*!< RST_STREAM frame. */
  SPDY_CTRL_SETTINGS = 4,       /*!< SETTINGS frame. */
  SPDY_CTRL_NOOP = 5,           /*!< NOOP frame. */
  SPDY_CTRL_PING = 6,           /*!< PING frame. */
  SPDY_CTRL_GOAWAY = 7,         /*!< GOAWAY frame. */
  SPDY_CTRL_HEADERS = 8,        /*!< HEADERS frame. */
  SPDY_CTRL_WINDOW_UPDATE = 9   /*!< WINDOW_UPDATE frame. */
};

/**
 * Control frame
 * - Contains all data (including the data payload) of a data frame.
 */
typedef struct
{
  bool _header_parsed;         /*!< Determines if the header has been parsed. */
  uint16_t version;             /*!< 15 bit version */
  uint16_t type;                /*!< 16 bit type */
  uint8_t flags;                /*!< 8 bit flags */
  uint32_t length;              /*!< 24 bit length */
  union
  {
    spdy_syn_stream syn_stream;
    spdy_syn_reply syn_reply;
    spdy_rst_stream rst_stream;
    spdy_headers headers;
  } obj;
} spdy_control_frame;

int spdy_control_frame_init(spdy_control_frame *frame);

int spdy_control_frame_parse_header(spdy_control_frame *frame,
                                    spdy_data *data);
int spdy_control_frame_pack_header(unsigned char *outp, size_t bufsize,
                                   size_t *outsize, spdy_control_frame *frame);
int spdy_control_frame_pack(unsigned char *outp, size_t bufsize,
                            size_t *outsize, spdy_control_frame *frame);
int spdy_control_frame_parse(spdy_control_frame *frame,
                             struct spindly_phys *phys,
                             spdy_data *data);
char *spdy_control_frame_get_type_name(int type);

void spdy_control_frame_destroy(spdy_control_frame *frame);

int spdy_control_mk_syn_stream(spdy_control_frame *frame,
                               uint32_t stream_id,
                               uint32_t associated_to,
                               int prio,
                               spdy_nv_block *nv_block);

int spdy_control_mk_syn_reply(spdy_control_frame *frame,
                              uint32_t stream_id,
                              spdy_nv_block *nv_block);

int spdy_control_mk_rst_stream(spdy_control_frame *frame,
                               uint32_t stream_id,
                               uint32_t status);

#endif
