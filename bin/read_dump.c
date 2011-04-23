#include <stdio.h>
#include <stdlib.h>

#include "spdy_frame.h"
#include "spdy_control_frame.h"
#include "spdy_data_frame.h"
#include "spdy_nv_block.h"
#include "spdy_zlib.h"

int handle_data_frame(spdy_frame *frame, FILE *f) {
	spdy_data_frame *data_frm = (spdy_data_frame*)frame->frame;
	printf("Data frame:\n");
	printf("\tStream ID:   % 5d\n", data_frm->stream_id);
	printf("\tFlags:       % 5d\n", data_frm->flags);
	printf("\tLength:      % 5d\n", data_frm->length);
	char *payload = malloc(sizeof(char)*data_frm->length);
	if(!payload) {
		printf("Failed to allocate memory for payload.\n");
		return EXIT_FAILURE;
	}
	if(fread(payload, 1, data_frm->length, f) != data_frm->length) {
		printf("Failed to read payload from file.\n");
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
int handle_control_frame(spdy_frame *frame, FILE *f) {
	spdy_control_frame *ctrl_frm = (spdy_control_frame*)frame->frame;
	printf("Control frame:\n");
	printf("\tVersion:      % 5d\n", ctrl_frm->version);
	printf("\tType:         % 5d (%s)\n",
			ctrl_frm->type,
			spdy_control_frame_get_type_name(ctrl_frm->type));
	printf("\tFlags:        % 5d\n", ctrl_frm->flags);
	printf("\tLength:       % 5d\n", ctrl_frm->length);
	char *payload = malloc(sizeof(char)*ctrl_frm->length);
	if(!payload) {
		printf("Failed to allocate memory for payload.\n");
		return EXIT_FAILURE;
	}
	if(fread(payload, 1, ctrl_frm->length, f) != ctrl_frm->length) {
		printf("Failed to read payload from file.\n");
		return EXIT_FAILURE;
	}
	switch(ctrl_frm->type) {
		case SPDY_CTRL_SYN_STREAM:
			// Skip the SYN_STREAM header to the NV block.
			payload += 10;
			break;
		case SPDY_CTRL_SYN_REPLY:
			payload += 6;
			break;
	}
	char *nv_block_deflate;
	size_t nv_block_deflate_size;
	spdy_zlib_context ctx;
	spdy_zlib_inflate_init(&ctx);
	if(spdy_zlib_inflate(&ctx, payload, ctrl_frm->length, &nv_block_deflate, &nv_block_deflate_size) < 0) {
		printf("Failed to inflate NV block.\n");
		return EXIT_FAILURE;
	}

	spdy_nv_block nv_block;
	if(spdy_nv_block_parse(&nv_block, nv_block_deflate, nv_block_deflate_size) < 0) {
		printf("Failed to parse NV block.\n");
		return EXIT_FAILURE;
	}
	printf("\tNV Block:     % 5d Pairs\n", nv_block.count);
	for(int i=0; i < nv_block.count; i++) {
		printf("\t\t%s: %s\n", nv_block.pairs[i].name, nv_block.pairs[i].values);
	}
	return 0;
}

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
	char pkg[8];
	while(1) {
		if(fread(pkg, 1, 8, f) != 8) {
			printf("Couldn't read header.\n");
			return EXIT_FAILURE;
		}
		spdy_frame frame;
		if(spdy_frame_parse_header(&frame, pkg) < 0) {
			printf("Failed to parse frame header.\n");
			return EXIT_FAILURE;
		}
		switch(frame.type) {
			case SPDY_DATA_FRAME:
				if(handle_data_frame(&frame, f) == EXIT_FAILURE) {
					return EXIT_FAILURE;
				}
				break;
			case SPDY_CONTROL_FRAME:
				if(handle_control_frame(&frame, f) == EXIT_FAILURE) {
					return EXIT_FAILURE;
				}
				break;
		}
		if(feof(f)) {
			return EXIT_SUCCESS;
		}
	}
	fclose(f);
	return EXIT_SUCCESS;
}

