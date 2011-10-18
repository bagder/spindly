#include "spdy_string.h"

#ifndef HAVE_STRNLEN
size_t strnlen(char *s, size_t maxlen)
{
	size_t i;
	for (i= 0; i < maxlen && *s != '\0'; i++, s++);
	return i;
}
#endif

