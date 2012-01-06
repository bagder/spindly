#ifndef SPDY_LOG_H_
#define SPDY_LOG_H_

#include <stdio.h>


#ifdef DEBUG
/**
 * fprintf ogging macro for SPDY.
 * Can be used with a format string, logs file line and function if
 * compiled with -DDEBUG.
 */
#define SPDYDEBUG(msg) \
	fprintf(stderr, "%s:%d:%s: ", __FILE__, __LINE__, __func__); \
	fprintf(stderr, msg); \
	fprintf(stderr, "\n");
#else
#define SPDYDEBUG(...)
#endif

#endif

