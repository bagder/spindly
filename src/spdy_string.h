#ifndef SPDY_STRING_H_
#define SPDY_STRING_H_

#include <stdlib.h>
#include <string.h>

#ifndef HAVE_STRNLEN
size_t strnlen(char *s, size_t maxlen);
#endif

#endif
