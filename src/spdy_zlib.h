#ifndef SPDY_ZLIB_H_
#define SPDY_ZLIB_H_

#include <stdint.h>
#include <stdlib.h>
#include <zlib.h>
typedef struct {
	z_stream stream;
} spdy_zlib_context;

int spdy_zlib_deflate(char *src, uint32_t length, char **dest, size_t *dest_size);
int spdy_zlib_inflate_init(spdy_zlib_context *ctx);
int spdy_zlib_inflate(spdy_zlib_context *ctx, char *src, uint32_t length, char **dest, size_t *dest_size);

#endif

