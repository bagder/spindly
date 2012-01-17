/*
 * Home of the spindly_phys_*() functions.
 */
#include "spdy_setup.h"         /* MUST be the first header to include */

#include <stdlib.h>
#include "spindly.h"
#include "spindly_phys.h"
#include "spindly_stream.h"

/*
 * Create a handle for a single duplex physical connection, SIDE is either
 * client or server - what side the handle is made to handle. PROTVER is the
 * specific SPDY protocol version.
 *
 * TODO: provide a means to replace the memory functions
 */
struct spindly_phys *spindly_phys_init(spindly_side_t side,
                                       spindly_spdyver_t protver,
                                       struct spindly_phys_config *config)
{
  struct spindly_phys *phys;

  /* this is the first malloc, it should use the malloc function provided in
     the config struct if set, but probably cannot use the MALLOC macro */
  phys = malloc(sizeof(struct spindly_phys));
  if(!phys)
    goto fail;
  phys->config = config;
  phys->side = side;
  phys->protver = protver;
  phys->num_streams = 0;
  phys->streamid = 0;

  _spindly_list_init(&phys->streams);
  _spindly_list_init(&phys->outq); /* the list of handles to go over for
                                      outgoing traffic */
  return phys;

fail:
  if(phys)
    free(phys);

  return NULL;
}

spindly_error_t _spindly_phys_add_stream(struct spindly_phys *phys,
                                         struct spindly_stream *s)
{
  _spindly_list_add(&phys->streams, &s->node);
  s->streamid = ++phys->streamid;
  phys->num_streams++;
  return SPINDLYE_OK;
}

#if 0 /* not yet implemented */

/*
 * when the application has read data off the transport, this function is
 * called to tell Spindly about more data that has arrived. As spindly doesn't
 * read any network data by itself, it needs to get the data passed into it by
 * the application.
 *
 * After data has been fed into the handle, call spindly_phys_demux() to make
 * it demux the incoming data.
 *
 */

spindly_error_t spindly_phys_incoming(struct spindly_phys * phys,
                                      unsigned char *data, size_t datalen)
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
                                   spindly_demux_t *msg, void **ptr)
{


}

/*
 * Returns info (pointer and length) about the data that PHYS holds that is
 * available to send over the transport medium immediately.
 */
spindly_error_t spindly_phys_outgoing(struct spindly_phys *phys,
                                      unsigned char **data, size_t len)
{


}

/*
 * Tell Spindly how many bytes of the data that has been sent and should be
 * considered consumed. The PHYS will then contain updated information of
 * amount of remaining data to send etc.
 */
spindly_error_t spindly_phys_sent(struct spindly_phys *phys, size_t len)
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

#endif

/*
 * Cleanup the entire connection.
 */
void spindly_phys_cleanup(struct spindly_phys *phys)
{
  if(phys) {
    FREE(phys, phys);
  }
}
