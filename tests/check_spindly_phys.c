#include "spindly.h"

#include <check.h>

static const unsigned char SPDY_SYN_STREAM[] =
  "\x80\x00\x00\x01\x00\x00\x00\x0a" \
  "\x00\x00\x00\x01\x00\x00\x00\x00" \
  "\x00\x00\x78\xbb\xdf\xa2\x51\xb2" \
  "\x63\x60\x00\x00\x00\x02\x00\x01";

START_TEST (test_spindly_phys_init)
{
  struct spindly_phys *phys_client;
  struct spindly_phys *phys_server;
  struct spindly_stream *stream_client;
  struct spindly_demux demux;
  spindly_error_t spint;
  unsigned char *data;
  size_t datalen;

  phys_client = spindly_phys_init(SPINDLY_SIDE_CLIENT, SPINDLY_DEFAULT, NULL);
  fail_unless(phys_client != NULL, "spindly_phys_init() failed");

  phys_server = spindly_phys_init(SPINDLY_SIDE_SERVER, SPINDLY_DEFAULT, NULL);
  fail_unless(phys_server != NULL, "spindly_phys_init() failed");

  spint = spindly_stream_new(phys_client, 0, &stream_client, NULL, NULL);
  fail_unless(spint == SPINDLYE_OK, "spindly_stream_new() failed");

  spint = spindly_phys_outgoing(phys_client, &data, &datalen);
  fail_unless(spint == SPINDLYE_OK, "spindly_phys_outgoing() failed");

  printf("got datalen %d\n", datalen);

#if 0
  {
    size_t i;
    for(i=0; i<datalen; i++) {
      printf("%02x%s ", data[i], SPDY_SYN_STREAM[i]==data[i]?"":"*");
    }
    printf("\n (%d bytes)", i);
  }
#endif

  fail_unless(datalen == 32, "spindly_phys_outgoing() returned bad value");


  fail_unless(memcmp(data, SPDY_SYN_STREAM, datalen) == 0,
              "SYN_STREAM data wrong");

  /* now feed the created outgoing packet from the client as incoming
     in the server! */
  spint = spindly_phys_incoming(phys_server, data, datalen, NULL);
  fail_unless(spint == SPINDLYE_OK, "spindly_phys_incoming() failed");

  /* demux the incoming data */
  spint = spindly_phys_demux(phys_server, &demux);
  fail_unless(spint == SPINDLYE_OK, "spindly_phys_demux() failed");

  printf("Demux type %d\n", demux.type);

  fail_unless(demux.type == SPINDLY_DX_STREAM_REQ,
              "spindly_phys_demux() demuxed incorrect message");

  spindly_phys_cleanup(phys_client);
  spindly_phys_cleanup(phys_server);
}
END_TEST


Suite *spindly_phys_suite()
{
  Suite *s = suite_create("spindly_phys");
  TCase *tc_core = tcase_create("spindly_phys_init");
  tcase_add_test(tc_core, test_spindly_phys_init);
  suite_add_tcase(s, tc_core);
  return s;
}

