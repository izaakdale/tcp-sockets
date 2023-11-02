#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <memory.h>

#include "common.h"

#define SERVER_PORT 8080
char data_buffer[1024];

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
  server_addr.sin_port = htons(SERVER_PORT);
  server_addr.sin_addr.s_addr = INADDR_ANY;

  uint addrln = sizeof(struct sockaddr);

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

    select(master_socket_fd + 1, &readfds, NULL, NULL, NULL);

    if (FD_ISSET(master_socket_fd, &readfds))
    {
      printf("New connection request received\n");

      comm_socket_fd = accept(master_socket_fd, (struct sockaddr *)&client_addr, &addrln);

      if (comm_socket_fd < 0)
      {
        printf("client socket accept failure\n");
        exit(0);
      }

      printf("connection from %s:%u\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

      while (1)
      {
        memset(data_buffer, 0, sizeof(data_buffer));

        uint sent_recv_bytes = recvfrom(comm_socket_fd, (char *)&data_buffer, sizeof(data_buffer), 0, (struct sockaddr *)&client_addr, &addrln);
        printf("server recieved %d bytes from the client %s:%u\n", sent_recv_bytes, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

        if (sent_recv_bytes == 0)
        {
          close(comm_socket_fd);
          break;
        }

        request_struct_t *client_data = (request_struct_t *)data_buffer;

        if (client_data->a == 0 && client_data->b == 0)
        {
          close(comm_socket_fd);
          break;
        }

        response_struct_t result;
        result.c = client_data->a + client_data->b;

        sent_recv_bytes = sendto(comm_socket_fd, (char *)&result, sizeof(response_struct_t), 0, (struct sockaddr *)&client_addr, sizeof(struct sockaddr));

        printf("server sent %d bytes to client %s:%u\n", sent_recv_bytes, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
      }
    }
  }
}

int main(int argc, char **argv)
{
  startServer();
  return 0;
}
