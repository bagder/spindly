#include <stdio.h>
#include <stdlib.h>

#include "spdy_data.h"
#include "spdy_stream.h"
#include "spdy_zlib.h"
#include "spdy_error.h"

int main(int argc, char *argv[])
{
  FILE *f = fopen(argv[1], "r");
  int ret = SPDY_ERROR_INSUFFICIENT_DATA;
  char pkg[2048];
  int ptr = 0;
  spdy_frame frm;
  spdy_stream strm;
  spdy_data data;
  spdy_zlib_context zlib_ctx;

  if(argc != 2) {
    printf("Usage:\n%s [file]\n", argv[0]);
    return EXIT_FAILURE;
  }
  if(f == NULL) {
    printf("Failed to open file \"%s\".\n", argv[1]);
    return EXIT_FAILURE;
  }

  spdy_stream_init(&strm, 1, 1);
  if(fread(pkg, 1, 8, f) != 8) {
    printf("Can't read/\n");
    return EXIT_FAILURE;
  }
  ptr = 8;
  spdy_data_use(&data, pkg, 8);
  if(spdy_zlib_inflate_init(&zlib_ctx) < 0) {
    printf("Failed to inflate init");
    return EXIT_FAILURE;
  }
  ret = spdy_frame_parse(&frm, &data, &zlib_ctx);
  while(1) {
    if(spdy_zlib_inflate_init(&zlib_ctx) < 0) {
      printf("Failed to inflate init");
      return EXIT_FAILURE;
    }
    while(ret == SPDY_ERROR_INSUFFICIENT_DATA) {
      if(fread(pkg + ptr, 1, 8, f) != 8) {
        printf("Can't read.\n");
        return EXIT_FAILURE;
      }
      ptr += 8;
      ret = spdy_frame_parse(&frm, spdy_data_use(&data, pkg, ptr), &zlib_ctx);
      if(ret == SPDY_ERROR_NONE) {
        printf("LEN: %d\n", ptr);
        if(frm.type == SPDY_DATA_FRAME) {
          printf("Dataframe: %d %*s\n",
                 ((spdy_data_frame *)frm.frame)->length,
                 ((spdy_data_frame *)frm.frame)->length,
                 ((spdy_data_frame *)frm.frame)->data);
        } else if(frm.type == SPDY_CONTROL_FRAME) {
          printf("Control frame.\n");
        } else {
          printf("Other frame.\n");
        }
      } else {
        printf("Working...\n");
      }
    }
    if(ret != SPDY_ERROR_NONE) {
      printf("FAIL YO UDO\n");
      return EXIT_FAILURE;
    }
    ptr = data.length;
    ret = SPDY_ERROR_INSUFFICIENT_DATA;
  }
  printf("DONE\n");
  return EXIT_SUCCESS;
}
