AUTOMAKE_OPTIONS = foreign nostdinc

SPINDLY_SOURCES = spindly_phys.c spindly_stream.c list.c list.h	\
  spindly_phys.h spindly_stream.h hash.c hash.h

libspdy_la_SOURCES = spdy_control_frame.c spdy_data.c spdy_data_frame.c	      \
  spdy_frame.c spdy_headers.c spdy_nv_block.c spdy_rst_stream.c \
  spdy_syn_reply.c spdy_syn_stream.c spdy_zlib.c spdy_setup.h		      \
  $(SPINDLY_SOURCES) spdy_control_frame.h spdy_headers.h spdy_syn_reply.h     \
  spdy_data_frame.h spdy_nv_block.h spdy_syn_stream.h spdy_data.h	      \
  spdy_rst_stream.h spdy_zlib.h spdy_error.h spdy_frame.h	      \
  spdy_string.h



libspdy_la_CFLAGS = -I$(top_srcdir)/include
libspdy_la_LDFLAGS = `pkg-config --libs zlib`

# This flag accepts an argument of the form current[:revision[:age]]. So,
# passing -version-info 3:12:1 sets current to 3, revision to 12, and age to
# 1.
#
# Here's the simplified rule guide on how to change -version-info:
# (current version is C:R:A)
#
# 1. if there are only source changes, use C:R+1:A
# 2. if interfaces were added use C+1:0:A+1
# 3. if interfaces were removed, then use C+1:0:0
VERSIONINFO=-version-info 0:0:0

lib_LTLIBRARIES = libspdy.la
