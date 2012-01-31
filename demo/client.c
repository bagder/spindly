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

#include "sockets.h"

int main(int argc, char *argv[])
{
  socket_t sock;
  struct sockaddr_in servaddr;
  char *server;
  int rc;
  char bye[3];
  int len;

  server = "127.0.0.1";

  sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (sock == SOCKET_BAD)
    errorout("socket() failed");

  memset(&servaddr, 0, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = inet_addr(server);
  servaddr.sin_port = htons(SERVER_PORT);

  /* Establish the connection to the echo server */
  rc = connect(sock, (struct sockaddr *) &servaddr, sizeof(servaddr));
  if (rc < 0)
    errorout("connect() failed");

  printf("Connected!\n");
  write(sock, "not spdy", strlen("not spdy"));
  len = read(sock, bye, sizeof(bye));
  printf("Recv: %.*s\n", 3, bye); 

  sclose(sock);

  return 0;
}
