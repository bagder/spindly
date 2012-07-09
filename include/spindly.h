#ifndef SPINDLY_H
#define SPINDLY_H 1
/***************************************************************************
 *  Project      _           _ _
 *     ___ _ __ (_)_ __   __| | |_   _
 *    / __| '_ \| | '_ \ / _` | | | | |
 *    \__ \ |_) | | | | | (_| | | |_| |
 *    |___/ .__/|_|_| |_|\__,_|_|\__, |
 *        |_|                    |___/
 *
 * Copyright (C) 2012, Daniel Stenberg <daniel@haxx.se>
 *
 * This software is licensed as described in the file LICENSE, which you
 * should have received as part of this distribution. The terms are also
 * available at http://spindly.haxx.se/license.html
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY
 * KIND, either express or implied.
 *
 ***************************************************************************/

#include <stdio.h>

#define SPINDLY_COPYRIGHT "2011-2012 The spindly project and its contributors"

/* The stringified version. This may have -DEV appended for non-released
   versions. */
#define SPINDLY_VERSION                             "0.1.0-DEV"

/* The numeric version number is also available "in parts" by using these
   defines: */
#define SPINDLY_VERSION_MAJOR                       0
#define SPINDLY_VERSION_MINOR                       1
#define SPINDLY_VERSION_PATCH                       0

/* This is the numeric version of the spindly version number, meant for easier
   parsing and comparions by programs. The SPINDLY_VERSION_NUM define will
   always follow this syntax:

         0xXXYYZZ

   Where XX, YY and ZZ are the main version, release and patch numbers in
   hexadecimal (using 8 bits each). All three numbers are always represented
   using two digits.  1.2 would appear as "0x010200" while version 9.11.7
   appears as "0x090b07".

   This 6-digit (24 bits) hexadecimal number does not show pre-release number,
   and it is always a greater number in a more recent release. It makes
   comparisons with greater than and less than work.
*/
#define SPINDLY_VERSION_NUM                         0x000100

/*
 * This is the date and time when the full source package was created. The
 * timestamp is not stored in the source code repo, as the timestamp is
 * properly set in the tarballs by the maketgz script.
 *
 * The format of the date should follow this template:
 *
 * "Mon Feb 12 11:35:33 UTC 2007"
 */
#define SPINDLY_TIMESTAMP "DEV"

/* A few generic types and forward-declarations */

struct spindly_phys;
struct spindly_stream;

typedef unsigned int spindly_streamid_t;

/*
 * This struct defines a header pair. Note that both the name and the value
 * may contain binary zero octets, making ordinary C string operations not
 * suitable to work with them. Beware.
 */
struct spindly_header_pair
{
  size_t namelen;
  char *name;                   /* pointer to a name of 'namelen' bytes */
  size_t valuelen;
  char *value;                  /* pointer to a value of 'valuelen' bytes */
};

/*
 * Create a handle for a single duplex connection, SIDE is either client or
 * server - what side the handle is made to handle. PROTVER is the specific
 * SPDY protocol version.
 */
typedef enum
{
  /* deliberately not using 0 to force a more active choice */
  SPINDLY_SIDE_CLIENT = 1,
  SPINDLY_SIDE_SERVER = 2
} spindly_side_t;

typedef enum
{
  SPINDLY_DEFAULT,              /* allow spindly to decide or figure out */
  SPINDLY_SPDYVER2 = 2,         /* SPDY draft-2 protocol */
  SPINDLY_SPDYVER3 = 3,         /* SPDY draft-3 protocol */

} spindly_spdyver_t;

#define SPINDLY_CONFIG_AGE 1
struct spindly_phys_config
{
  int age;                      /* MUST be set to SPINDLY_CONFIG_AGE */
};

struct spindly_stream_config
{
  int age;                      /* MUST be set to SPINDLY_CONFIG_AGE */
};

struct spindly_phys *spindly_phys_init(spindly_side_t side,
                                       spindly_spdyver_t protver,
                                       struct spindly_phys_config *config);

typedef enum
{
  SPINDLYE_OK,           /* 0 - all is fine */
  SPINDLYE_NOMEM,        /* 1 - out of memory */
  SPINDLYE_INVAL,        /* 2 - invalid input/argument/value */
  SPINDLYE_INSUFF,       /* 3 - insufficient data */
  SPINDLYE_ZLIB,         /* 4 - a compression problem */
  SPINDLYE_RESET,        /* 5 - stream was (unexpectedly) reset */
  SPINDLYE_STOP,         /* 6 - stream was (unexpectedly) terminated */
  SPINDLYE_SMALL_BUFFER, /* 7 - too small buffer */
  SPINDLYE_PROTOCOL,     /* 8 - bad SPDY protocol received */

  SPINDLYE_LAST          /* not used, always the last */
} spindly_error_t;


#define SPINDLY_INCOMING_NONE (0)
#define SPINDLY_INCOMING_COPY (1<<0) /* force a copy at once */

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
                                      unsigned char *data, size_t datalen,
                                      int flags,
                                      void *identifier);



typedef enum
{                               /* what 'ptr' points to */
  SPINDLY_DX_NONE,              /* NULL */
  SPINDLY_DX_GOAWAY,            /* NULL */
  SPINDLY_DX_STREAM_ACK,        /* struct spindly_dx_stream */
  SPINDLY_DX_STREAM_REQ,        /* struct spindly_dx_stream */
  SPINDLY_DX_STREAM_KILL,       /* struct spindly_dx_stream */
  SPINDLY_DX_SETTINGS,          /* struct spindly_dx_settings */
  SPINDLY_DX_PING,              /* NULL */
  SPINDLY_DX_DATA,              /* struct spindly_dx_data */
  SPINDLY_DX_HEADERS,           /* struct spindly_dx_headers */

  SPINDLY_DX_LAST               /* not used, always the last */
} spindly_demux_t;

/*
 * Returns information about incoming data on the connection, split up for
 * consumption accordingly. Subsequent calls will return the next result and
 * so on until there's nothing left to demux - until spindly_phys_incoming()
 * is called again to feed it with more data. Not that when it returns that
 * there is no more message, it may still hold trailing data that forms the
 * beginning of the subsequent message. 'ptr' will point to a struct dedicated
 * to the particular message.
 */

struct spindly_dx_stream
{
  spindly_streamid_t streamid;
  struct spindly_stream *stream;        /* NULL or pointing to the handle */
};

struct spindly_dx_settings
{
  /* TODO: how should this be? */
  char *setting;
  char *value;
};

struct spindly_dx_data
{
  struct spindly_stream *stream;
  unsigned char *datap;
  size_t len;
};

struct spindly_dx_headers
{
  struct spindly_stream *stream;
  int num_of_pairs;                      /* there is at least 1 */
  struct spindly_header_pair headers[1]; /* this array will be num_of_pairs
                                            size big */
};

struct spindly_demux {
  spindly_demux_t type;
  union {
    struct spindly_dx_stream stream;
    struct spindly_dx_settings settings;
  } msg;
};

spindly_error_t spindly_phys_demux(struct spindly_phys *phys,
                                   struct spindly_demux *ptr);

/*
 * Returns info (pointer and length) about the data that PHYS holds that is
 * available to send over the transport medium immediately.
 */
spindly_error_t spindly_phys_outgoing(struct spindly_phys *phys,
                                      unsigned char **data, size_t *len);

/*
 * Tell Spindly how many bytes of the data that has been sent and should be
 * considered consumed. The PHYS will then contain updated information of
 * amount of remaining data to send etc.
 */
spindly_error_t spindly_phys_sent(struct spindly_phys *phys, size_t len);

/*
 * Change one or more settings associated with the connection. This will
 * result in a SPINDLY_DX_SETTINGS message to end up on the remote side.
 *
 * TODO: figure out how to pass in 'settings' the best way
 */
spindly_error_t spindly_phys_settings(struct spindly_phys *phys,
                                      void *settings);

/*
 * Cleanup the entire connection and all associated streams and data.
 */
void spindly_phys_cleanup(struct spindly_phys *phys);

/*
 * Handle separate streams over the physical connection
 * ====================================================
 */

/*
 * Creates a request for a new stream and muxes the request into the output
 * connection, creates a STREAM handle for the new stream and returns the
 * RESULT. The CUSTOMP pointer will be associated with the STREAM to allow the
 * application to identify it.
 *
 * PRIO is a priority, 0 - 7 where 0 is the most important.
 *
 * Note that the stream is not yet ready to be used until it has been
 * acknowledged by the peer and we get a SPINDLY_DX_STREAM_ACK response.
 *
 */
spindly_error_t spindly_stream_new(struct spindly_phys *phys,
                                   unsigned int prio,
                                   struct spindly_stream **stream,
                                   void *userp,
                                   struct spindly_stream_config *config);


/*
 * The STREAM as requested to get opened by the remote is allowed! This
 * function is only used as a response to a SPINDLY_DX_STREAM_REQ.
 */
spindly_error_t spindly_stream_ack(struct spindly_stream *stream);

/*
 * The STREAM as requested to get opened by the remote is NOT allowed! This
 * function is only used as a response to a SPINDLY_DX_STREAM_REQ.
 */
spindly_error_t spindly_stream_nack(struct spindly_stream *stream);

/*
 * Close the STREAM. Can be used as a response to a SPINDLY_DX_STREAM_KILL
 * message or it will generate such a message to the other side.
 */
spindly_error_t spindly_stream_close(struct spindly_stream *stream);

/*
 * Send data on this stream.
 */
spindly_error_t spindly_stream_data(struct spindly_stream *stream,
                                    unsigned char *data,
                                    size_t len, void *handled);

/*
 * Send headers on this stream.
 */
spindly_error_t spindly_stream_headers(struct spindly_stream *stream,
                                       int num_of_pairs,
                                       struct spindly_header_pair **pairs,
                                       void *handled);

/*
 * Figure out the physical handle a particular stream is associated with.
 */
struct spindly_phys *spindly_stream_getphys(struct spindly_stream *stream);


#endif /* SPINDLY_H */
