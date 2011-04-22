#ifndef SPDY_ZLIB_H_
#define SPDY_ZLIB_H_

#include <stdint.h>

int spdy_zlib_deflate(char *src, uint32_t length, char **dest);
int spdy_zlib_inflate(char *src, uint32_t length, char **dest);

#endif

