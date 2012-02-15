#ifndef SPDY_DATA_H_
#define SPDY_DATA_H_

#include <stdlib.h>

/** Contains byte-data that is given to functions.
 * spdy_data is used to keep track of data that comes from the transport
 * layer and is handled by libspdy. It keeps track of position and length
 * of the data and contains the amount of data that is still needed after a
 * function returned SPDY_ERROR_INSUFFICIENT_DATA.
 * @see SPDY_ERROR_INSUFFICIENT_DATA
 */

typedef struct
{
  unsigned char *data;
  unsigned char *data_end;
  unsigned char *cursor;
  size_t needed;
} spdy_data;

spdy_data *spdy_data_use(spdy_data *data, unsigned char *data_in,
                         size_t length);

void spdy_data_copy(spdy_data *data_out, spdy_data *data_in);

#endif
