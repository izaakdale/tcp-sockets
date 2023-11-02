#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
// #include <netdb.h>
// #include <memory.h>
// #include <errno.h>

void startServer()
{
  int master_socket_fd = 0, comm_socket_fd = 0, opt = 1;
  fd_set readfds;

  struct sockaddr_in server_addr, client_addr;

  if ((master_socket_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
  {
    printf("failed to create socket\n");
    exit(EXIT_FAILURE);
  }

  if (setsockopt(master_socket_fd, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0)
  {
    printf("TCP socket creation failed for multiple connections\n");
    exit(EXIT_FAILURE);
  }

  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(8080);
  server_addr.sin_addr.s_addr = INADDR_ANY;

  int addrln = sizeof(struct sockaddr);

  if (bind(master_socket_fd, (struct sockaddr *)&server_addr, addrln) < 0)
  {
    printf("socket bind failed\n");
    exit(EXIT_FAILURE);
  }

  if (listen(master_socket_fd, 5) < 0)
  {
    printf("listen failed\n");
    exit(EXIT_FAILURE);
  }

  while (1)
  {
    FD_ZERO(&readfds);
    FD_SET(master_socket_fd, &readfds);
  }
}

int main(int argc, char **argv)
{
  startServer();
  return 0;
}
