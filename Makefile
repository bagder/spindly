# libspdy makefile
#
CC=gcc

CFLAGS += -Werror -Wall -Wextra -Wformat=2 -std=c99 -pedantic
LDFLAGS +=

SRCS_SPDY =  src/spdy_frame.c
SRCS_SPDY += src/spdy_nv_block.c
HDRS_SPDY = $(SRCS_SPDY,.c=.h)
OBJS_SPDY = $(SRCS_SPDY:.c=.o)

SRCS_TEST =  tests/check_spdy_nv_block.c
OBJS_TEST = $(SRCS_TEST:.c=.o)
LIBS_TEST = `pkg-config --libs check`
EXEC_TEST = tests/checks

AUX += $(HDRS_SPDY)

all: spdy 

spdy: $(OBJS_SPDY)

checks: $(EXEC_TEST)
$(EXEC_TEST): $(OBJS_SPDY) $(OBJS_TEST)
	$(CC) $(CFLAGS) $(LDFLAGS) -g $(OBJS_SPDY) $(OBJS_TEST) $(LIBS_TEST) -o $(EXEC_TEST)
	$(EXEC_TEST)

doc:
	doxygen Doxyfile

clean:
	rm $(OBJS_SPDY)||true
	rm $(OBJS_TEST)||true
	rm $(EXEC_TEST)||true

gource:
	gource -a 1 --bloom-multiplier 1 --bloom-intensity 2 --hide date --disable-progress --stop-at-end --output-ppm-stream - | ffmpeg -y -b 3000k -r 60 -f image2pipe -vcodec ppm -i - -fpre /usr/share/ffmpeg/libx264-default.ffpreset -vcodec libx264 gource.mp4

.PHONY: doc
