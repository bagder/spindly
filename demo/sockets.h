#include "spdy_config.h"

#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif

#ifdef WIN32
typedef SOCKET socket_t;
#define SOCKET_BAD ~0
#define sclose(x) closesocket(x)
#else
typedef int socket_t;
#define SOCKET_BAD -1
#define sclose(x) close(x)
#endif

#define errorout(x) do { fputs(x, stderr); exit(1); } while(0)

#define SERVER_PORT 9999

