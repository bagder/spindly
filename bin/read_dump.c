#include <stdio.h>
#include <stdlib.h>

#include "spdy_frame.h"
#include "spdy_control_frame.h"
#include "spdy_syn_stream.h"
#include "spdy_syn_reply.h"
#include "spdy_data_frame.h"
#include "spdy_nv_block.h"
#include "spdy_zlib.h"

spdy_zlib_context ctx[2];

int handle_syn_stream_frame(spdy_control_frame *frame, char *payload, FILE *f) {
	spdy_syn_stream syn_stream;
	if(spdy_syn_stream_parse(&syn_stream, payload, frame->length, &ctx[0]) < 0) {
		printf("Failed to parse SYN_STREAM.\n");
		return EXIT_FAILURE;
	}
	printf("\tStream-ID:    % 5d\n", syn_stream.stream_id);
	printf("\tAssociated:   % 5d\n", syn_stream.associated_to);
	printf("\tPriority:     % 5d\n", syn_stream.priority);
	printf("\tNV Block:     % 5d\n", syn_stream.nv_block->count);
	for(int i=0; i < syn_stream.nv_block->count; i++) {
		printf("\t\t%s: %s\n", syn_stream.nv_block->pairs[i].name, syn_stream.nv_block->pairs[i].values);
	}
	spdy_nv_block_destroy(syn_stream.nv_block);
	free(syn_stream.nv_block);

	return EXIT_SUCCESS;
}

int handle_syn_reply_frame(spdy_control_frame *frame, char *payload, FILE *f) {
	spdy_syn_reply syn_reply;
	if(spdy_syn_reply_parse(&syn_reply, payload, frame->length, &ctx[1]) < 0) {
		printf("Failed to parse SYN_REPLY.\n");
		return EXIT_FAILURE;
	}
	printf("\tStream-ID:    % 5d\n", syn_reply.stream_id);
	printf("\tNV Block:     % 5d\n", syn_reply.nv_block->count);
	for(int i=0; i < syn_reply.nv_block->count; i++) {
		printf("\t\t%s: %s\n", syn_reply.nv_block->pairs[i].name, syn_reply.nv_block->pairs[i].values);
	}
	spdy_nv_block_destroy(syn_reply.nv_block);
	free(syn_reply.nv_block);

	return EXIT_SUCCESS;
}

int handle_data_frame(spdy_frame *frame, FILE *f) {
	spdy_data_frame *data_frm = (spdy_data_frame*)frame->frame;
	printf("Data frame:\n");
	printf("\tStream ID:    % 5d\n", data_frm->stream_id);
	printf("\tFlags:        % 5d\n", data_frm->flags);
	printf("\tLength:       % 5d\n", data_frm->length);
	char *payload = malloc(sizeof(char)*data_frm->length);
	if(!payload) {
		printf("Failed to allocate memory for payload.\n");
		return EXIT_FAILURE;
	}
	if(fread(payload, 1, data_frm->length, f) != data_frm->length) {
		printf("Failed to read payload from file.\n");
		return EXIT_FAILURE;
	}

	free(payload);
	return EXIT_SUCCESS;
}

int handle_control_frame(spdy_frame *frame, FILE *f) {
	int ret = EXIT_SUCCESS;
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
			ret = handle_syn_stream_frame(ctrl_frm, payload, f);
			break;
		case SPDY_CTRL_SYN_REPLY:
			ret = handle_syn_reply_frame(ctrl_frm, payload, f);
			break;
		default:
			printf("Unknown frame type.");
			ret = EXIT_SUCCESS;
	}
	free(payload);

	return ret;
}

void end_zlib_contexts() {
	spdy_zlib_inflate_end(&ctx[0]);
	spdy_zlib_inflate_end(&ctx[1]);
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

	if(spdy_zlib_inflate_init(&ctx[0]) < 0) {
		printf("Failed to inflate-init");
		return EXIT_FAILURE;
	}
	if(spdy_zlib_inflate_init(&ctx[1]) < 0) {
		printf("Failed to inflate-init");
		return EXIT_FAILURE;
	}
//	spdy_zlib_inflate_init(&ctx[1]);

	char pkg[8];
	while(1) {
		if(fread(pkg, 1, 8, f) != 8) {
			printf("Couldn't read header.\n");
			end_zlib_contexts();
			return EXIT_FAILURE;
		}
		spdy_frame frame;
		if(spdy_frame_parse_header(&frame, pkg) < 0) {
			free(frame.frame);
			end_zlib_contexts();
			printf("Failed to parse frame header.\n");
			return EXIT_FAILURE;
		}
		switch(frame.type) {
			case SPDY_DATA_FRAME:
				if(handle_data_frame(&frame, f) == EXIT_FAILURE) {
					free(frame.frame);
					end_zlib_contexts();
					return EXIT_FAILURE;
				}
				break;
			case SPDY_CONTROL_FRAME:
				if(handle_control_frame(&frame, f) == EXIT_FAILURE) {
					free(frame.frame);
					end_zlib_contexts();
					return EXIT_FAILURE;
				}
				break;
		}
		free(frame.frame);
		if(feof(f)) {
			end_zlib_contexts();
			return EXIT_SUCCESS;
		}
	}
	fclose(f);
	return EXIT_SUCCESS;
}

