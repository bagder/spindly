#ifndef SPDY_DATA_H_
#define SPDY_DATA_H_

#include <stdlib.h>

typedef struct {
	char *data_in;    /*!< Beginning of data block. */
	char *data;       /*!< Datalocation. */
	size_t length_in; /*!< Length of data_in. */
	size_t length;    /*!< Length of data. */
	size_t used;      /*!< Amount of data that is used. */
	size_t needed;    /*!< Only use after SPDY_ERROR_INSUFFICIENT_DATA.
	                       Contains the amount of data that is needed for a
	                       function to continue. */
} spdy_data;

spdy_data *spdy_data_use(
		spdy_data *data,
		char *data_in,
		size_t length);

void spdy_data_copy(
		spdy_data *data_out,
		spdy_data *data_in);

#endif

