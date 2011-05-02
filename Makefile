# libspdy makefile
#
CC=gcc

CFLAGS += -Werror -Wall -Wextra -Wformat=2 -std=c99 -pedantic -DDEBUG
LDFLAGS +=

SRCS_SPDY =  src/spdy_frame.c
SRCS_SPDY += src/spdy_control_frame.c
SRCS_SPDY += src/spdy_syn_stream.c
SRCS_SPDY += src/spdy_syn_reply.c
SRCS_SPDY += src/spdy_rst_stream.c
SRCS_SPDY += src/spdy_data_frame.c
SRCS_SPDY += src/spdy_nv_block.c
SRCS_SPDY += src/spdy_zlib.c
SRCS_SPDY += src/spdy_stream.c
HDRS_SPDY = $(SRCS_SPDY,.c=.h)
OBJS_SPDY = $(SRCS_SPDY:.c=.o)

SRCS_TEST =  tests/check_spdy.c
SRCS_TEST += tests/check_spdy_frame.c
SRCS_TEST += tests/check_spdy_control_frame.c
SRCS_TEST += tests/check_spdy_syn_stream.c
SRCS_TEST += tests/check_spdy_syn_reply.c
SRCS_TEST += tests/check_spdy_rst_stream.c
SRCS_TEST += tests/check_spdy_data_frame.c
SRCS_TEST += tests/check_spdy_nv_block.c
SRCS_TEST += tests/check_spdy_zlib.c
SRCS_TEST += tests/testdata.c
OBJS_TEST = $(SRCS_TEST:.c=.o)
LIBS_TEST = `pkg-config --libs check` `pkg-config --libs zlib`
EXEC_TEST = tests/checks

AUX += $(HDRS_SPDY)

all: spdy 

spdy: $(OBJS_SPDY)

checks_build: $(EXEC_TEST)

checks: checks_build
	$(EXEC_TEST)

$(EXEC_TEST): $(OBJS_SPDY) $(OBJS_TEST)
	$(CC) $(CFLAGS) $(LDFLAGS) -g $(OBJS_SPDY) $(OBJS_TEST) $(LIBS_TEST) -o $(EXEC_TEST)

doc:
	doxygen Doxyfile

clean:
	rm $(OBJS_SPDY)||true
	rm $(OBJS_TEST)||true
	rm $(EXEC_TEST)||true

cloc:
	cloc src/

tags:
	ctags -R src/

gource:
	gource -1440x900 -a 1 --file-extensions --hide date --disable-progress --stop-at-end --output-ppm-stream - | ffmpeg -y -b 3000k -r 60 -f image2pipe -vcodec ppm -i - -fpre /usr/share/ffmpeg/libx264-medium.ffpreset -vcodec libx264 gource.mp4

.PHONY: doc
