/*
 * Home of the spindly_phys_*() functions.
 */
#include "spdy_setup.h" /* MUST be the first header to include */

#ifdef HAVE_STDINT_H
#include <stdint.h>
#else
typedef unsigned int uint32_t;
#endif

#include "spindly.h"

/* fixed size buffer used to create outgoing data into that is then told to
   the application */
#define PHYS_BUFFER_SIZE 16738

struct spindly_phys {
  enum { PHYS_CLIENT, PHYS_SERVER } side;

  struct spindly_stream **streams;
  int added_stream;  /* how many have been added so far */
  int added_alloced; /* how large is the streams array alloc */

  uint32_t streamid; /* the next streamid to ask for */
  char outbuf[PHYS_BUFFER_SIZE];
};

/*
 * Create a handle for a single duplex connection, SIDE is either client or
 * server - what side the handle is made to handle. PROTVER is the specific
 * SPDY protocol version.
 */
struct spindly_phys *spindly_phys_init(int side, int protver)
{


}

/*
 * when the application has read data off the transport, this function is
 * called to tell Spindly about more data that has arrived. As spindly doesn't
 * read any network data by itself, it needs to get the data passed into it by
 * the application.
 *
 * After data has been fed into the handle, call spindly_phys_demux() to make
 * it demux the incoming data.
 */

spindly_error_t spindly_phys_incoming(struct spindly_phys *phys,
                                      unsigned char *data,
                                      size_t datalen)
{


}

/*
 * Returns information about incoming data on the connection, split up for
 * consumption accordingly. Subsequent calls will return the next result and
 * so on until there's nothing left to demux - until spindly_phys_incoming()
 * is called again to feed it with more data. Not that when it returns that
 * there is no more message, it may still hold trailing data that forms the
 * beginning of the subsequent message. 'ptr' will point to a struct dedicated
 * to the particular message.
 */
spindly_error_t spindly_phys_demux(struct spindly_phys *phys,
                                   spindly_demux_t *msg,
                                   void **ptr)
{


}

/*
 * Returns info (pointer and length) about the data that PHYS holds that is
 * available to send over the transport medium immediately.
 */
spindly_error_t spindly_phys_ready(struct spindly_phys *phys,
                                   unsigned char **data,
                                   size_t len)
{


}

/*
 * Tell Spindly how many bytes of the data that has been sent and should be
 * considered consumed. The PHYS will then contain updated information of
 * amount of remaining data to send etc.
 */
spindly_error_t spindly_phys_sent(struct spindly_phys *phys,
                                  size_t len)
{

}

/*
 * Change one or more settings associated with the connection. This will
 * result in a SPINDLY_DX_SETTINGS message to end up on the remote side.
 *
 * TODO: figure out how to pass in 'settings' the best way
 */
spindly_error_t spindly_phys_settings(struct spindly_phys *phys,
                                      void *settings)
{

}

/*
 * Cleanup the entire connection and all associated streams and data.
 */
void spindly_phys_cleanup(struct spindly_phys *phys)
{

}
