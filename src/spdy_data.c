#include "spdy_setup.h"         /* MUST be the first header to include */
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
spdy_data *spdy_data_use(spdy_data *data, unsigned char *data_in,
                         size_t length)
{
  data->data = data_in;
  data->data_end = data_in + length;
  data->cursor = data->data;
  data->needed = 0;
  return data;
}

/**
 * Copy the data from one spdy_data structure into another.
 * @param data_out - Target spdy_data object.
 * @param data_in  - Source spdy_data object.
 * @todo Check if this is really portable.
 * @return Void. Can't fail.
 */
void spdy_data_copy(spdy_data *data_out, spdy_data *data_in)
{
  memcpy(data_out, data_in, sizeof(spdy_data));
  return;
}
