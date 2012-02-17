#include "spdy_setup.h"         /* MUST be the first header to include */
#include "spdy_nv_block.h"
#include "spdy_log.h"
#include "spdy_error.h"
#include "spdy_bytes.h"

#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <assert.h>

/**
 * Allocate and initialize an NV pair.
 *
 * @param pair - Pair to create
 * @see spdy_nv_pair
 * @return Errorcode
 */
int spdy_nv_pair_create(spdy_nv_pair **pair)
{
  *pair = malloc(sizeof(spdy_nv_pair));
  if(!(*pair)) {
    return SPDY_ERROR_MALLOC_FAILED;
  }

  return spdy_nv_pair_init(*pair);
}

/**
 * Initialize an already allocated NV pair with sane default values.
 *
 * @param pair - Pair to initialize
 * @see spdy_nv_pair
 * @return Errorcode
 */
int spdy_nv_pair_init(spdy_nv_pair *pair)
{
  pair->name = NULL;
  pair->values_count = 0;
  pair->values = NULL;
  return SPDY_ERROR_NONE;
}

/**
 * Destroy and free an NV pair created by spdy_nv_pair_create.
 *
 * @param pair - Pair to destroy
 * @see spdy_nv_pair
 * @return Errorcode
 */
int spdy_nv_pair_destroy(spdy_nv_pair **pair)
{
  /* Free name & values */
  if((*pair)->name) {
    free((*pair)->name);
  }
  if((*pair)->values) {
    free((*pair)->values);
  }

  free(*pair);
  *pair = NULL;
  return SPDY_ERROR_NONE;
}


/**
 * Initialize an NV block.
 *
 * @param block - NV block to initialize
 * @see spdy_nv_block
 * @return Errorcode
 */
int spdy_nv_block_init(spdy_nv_block *block)
{
  block->has_count = 0;
  block->count = 0;
  block->pairs_parsed = 0;
  block->pairs = NULL;
  return SPDY_ERROR_NONE;
}

/**
 * Parse a Name/Value block payload.
 * @param block - Target block.
 * @param data - Data to parse.
 * @param data_length - Length of data.
 * @see spdy_nv_block
 * @todo Replace mallocs with a single one. (Speed up!)
 * @todo Freeing in the loop.
 * @todo Multiple value support.
 * @return Errorcode
 */
int spdy_nv_block_parse(spdy_nv_block *block, unsigned char *data,
                        size_t data_length)
{
  /* The bounds of data. */
  unsigned char *data_max = data + data_length;

  /* For the for-loop: */

  /* Parsing block pair count */
  if(!block->has_count) {
    /* Data must at least contain the number of NV pairs. */
    if(data_length < 2) {
      SPDYDEBUG("Data to small.");
      return SPDY_ERROR_INSUFFICIENT_DATA;
    }

    /* Read the 16 bit integer containing the number of name/value pairs. */
    block->count = BE_LOAD_16(data);
    block->has_count = 1;
    block->pairs_parsed = 0;
    /* Move forward by two bytes. */
    data += 2;
    if(block->count == 0) {
      block->pairs = NULL;
      return SPDY_ERROR_NONE;
    }

    /* Allocate memory for Name/Value pairs. */
    block->pairs = calloc(block->count, sizeof(spdy_nv_pair));
    /* Malloc failed */
    if(!block->pairs) {
      SPDYDEBUG("Malloc of pairs failed.");
      return SPDY_ERROR_MALLOC_FAILED;
    }
  }
  /* End of parsing block pair count */


  /* Loop through all pairs */
  for(; block->pairs_parsed < block->count; block->pairs_parsed++) {
    size_t size;
    spdy_nv_pair *pair;
    uint16_t tmp_item_length;

    if(data + 2 > data_max) {
      SPDYDEBUG("Data to small.");
      return SPDY_ERROR_INSUFFICIENT_DATA;
    }

    pair = &block->pairs[block->pairs_parsed];

    /* Read Name */
    /* Read length of name */
    tmp_item_length = BE_LOAD_16(data);
    data += 2;
    /* Allocate space for name */
    size = tmp_item_length + 1;
    if(data + tmp_item_length > data_max) {
      SPDYDEBUG("Data to small.");
      return SPDY_ERROR_INSUFFICIENT_DATA;
    }

    pair->name = malloc(size);
    if(!pair->name) {
      SPDYDEBUG("Pair name malloc failed.");
      return SPDY_ERROR_MALLOC_FAILED;
    }
    memcpy(pair->name, data, tmp_item_length);
    pair->name[tmp_item_length] = '\0';
    data += tmp_item_length;
    /* End of read name */

    /* Read Values */
    /* TODO: Support multiple values. */
    /* Read length of value */
    if(data + 2 > data_max) {
      SPDYDEBUG("Data to small.");
      free(pair->name);
      return SPDY_ERROR_INSUFFICIENT_DATA;
    }
    tmp_item_length = BE_LOAD_16(data);
    data += 2;
    /* Allocate space for values */
    size = tmp_item_length + 1;
    if(data + tmp_item_length > data_max) {
      SPDYDEBUG("Insufficient data for block parse.");
      free(pair->name);
      return SPDY_ERROR_INSUFFICIENT_DATA;
    }
    pair->values = malloc(size);
    if(!pair->values) {
      SPDYDEBUG("Pair value malloc failed.");
      free(pair->name);
      return SPDY_ERROR_MALLOC_FAILED;
    }
    memcpy(pair->values, data, tmp_item_length);
    pair->values[tmp_item_length] = '\0';
    data += tmp_item_length;
    /* End of read value */
  }

  return SPDY_ERROR_NONE;
}

/* TODO: Test & documentation. Do not use yet! */
int spdy_nv_block_inflate_parse(spdy_nv_block *nv_block,
                                unsigned char *data,
                                size_t data_length,
                                spdy_zlib_context *zlib_ctx)
{
  int ret;

  /* Inflate NV block. */
  char *inflate = NULL;
  size_t inflate_size = 0;

  if((ret = spdy_zlib_inflate(zlib_ctx,
                              (char *)data,
                              data_length,
                              &inflate, &inflate_size)) != SPDY_ERROR_NONE) {
    SPDYDEBUG("Failed to inflate data.");
    return ret;
  }

  /* Parse NV block. */
  ret = spdy_nv_block_parse(nv_block, (unsigned char *)inflate, inflate_size);

  free(inflate);

  if(ret != SPDY_ERROR_NONE) {
    /* Clean up. */
    SPDYDEBUG("Failed to parse NV block.");
    return ret;
  }

  return SPDY_ERROR_NONE;
}

/**
 * Pack a Name/Value block into a payload for transmitting.
 *
 * Note that this function returns an allocated string in 'dest'.
 *
 * @param dest - Destination buffer.
 * @param dest_size - Pointer for storing the size of the destination buffer.
 * @param nv_block - NV block to pack.
 * @see spdy_nv_block
 * @see spdy_nv_block_parse
 * @todo Multiple value support.
 * @return 0 on success, -1 on failure.
 */
int spdy_nv_block_pack(char **dest, size_t *dest_size, spdy_nv_block *nv_block)
{
  int i;
  char *cursor;

  *dest = NULL;
  /* Two bytes for the number of pairs. */
  *dest_size = 2;
  /* Calculate the size needed for the ouput buffer. */
  for(i = 0; i < nv_block->count; i++) {
    /* Two bytes (length) + stringlength */
    *dest_size += 2 + strlen(nv_block->pairs[i].name);
    *dest_size += 2 + strlen(nv_block->pairs[i].values);
  }

  /* Allocate memory for dest */
  *dest = malloc(*dest_size);
  if(!*dest) {
    SPDYDEBUG("Memoy allocation failed.");
    return SPDY_ERROR_MALLOC_FAILED;
  }
  /* Cursor always points to the location in dest where we're working. */
  cursor = *dest;

  /* 2-bytes for the number of NV-pairs that follow. */
  BE_STORE_16(cursor, nv_block->count);
  cursor += 2;

  for(i = 0; i < nv_block->count; i++) {
    uint16_t length;

    /* Read the length and copy the data of each name/value into the
     * destination buffer. */
    length = strlen(nv_block->pairs[i].name);
    BE_STORE_16(cursor, length);
    memcpy(cursor + 2, nv_block->pairs[i].name, length);
    cursor += length + 2;
    length = strlen(nv_block->pairs[i].values);
    BE_STORE_16(cursor, length);
    memcpy(cursor + 2, nv_block->pairs[i].values, length);
    cursor += length + 2;
  }
  return SPDY_ERROR_NONE;
}

/**
 * Frees all the content of an nv_block and the nv_block itself.
 * @param nv_block - NV block to destroy.
 * @todo How to test this?
 */
void spdy_nv_block_destroy(spdy_nv_block *nv_block)
{
  if(nv_block) {
    if(nv_block->pairs) {
      int i;
      for(i = 0; i < nv_block->pairs_parsed; i++) {
        free(nv_block->pairs[i].name);
        free(nv_block->pairs[i].values);
      }
      free(nv_block->pairs);
    }
  }
}
