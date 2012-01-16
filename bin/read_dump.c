#include <stdio.h>
#include <stdlib.h>

#include "spdy_setup.h"
#include "spdy_data.h"
#include "spdy_frame.h"
#include "spdy_control_frame.h"
#include "spdy_syn_stream.h"
#include "spdy_syn_reply.h"
#include "spdy_data_frame.h"
#include "spdy_nv_block.h"
#include "spdy_zlib.h"
#include "spdy_error.h"

void handle_data_frame(spdy_frame *frame)
{
  spdy_data_frame *data_frm = (spdy_data_frame *)&frame->frame.data;
  printf("Data frame:\n");
  printf("\tStream ID:    % 5d\n", data_frm->stream_id);
  printf("\tFlags:        % 5d\n", data_frm->flags);
  printf("\tLength:       % 5d\n", data_frm->length);
}

void handle_control_frame(spdy_frame *frame)
{
  int i;
  spdy_syn_stream *syn_stream = NULL;
  spdy_syn_reply *syn_reply = NULL;
  spdy_control_frame *ctrl_frm = (spdy_control_frame *)&frame->frame.control;
  printf("Control frame:\n");
  printf("\tVersion:      % 5d\n", ctrl_frm->version);
  printf("\tType:         % 5d (%s)\n",
         ctrl_frm->type, spdy_control_frame_get_type_name(ctrl_frm->type));
  printf("\tFlags:        % 5d\n", ctrl_frm->flags);
  printf("\tLength:       % 5d\n", ctrl_frm->length);
  switch (ctrl_frm->type) {
  case SPDY_CTRL_SYN_STREAM:
    syn_stream = (spdy_syn_stream *)&ctrl_frm->obj.syn_stream;
    printf("\tStream-ID:    % 5d\n", syn_stream->stream_id);
    printf("\tAssociated:   % 5d\n", syn_stream->associated_to);
    printf("\tPriority:     % 5d\n", syn_stream->priority);
    printf("\tNV Block:     % 5d\n", syn_stream->nv_block.count);
    for(i = 0; i < syn_stream->nv_block.count; i++) {
      printf("\t\t%s: %s\n", syn_stream->nv_block.pairs[i].name,
             syn_stream->nv_block.pairs[i].values);
    }
    break;
  case SPDY_CTRL_SYN_REPLY:
    syn_reply = (spdy_syn_reply *)&ctrl_frm->obj.syn_reply;
    printf("\tStream-ID:    % 5d\n", syn_reply->stream_id);
    printf("\tNV Block:     % 5d\n", syn_reply->nv_block.count);
    for(i = 0; i < syn_reply->nv_block.count; i++) {
      printf("\t\t%s: %s\n", syn_reply->nv_block.pairs[i].name,
             syn_reply->nv_block.pairs[i].values);
    }
    break;
  default:
    printf("Unknown frame type.");
    break;
  }
}

char *read_file(const char *filename, long *fsiz)
{
  FILE *f;
  long siz;
  char *buf;
  size_t res;
  f = fopen(filename, "rb");
  if(f == NULL)
    return NULL;
  fseek(f, 0, SEEK_END);
  siz = ftell(f);
  rewind(f);
  buf = (char *)malloc(sizeof(char) * siz);
  if(buf == NULL)
    return NULL;
  res = fread(buf, 1, siz, f);
  if(res != siz) {
    free(buf);
    return NULL;
  }
  *fsiz = siz;
  return buf;
}

int main(int argc, char *argv[])
{
  if(argc != 2) {
    printf("Usage:\n%s [file]\n", argv[0]);
    return EXIT_FAILURE;
  }
  long fsiz;
  char *fbuf = read_file(argv[1], &fsiz);
  if(fbuf == NULL) {
    printf("Failed to open file \"%s\".\n", argv[1]);
    return EXIT_FAILURE;
  }
  spdy_zlib_context zlib_ctx;
  spdy_frame frame;
  spdy_frame_init(&frame);
  spdy_data data;
  spdy_data_use(&data, fbuf, fsiz);
  spdy_zlib_inflate_init(&zlib_ctx);
  int ret = spdy_frame_parse(&frame, &data, &zlib_ctx);
  if(ret == SPDY_ERROR_NONE) {
    switch (frame.type) {
    case SPDY_DATA_FRAME:
      handle_data_frame(&frame);
      break;
    case SPDY_CONTROL_FRAME:
      handle_control_frame(&frame);
      break;
    }
  }
  spdy_frame_destroy(&frame);
  spdy_zlib_inflate_end(&zlib_ctx);
  free(fbuf);
  return EXIT_SUCCESS;
}
