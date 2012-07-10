/*
 * Copyright (C) 2012 by Daniel Stenberg.
 *
 * spdy-server.
 *
 * This is an EXAMPLE implementation of a simple SPDY server using the spindly
 * library. Do not assume release quality. Do not assume that this shows "the
 * only way" to do things with spindly. It is an EXAMPLE of how a server can
 * be implemented.
 *
 * It uses TCP on IPv4 on port 9999 by default.
 *
 * Allow many clients to connect. All headers and data that is received by
 * the server get sent out to all connected clients.
 *
 * Right now: only allow a single client.
 */

#include <stdio.h>

#include "sockets.h"

#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <fcntl.h>

#include <spindly.h>

#define TIMEOUT_SECS 20

static int acceptclient(int sock)
{
  socket_t clientfd;
  struct sockaddr_in clientaddr;
  unsigned int len;

  len = sizeof(clientaddr);

  clientfd = accept(sock, (struct sockaddr *) &clientaddr, &len);
  if (clientfd < 0) {
    fprintf(stderr, "accept() failed\n");
    return -1;
  }

  printf("Handling client %s\n", inet_ntoa(clientaddr.sin_addr));

  return clientfd;
}
#define MAXPENDING 4

static int createserver(unsigned short port)
{
  socket_t sock;
  struct sockaddr_in servaddr;

  /* Create socket for incoming connections */
  if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
    errorout("socket() failed");

  /* Construct local address structure */
  memset(&servaddr, 0, sizeof(servaddr));   /* Zero out structure */
  servaddr.sin_family = AF_INET;                /* Internet address family */
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY); /* Any incoming interface */
  servaddr.sin_port = htons(port);              /* Local port */

  /* Bind to the local address */
  if (bind(sock, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0)
    errorout("bind() failed");

  /* Mark the socket so it will listen for incoming connections */
  if (listen(sock, MAXPENDING) < 0)
    errorout("listen() failed");

  return sock;
}

struct client {
  struct spindly_phys *phys_server;
  struct spindly_stream *stream_client;
  socket_t sock;
};

static struct client cl; /* this should be turned into a linked list of struct
                            later, one for each connected client */

static int handleclient(struct client *clp)
{
  char buffer[256];
  int len;

  len = recv(clp->sock, buffer, sizeof(buffer), 0);

  fprintf(stderr, "recv() returned %d!\n", len);

  if (0 == len)
    /* connection is presumed to be dead */
    return -1;

  else if (len >= 0) {
    /* feed incoming data into spindly and request an immediate copy since
       we have the buffer on the stack. */
    struct spindly_demux demux;
    spindly_error_t spint = spindly_phys_incoming(clp->phys_server,
                                                  (unsigned char *)buffer,
                                                  len,
                                                  SPINDLY_INCOMING_COPY, NULL);
    if(spint != SPINDLYE_OK)
      return -1;

    /* demux the incoming data to figure out if there's anything to do (yet) */
    spint = spindly_phys_demux(clp->phys_server, &demux);

    if(spint != SPINDLYE_OK)
      return -1;

    if(demux.type == SPINDLY_DX_NONE)
      /* we haven't demux'ed enough for a message yet */
      return len;

    fprintf(stderr, "demux type: %x\n", demux.type);
  }

  return len;
}

int main(int argc, char *argv[])
{
  socket_t servsock;
  int maxfd;
  fd_set readfds;
  fd_set writefds;
  struct timeval seltimeout;
  int running = 1;

  /* initially we support only one client */
  socket_t clientsock = SOCKET_BAD;

  (void)argc;
  (void)argv;

  /* Create port socket */
  servsock = createserver(SERVER_PORT);

  /* Initialize maxfd for use by select() */
  maxfd = servsock;

  while (running) {
    int rc;
    FD_ZERO(&readfds);
    FD_ZERO(&writefds);
    FD_SET(STDIN_FILENO, &readfds);

    /* right now we only allow one client to connect, so when there's a client
       we don't listen for new connects */
    if (SOCKET_BAD != clientsock) {
      FD_SET(clientsock, &readfds);

      /* if there is anything to write, wait to send */
      FD_SET(clientsock, &writefds);
    }
    else
      FD_SET(servsock, &readfds);

    seltimeout.tv_sec =  TIMEOUT_SECS;
    seltimeout.tv_usec = 0;

    rc = select(maxfd + 1, &readfds, &writefds, NULL, &seltimeout);
    if (!rc)
      errorout("timeout expired!\n");
    else {
      if (FD_ISSET(STDIN_FILENO, &readfds)) {
        /* read stdin and pass as data to clients */
      }

      if (clientsock > 0) {
        if (FD_ISSET(clientsock, &readfds)) {
          /* client is readable */
          if (handleclient(&cl) < 0) {
            sclose(clientsock);
            clientsock = SOCKET_BAD;
          }
        }
        if (FD_ISSET(clientsock, &writefds)) {
          /* client is writeable */
        }
      }

      if (FD_ISSET(servsock, &readfds)) {
        printf("New client connects on port %d - assume fine TLS-NPN",
               SERVER_PORT);
        clientsock = acceptclient(servsock);

        /* create a spindly handle for the physical connection */
        cl.phys_server = spindly_phys_init(SPINDLY_SIDE_SERVER,
                                           SPINDLY_DEFAULT, NULL);
        cl.sock = clientsock; /* store the socket */
        maxfd = clientsock;
      }
    }
  }

  /* Close server socket */
  sclose(servsock);

  exit(0);
}
