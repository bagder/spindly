#include "spdy_nv_block.h"
#include "spdy_log.h"
#include "spdy_error.h"

#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <assert.h>

/**
 * Parse a Name/Value block payload.
 * @param nv_block - Target block.
 * @param data - Data to parse.
 * @param data_length - Length of data.
 * @see spdy_nv_block
 * @todo Replace mallocs with a single one. (Speed up!)
 * @todo Freeing in the loop.
 * @todo Multiple value support.
 * @return 0 on success, -1 on failure.
 */
int spdy_nv_block_parse(
		spdy_nv_block *nv_block,
		char *data,
		size_t data_length) {
	// The bounds of data.
	char *data_max = data + data_length;

	// Data must at least contain the number of NV pairs.
	if(data_length < 2) {
		SPDYDEBUG("Data to small.");
		return SPDY_ERROR_INSUFFICIENT_DATA;
	}

	// Read the 16 bit integer containing the number of name/value pairs.
	nv_block->count = ntohs(*((uint16_t*) data));
	assert(nv_block->count > 0);

	// Allocate memory for Name/Value pairs.
	nv_block->pairs = calloc(nv_block->count, sizeof(spdy_nv_pair));
	// Malloc failed
	if(!nv_block->pairs) {
		SPDYDEBUG("Malloc of pairs failed.");
		return SPDY_ERROR_MALLOC_FAILED;
	}

	// Move forward by two bytes.
	data += 2;

	uint16_t item_length;
	size_t size;
	// Loop through all pairs
	for(int i=0; i < nv_block->count; i++) {
		if(data+2 > data_max) {
			SPDYDEBUG("Data to small.");
			return SPDY_ERROR_INSUFFICIENT_DATA;
		}
		spdy_nv_pair *pair = &nv_block->pairs[i];

		// Read Name
		// Read length of name
		item_length = ntohs(*((uint16_t*) data));
		data += 2;
		// Allocate space for name
		size = (sizeof(char)*item_length)+1;
		if(data+item_length > data_max) {
			SPDYDEBUG("Data to small.");
			return SPDY_ERROR_INSUFFICIENT_DATA;
		}
		pair->name = malloc(size);
		if(!pair->name) {
			SPDYDEBUG("Pair name malloc failed.");
			return SPDY_ERROR_MALLOC_FAILED;
		}
		memcpy(pair->name, data, item_length);
		pair->name[item_length] = '\0';
		data += item_length;

		// Read Values
		// Read length of value
		if(data+2 > data_max) {
			SPDYDEBUG("Data to small.");
			return SPDY_ERROR_INSUFFICIENT_DATA;
		}
		item_length = ntohs(*((uint16_t*) data));
		data += 2;
		// Allocate space for values
		size = (sizeof(char)*item_length)+1;
		if(data+item_length > data_max) {
			SPDYDEBUG("Data to small.");
			return SPDY_ERROR_INSUFFICIENT_DATA;
		}
		pair->values = malloc(size);
		if(!pair->name) {
			SPDYDEBUG("Pair value malloc failed.");
			return SPDY_ERROR_MALLOC_FAILED;
		}
		memcpy(pair->values, data, item_length);
		pair->values[item_length] = '\0';
		data += item_length;
	}

	return 0;
}

/**
 * Pack a Name/Value block into a payload for transmitting.
 * @param dest - Destination buffer.
 * @param dest_size - Pointer for storing the size of the destination buffer.
 * @param nv_block - NV block to pack.
 * @see spdy_nv_block
 * @see spdy_nv_block_parse
 * @todo Multiple value support.
 * @return 0 on succes, -1 on failure.
 */
int spdy_nv_block_pack(
		char **dest,
		size_t *dest_size,
		spdy_nv_block *nv_block) {
	*dest = NULL;

	// Two bytes for the number of pairs.
	*dest_size = 2;
	// Calculate the size needed for the ouput buffer.
	for(int i=0; i < nv_block->count; i++) {
		// Two bytes (length) + stringlength
		*dest_size += 2+strlen(nv_block->pairs[i].name);
		*dest_size += 2+strlen(nv_block->pairs[i].values);
	}

	// Allocate memory for dest
	*dest = malloc(sizeof(char)*(*dest_size));
	if(!*dest) {
		SPDYDEBUG("Memoy allocation failed.");
		return SPDY_ERROR_MALLOC_FAILED;
	}
	char *cursor = *dest;

	*((uint16_t*)cursor) = htons(nv_block->count);
	cursor += 2;
	uint16_t length;
	for(int i=0; i < nv_block->count; i++) {
		length = strlen(nv_block->pairs[i].name);
		*((uint16_t*)cursor) = htons(length);
		memcpy(
				cursor+2,
				nv_block->pairs[i].name,
				length);
		cursor += length+2;
		length = strlen(nv_block->pairs[i].values);
		*((uint16_t*)cursor) = htons(length);
		memcpy(
				cursor+2,
				nv_block->pairs[i].values,
				length);
		cursor += length+2;
	}
	return 0;
}

/**
 * Frees all the content of an nv_block and the nv_block itself.
 * @param nv_block - NV block to destroy.
 * @todo How to test this?
 */
void spdy_nv_block_destroy(spdy_nv_block *nv_block) {
	if(nv_block) {
		if(nv_block->pairs) {
			for(int i=0; i < nv_block->count; i++) {
				free(nv_block->pairs[i].name);
				free(nv_block->pairs[i].values);
			}
			free(nv_block->pairs);
		}
		free(nv_block);
	}
}

