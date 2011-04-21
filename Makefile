# libspdy makefile
#
CC=gcc

CFLAGS += -Werror -Wall -Wextra -Wformat=2 -std=c99 -pedantic
LDFLAGS +=

SRCS_SPDY =  spdy_frame.c
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

.PHONY: doc
