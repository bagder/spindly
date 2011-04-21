# libspdy makefile
#
CC=gcc

CFLAGS += -Werror -Wall -Wextra -Wformat=2 -std=c99 -pedantic
LDFLAGS +=

SRCS_SPDY =  spdy_frame.c
SRCS_SPDY += spdy_nv_block.c
HDRS_SPDY = $(SRCS_SPDY,.c=.h)
OBJS_SPDY = $(SRCS_SPDY:.c=.o)

AUX += $(HDRS_SPDY)

EXECUTABLE=spdy_test

all: spdy

spdy: $(OBJS_SPDY)

doc:
	doxygen Doxyfile

clean:
	rm $(EXECUTABLE)||true
	rm $(OBJS_SPDY)||true

gource:
	gource -a 1 --bloom-multiplier 1 --bloom-intensity 2 --hide date --disable-progress --stop-at-end --output-ppm-stream - | ffmpeg -y -b 3000k -r 60 -f image2pipe -vcodec ppm -i - -fpre /usr/share/ffmpeg/libx264-default.ffpreset -vcodec libx264 gource.mp4

.PHONY: doc
