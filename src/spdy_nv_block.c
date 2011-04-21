#include "spdy_nv_block.h"

#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <assert.h>

/**
 * Parse a Name/Value block payload.
 * @param nv_block Target block.
 * @param data Data to parse.
 * @see spdy_nv_block
 * @return 0 on success, -1 on failure.
 */
int spdy_nv_block_parse(spdy_nv_block *nv_block, char *data) {
	// Read the 16 bit integer containing the number of name/value pairs.
	nv_block->count = ntohs(*((uint16_t*) data));
	assert(nv_block->count > 0);

	// Allocate memory for Name/Value pairs.
	nv_block->pairs = calloc(nv_block->count, sizeof(spdy_nv_pair));
	// Malloc failed
	if(!nv_block->pairs) {
		return -1;
	}

	// Move forward by two bytes.
	data += 2;

	uint16_t item_length;
	size_t size;
	// TODO: It should be possible to replace all the mallocs with a single one.
	// Loop through all pairs
	for(int i=0; i < nv_block->count; i++) {
		spdy_nv_pair *pair = &nv_block->pairs[i];

		// Read Name
		// Read length of name
		item_length = ntohs(*((uint16_t*) data));
		data += 2;
		// Allocate space for name
		size = (sizeof(char)*item_length)+1;
		pair->name = malloc(size);
		if(!pair->name) {
			// TODO: Cleanup!
			return -1;
		}
		memcpy(pair->name, data, item_length);
		pair->name[item_length] = '\0';
		data += item_length;

		// Read Values
		item_length = ntohs(*((uint16_t*) data));
		data += 2;
		// Allocate space for values
		size = (sizeof(char)*item_length)+1;
		pair->values = malloc(size);
		if(!pair->name) {
			// TODO: Cleanup!
			return -1;
		}
		memcpy(pair->values, data, item_length);
		pair->values[item_length] = '\0';
		data += item_length;
	}

	return 0;
}

