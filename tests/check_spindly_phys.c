#include "spindly.h"

#include <check.h>

static const unsigned char SPDY_SYN_STREAM[] =
  "\x80\x02\x00\x01\x00\x00\x00\x0a"
  "\x00\x00\x00\x01\x00\x00\x00\x00"
  "\x00\x00\x78\xbb\xdf\xa2\x51\xb2"
  "\x63\x60\x00\x00\x00\x02\x00\x01";

static const unsigned char SPDY_SYN_REPLY[] =
  "\x80\x02\x00\x02\x00\x00\x00\x0a"
  "\x00\x00\x00\x01\x00\x00"
  "\x78\xbb\xdf\xa2\x51\xb2"
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

  /* CLIENT: create a handle for the physical connection */
  phys_client = spindly_phys_init(SPINDLY_SIDE_CLIENT, SPINDLY_DEFAULT, NULL);
  fail_unless(phys_client != NULL, "spindly_phys_init() failed");

  /* SERVER: create a handle for the physical connection */
  phys_server = spindly_phys_init(SPINDLY_SIDE_SERVER, SPINDLY_DEFAULT, NULL);
  fail_unless(phys_server != NULL, "spindly_phys_init() failed");

  /* CLIENT: create a stream on the physical connection */
  spint = spindly_stream_new(phys_client, 0, &stream_client, NULL, NULL);
  fail_unless(spint == SPINDLYE_OK, "spindly_stream_new() failed");

  /* CLIENT: get data to send */
  spint = spindly_phys_outgoing(phys_client, &data, &datalen);
  fail_unless(spint == SPINDLYE_OK, "spindly_phys_outgoing() failed");

#if 0
  {
    size_t i;
    for(i=0; i<datalen; i++) {
      printf("%02x%s ", data[i], SPDY_SYN_STREAM[i]==data[i]?"":"*");
    }
    printf("\n (%d bytes)\n", i);
  }
#endif

  fail_unless(datalen == 32, "spindly_phys_outgoing() returned bad value");
  fail_unless(memcmp(data, SPDY_SYN_STREAM, datalen) == 0,
              "SYN_STREAM data wrong");

  /* SERVER: now feed the created outgoing packet from the client as incoming
     in the server! */
  spint = spindly_phys_incoming(phys_server, data, datalen,
                                SPINDLY_INCOMING_NONE, NULL);
  fail_unless(spint == SPINDLYE_OK, "spindly_phys_incoming() failed");

  /* NOTE: since spindly_phys_incoming() does not immediately copy the data
     passed to it, we cannot immediately call spindly_phys_sent() */

  /* SERVER: demux the incoming data */
  spint = spindly_phys_demux(phys_server, &demux);
  fail_unless(spint == SPINDLYE_OK, "spindly_phys_demux() failed");

  fail_unless(demux.type == SPINDLY_DX_STREAM_REQ,
              "spindly_phys_demux() demuxed incorrect message");
  fail_unless(demux.msg.stream.stream != NULL,
              "spindly_phys_demux() demuxed incorrect message");

  /* CLIENT: consider the data sent and tell spindly so */
  spint = spindly_phys_sent(phys_client, datalen);
  fail_unless(spint == SPINDLYE_OK, "spindly_phys_sent() failed");

  /* SERVER: ACK the new stream */
  spint = spindly_stream_ack(demux.msg.stream.stream);
  fail_unless(spint == SPINDLYE_OK, "spindly_stream_ack() failed");

  /* SERVER: figure out what to send to client */
  spint = spindly_phys_outgoing(phys_server, &data, &datalen);
  fail_unless(spint == SPINDLYE_OK, "spindly_phys_outgoing() failed");

  fail_unless(datalen == 28, "spindly_phys_outgoing() returned bad value");

#if 0
  {
    size_t i;
    for(i=0; i<datalen; i++) {
      printf("%02x%s ", data[i], SPDY_SYN_REPLY[i]==data[i]?"":"*");
    }
    printf("\n (%d bytes)\n", i);
  }
#endif
  fail_unless(memcmp(data, SPDY_SYN_REPLY, datalen) == 0,
              "SYN_REPLY data wrong");

  /* CLIENT feed the data back as incoming, as a response to what the client
     sent initially */
  spint = spindly_phys_incoming(phys_client, data, datalen,
                                SPINDLY_INCOMING_COPY, NULL);
  fail_unless(spint == SPINDLYE_OK, "spindly_phys_incoming() failed");

  /* NOTE: since spindly_phys_incoming() in this case *does* immediately copy
     the data passed to it, we can call spindly_phys_sent() immediately if we
     want to. */

  /* CLIENT: demux the incoming data */
  spint = spindly_phys_demux(phys_client, &demux);
  fail_unless(spint == SPINDLYE_OK, "spindly_phys_demux() failed");
  fail_unless(demux.type == SPINDLY_DX_STREAM_ACK,
              "spindly_phys_demux() demuxed incorrect message");
  fail_unless(demux.msg.stream.stream != NULL,
              "spindly_phys_demux() demuxed incorrect message");

  /* SERVER: consider the data sent and tell spindly so */
  spint = spindly_phys_sent(phys_server, datalen);
  fail_unless(spint == SPINDLYE_OK, "spindly_phys_sent() failed");

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

