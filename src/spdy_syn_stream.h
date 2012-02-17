#ifndef SPDY_SYN_STREAM_H_
#define SPDY_SYN_STREAM_H_ 1

#include "spdy_data.h"
#include "spdy_nv_block.h"
#include "spdy_zlib.h"

#include <stdint.h>
#include <stdlib.h>

struct spindly_phys;

/**
 * Flags for SYN_STREAM frames.
 */
enum SPDY_SYN_STREAM_FLAGS
{
  SPDY_SYN_STREAM_FLAG_FIN = 0x01,      /*!< FLAG_FIN */
  SPDY_SYN_STREAM_FLAG_UNIDIRECTIONAL = 0x02    /*!< FLAG_UNIDIRECTIONAL */
};

/**
 * SYN_STREAM control frame
 */
typedef struct
{
  uint32_t stream_id;           /*!< 31 bit stream id */
  uint32_t associated_to;       /*!< 31 bit assocaited to stream id */
  int priority;                 /*!< 3 bit priority */
  spdy_nv_block nv_block;       /*!< Name/Value block */
} spdy_syn_stream;

int spdy_syn_stream_parse_header(spdy_syn_stream *syn_stream, spdy_data *data);
int spdy_syn_stream_parse(spdy_syn_stream *syn_stream,
                          struct spindly_phys *hash,
                          spdy_data *data,
                          uint32_t frame_length);

void spdy_syn_stream_destroy(spdy_syn_stream *syn_stream);

int spdy_syn_stream_pack(unsigned char *out, size_t bufsize,
                         size_t *outsize, spdy_syn_stream *str);

int spdy_syn_stream_init(spdy_syn_stream *str, uint32_t stream_id,
                         uint32_t associated_to, int prio,
                         spdy_nv_block *nv_block);
#endif
