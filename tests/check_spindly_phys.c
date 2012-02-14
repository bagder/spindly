#include "spindly.h"

#include <check.h>

static const unsigned char SPDY_SYN_STREAM[18] =
  "\x80\x00\x00\x01\x00\x00\x00\x0a\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00";

START_TEST (test_spindly_phys_init)
{
  struct spindly_phys *phys_client;
  struct spindly_phys *phys_server;
  struct spindly_stream *stream_client;
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

  fail_unless(datalen == 18, "spindly_phys_outgoing() returned bad value");

#if 0
  {
    size_t i;
    for(i=0; i<datalen; i++) {
      printf("\\x%02x", data[i]);
    }
    printf("\n");
  }
#endif

  fail_unless(memcmp(data, SPDY_SYN_STREAM, datalen) == 0,
              "SYN_STREAM data wrong");

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

