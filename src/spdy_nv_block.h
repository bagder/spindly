#ifndef SPDY_NV_BLOCK_H_
#define SPDY_NV_BLOCK_H_ 1

#include <stdint.h>
#include <stdlib.h>

#include "spdy_zlib.h"

/**
 * Name/Value Pair
 * Contains the name and the values of a single Name/Value pair.
 */
typedef struct {
	char *name;            /*!< Name of the values */
	uint16_t values_count; /*!< Number of values */
	char *values;          /*!< Values */
} spdy_nv_pair;

/**
 * Name/Value Header Block
 * Structure for holding data from a name/value header like in
 * in SYN_STREAM and SYN_REPLY.
 */
typedef struct {
	uint16_t count;      /*!< Number of Name/Value pairs */
	spdy_nv_pair *pairs; /*!< Array of Name/Value pairs */
} spdy_nv_block;

int spdy_nv_block_parse(
		spdy_nv_block *nv_block,
		char *data,
		size_t data_length);
int spdy_nv_block_inflate_parse(
		spdy_nv_block *nv_block,
		char *data,
		size_t data_length,
		spdy_zlib_context *zlib_ctx);
int spdy_nv_block_pack(
		char **dest,
		size_t *dest_size,
		spdy_nv_block *nv_block);
void spdy_nv_block_destroy(spdy_nv_block *nv_block);

#endif

