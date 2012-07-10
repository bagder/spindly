/*
 * Copyright (C) 2012 by Daniel Stenberg.
 *
 * spdy-client.
 *
 * This is an implementation of a simple SPDY client using the spindly
 * library.
 *
 */

#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <spindly.h>

#include "sockets.h"

int main(int argc, char *argv[])
{
  socket_t sock;
  struct sockaddr_in servaddr;
  char *server;
  int rc;
  char bye[3];
  int len;
  struct spindly_phys *phys_client;
  struct spindly_stream *stream_client;
  spindly_error_t spint;
  unsigned char *data;
  size_t datalen;

  server = "127.0.0.1";

  sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (sock == SOCKET_BAD)
    errorout("socket() failed");

  /* create a spindly handle for the physical connection */
  phys_client = spindly_phys_init(SPINDLY_SIDE_CLIENT, SPINDLY_DEFAULT, NULL);

  memset(&servaddr, 0, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = inet_addr(server);
  servaddr.sin_port = htons(SERVER_PORT);

  /* Establish the connection to the echo server */
  rc = connect(sock, (struct sockaddr *) &servaddr, sizeof(servaddr));
  if (rc < 0)
    errorout("connect() failed");

  printf("Connected! Pretend TLS-NPN succeeded.\n");

  /* create a new stream on the physical connection */
  spint = spindly_stream_new(phys_client, 0, &stream_client, NULL, NULL);

  /* get data to send over the socket */
  spint = spindly_phys_outgoing(phys_client, &data, &datalen);

  printf("Ask for a new stream\n");

  /* send away the SPDY packet */
  rc = send(sock, data, datalen, 0);

  if(rc > 0) {
    /* tell spindly how much of that data that was actually sent */
    spindly_phys_sent(phys_client, rc);
    printf("Send %d bytes\n", rc);
  }

  /* now wait for data to arrive on the socket and demux it to figure out
     what the peer says to us */
  sleep(5);

  sclose(sock);

  return 0;
}
