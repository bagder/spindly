#ifndef SPDY_NV_BLOCK_H_
#define SPDY_NV_BLOCK_H_ 1

#include <stdint.h>
#include <stdlib.h>

#include "spdy_zlib.h"

typedef struct spdy_nv_pair spdy_nv_pair;
typedef struct spdy_nv_block spdy_nv_block;

/**
 * Name/Value Pair
 * Contains the name and the values of a single Name/Value pair.
 */
struct spdy_nv_pair
{
  char *name;                   /*!< Name of the values */
  uint16_t values_count;        /*!< Number of values */
  char *values;                 /*!< Values */
};

/**
 * Name/Value Header Block
 * Structure for holding data from a name/value header like in
 * in SYN_STREAM and SYN_REPLY.
 */
struct spdy_nv_block
{
  bool has_count;          /*!< Determines if the count has been parsed. */
  int count;               /*!< Number of Name/Value pairs */
  int pairs_parsed;        /*!< Number of pairs that have been parsed. */
  spdy_nv_pair *pairs;     /*!< Array of Name/Value pairs */
};

/* NV pair functions */
int spdy_nv_pair_create(spdy_nv_pair **pair);
int spdy_nv_pair_init(spdy_nv_pair *pair);
int spdy_nv_pair_destroy(spdy_nv_pair **pair);

/* NV block functions */
int spdy_nv_block_init(spdy_nv_block *block);

int spdy_nv_block_parse(spdy_nv_block *nv_block,
                        unsigned char *data, size_t data_length);
int spdy_nv_block_inflate_parse(spdy_nv_block *nv_block,
                                unsigned char *data,
                                size_t data_length,
                                spdy_zlib_context *zlib_ctx);
int spdy_nv_block_pack(char **dest,
                       size_t *dest_size, spdy_nv_block *nv_block);
void spdy_nv_block_destroy(spdy_nv_block *nv_block);

#endif
