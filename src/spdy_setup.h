#ifndef SPDY_SETUP_H
#define SPDY_SETUP_H

#ifdef HAVE_CONFIG_H
#include "spdy_config.h"
#endif

#ifdef HAVE_STDBOOL_H
#include <stdbool.h>
#endif

/*
 * 'bool' exists on platforms with <stdbool.h>, i.e. C99 platforms.
 * On non-C99 platforms there's no bool, so define an enum for that.
 * On C99 platforms 'false' and 'true' also exist. Enum uses a
 * global namespace though, so use bool_false and bool_true.
 */

#ifndef HAVE_BOOL_T
typedef enum {
  bool_false = 0,
  bool_true  = 1
} bool;

/*
 * Use a define to let 'true' and 'false' use those enums.
 */
#  define false bool_false
#  define true  bool_true
#  define HAVE_BOOL_T
#endif

#endif /* SPDY_SETUP_H */
