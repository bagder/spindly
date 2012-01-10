
bin_PROGRAMS = runtests

runtests_SOURCES = check_spdy.c check_spdy_frame.c check_spdy_control_frame.c \
  check_spdy_syn_stream.c check_spdy_syn_reply.c check_spdy_rst_stream.c      \
  check_spdy_data_frame.c check_spdy_nv_block.c check_spdy_zlib.c testdata.c \
  check_spindly_phys.c

runtests_LDFLAGS = `pkg-config --libs check` `pkg-config --libs zlib`

runtests_LDADD = $(top_builddir)/src/libspdy.la
runtests_DEPENDENCIES = $(top_builddir)/src/libspdy.la
runtests_CFLAGS = `pkg-config --cflags check` -I$(top_srcdir)/include

run: $(bin_PROGRAMS)
	./$(bin_PROGRAMS)
