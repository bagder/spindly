#ifndef SPDY_ZLIB_H_
#define SPDY_ZLIB_H_

#include <stdint.h>
#include <stdlib.h>
#include <zlib.h>

/**
 * Context for zlib deflating and inflating.
 * Allows to use the same zlib stream on multiple frames. (Needed
 * for inflating multiple compressed headers on a SPDY stream.)
 */
typedef struct
{
  z_stream stream;              /*!< zlib stream */
} spdy_zlib_context;

int spdy_zlib_deflate(char *src, uint32_t length, size_t *data_used,
                      char **dest, size_t *dest_size);
int spdy_zlib_inflate_init(spdy_zlib_context *ctx);
void spdy_zlib_inflate_end(spdy_zlib_context *ctx);
int spdy_zlib_inflate(spdy_zlib_context *ctx,
                      char *src,
                      uint32_t length, char **dest, size_t *dest_size);

#endif
