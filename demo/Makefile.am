
noinst_PROGRAMS = server client

server_SOURCES = server.c

client_SOURCES = client.c

AM_LDFLAGS = `pkg-config --libs check` `pkg-config --libs zlib`

LDADD = $(top_builddir)/src/libspdy.la

AM_CFLAGS = `pkg-config --cflags check` -I$(top_srcdir)/include

