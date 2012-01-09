#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.64)
AC_INIT([spindly],[-],[spdy-library@cool.haxx.se])
AC_COPYRIGHT([Copyright (c) .. fill in])

AC_CONFIG_SRCDIR([src/spdy_data.c])
AM_CONFIG_HEADER([src/spdy_config.h])
AC_CONFIG_MACRO_DIR([m4])
AM_MAINTAINER_MODE

# Checks for programs.
AC_PROG_CC

dnl libtool setup
AC_PROG_LIBTOOL
AM_INIT_AUTOMAKE

dnl figure out the spindly version
SPINDLYVER=`$SED -ne 's/^#define SPINDLY_VERSION *"\(.*\)"/\1/p' ${srcdir}/include/spindly.h`
AC_MSG_CHECKING([spindly version])
AC_MSG_RESULT($SPINDLYVER)
AC_SUBST(SPINDLYVER)

# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([fcntl.h netinet/in.h stdint.h stdlib.h string.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_SIZE_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT8_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_CHECK_FUNCS([memset])

AC_CONFIG_FILES([Makefile src/Makefile tests/Makefile doc/Makefile \
                 include/Makefile bin/Makefile])
AC_OUTPUT
