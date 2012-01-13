#  read_stream does not compile
bin_PROGRAMS = create_nv_block read_dump

AM_CFLAGS= -I$(top_srcdir)/include

LDADD = $(top_builddir)/src/libspdy.la
AM_LDFLAGS = `pkg-config --libs check` `pkg-config --libs zlib`


