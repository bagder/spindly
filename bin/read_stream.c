#include <stdio.h>
#include <stdlib.h>

#include "spdy_data.h"
#include "spdy_stream.h"
#include "spdy_zlib.h"
#include "spdy_error.h"

int main(int argc, char *argv[]) {
	if(argc != 2) {
		printf("Usage:\n%s [file]\n", argv[0]);
		return EXIT_FAILURE;
	}
	FILE *f = fopen(argv[1], "r");
	if(f == NULL) {
		printf("Failed to open file \"%s\".\n", argv[1]);
		return EXIT_FAILURE;
	}

	spdy_zlib_context zlib_ctx;
	if(spdy_zlib_inflate_init(&zlib_ctx) < 0) {
		printf("Failed to infalte init");
		return EXIT_FAILURE;
	}
	int ret = SPDY_ERROR_INSUFFICIENT_DATA;
	char pkg[2048];
	int ptr=0;
	spdy_frame frm;
	spdy_stream strm;
	spdy_stream_init(&strm, 1, 1);
	spdy_data data;
	if(fread(pkg, 1, 8, f) != 8) {
		printf("Can't read/\n");
		return EXIT_FAILURE;
	}
	spdy_data_use(&data, pkg, 8);
	ret = spdy_frame_parse(
			&frm,
			&data,
			&zlib_ctx);
	while(ret == SPDY_ERROR_INSUFFICIENT_DATA) {
		if(fread(pkg+ptr, 1, 8, f) != 8) {
			printf("Can't read.\n");
			return EXIT_FAILURE;
		}
		ptr += 8;
		ret=spdy_frame_parse(
				&frm,
				spdy_data_use(&data, pkg, ptr),
				&zlib_ctx);
	}
	printf("DONE\n");
	return EXIT_SUCCESS;
}

