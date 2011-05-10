#include "spdy_data.h"

#include <string.h>

/**
 * Simple function for using an already allocated spdy_data easily,
 * for instance for a function call.
 * @param data - Target spdy_data object.
 * @param data_in - Data in the object.
 * @param length - Length of data_in.
 * @return spdy_data - Filled with the supplied data. Can't fail.
 */
spdy_data *spdy_data_use(
		spdy_data *data,
		char *data_in,
		size_t length) {
	data->data_in = data_in;
	data->data = data_in;
	data->length_in = length;
	data->length = length;
	data->used = 0;
	data->needed = 0;
	return data;
}

/**
 * Copy the data from one spdy_data structure into another.
 * @param data_out - Target spdy_data object.
 * @param data_in  - Source spdy_data object.
 * @return Void. Can't fail.
 */
void spdy_data_copy(
		spdy_data *data_out,
		spdy_data *data_in) {
	memcpy(data_out, data_in, sizeof(spdy_data));
	return;
}

