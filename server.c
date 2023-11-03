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
#define MAX_N_CLIENTS 32

int monitored_fd_set[MAX_N_CLIENTS];
static void initilise_fd_set()
{
  for (int i = 0; i < MAX_N_CLIENTS; i++)
  {
    monitored_fd_set[i] = -1;
  }
}
static void add_to_fd_set(int fd)
{
  for (int i = 0; i < MAX_N_CLIENTS; i++)
  {
    if (monitored_fd_set[i] == -1)
    {
      monitored_fd_set[i] = fd;
      break;
    }
  }
}
static void remove_from_monitored_fd_set(int fd)
{
  for (int i = 0; i < MAX_N_CLIENTS; i++)
  {
    if (monitored_fd_set[i] == fd)
    {
      monitored_fd_set[i] = -1;
      break;
    }
  }
}
static void reinit_readfds(fd_set *p_fd_set)
{
  FD_ZERO(p_fd_set);
  for (int i = 0; i < MAX_N_CLIENTS; i++)
  {
    if (monitored_fd_set[i] != -1)
    {
      FD_SET(monitored_fd_set[i], p_fd_set);
    }
  }
}
static int get_max_fd()
{
  int max = -1;
  for (int i = 0; i < MAX_N_CLIENTS; i++)
  {
    if (monitored_fd_set[i] > max)
    {
      max = monitored_fd_set[i];
    }
  }
  return max;
}

void startMuxServer()
{
  initilise_fd_set();

  int master_socket_fd = 0;
  if ((master_socket_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
  {
    printf("socket creation failed\n");
    exit(EXIT_FAILURE);
  }

  struct sockaddr_in server_addr;
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(SERVER_PORT);
  server_addr.sin_addr.s_addr = INADDR_ANY;

  struct sockaddr_in client_addr;

  uint addrln = sizeof(struct sockaddr_in);

  if ((bind(master_socket_fd, (struct sockaddr *)&server_addr, addrln)) < 0)
  {
    printf("bind failure\n");
    exit(EXIT_FAILURE);
  }

  if ((listen(master_socket_fd, 5)) < 0)
  {
    printf("listen failure\n");
    exit(EXIT_FAILURE);
  }
  add_to_fd_set(master_socket_fd);

  fd_set readfds;
  while (1)
  {
    reinit_readfds(&readfds);

    printf("Blocking on select system call\n");

    select(get_max_fd() + 1, &readfds, NULL, NULL, NULL);

    printf("Selected a file descriptor\n");

    if (FD_ISSET(master_socket_fd, &readfds))
    {
      printf("New connection request\n");
      int comm_socket_fd = accept(master_socket_fd, (struct sockaddr *)&client_addr, &addrln);
      if (comm_socket_fd < 0)
      {
        printf("client accept failure\n");
        exit(EXIT_FAILURE);
      }

      add_to_fd_set(comm_socket_fd);
      printf("Connection to client %s:%u accepted\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
    }
    else
    {
      for (int i = 0; i < MAX_N_CLIENTS; i++)
      {
        if (FD_ISSET(monitored_fd_set[i], &readfds))
        {
          int comm_socket_fd = monitored_fd_set[i];

          char data_buffer[1024];
          memset(data_buffer, 0, sizeof(data_buffer));

          int sent_recv_bytes = recvfrom(comm_socket_fd, &data_buffer, sizeof(data_buffer), 0, (struct sockaddr *)&client_addr, &addrln);

          if (sent_recv_bytes == 0)
          {
            close(comm_socket_fd);
            remove_from_monitored_fd_set(comm_socket_fd);
            break;
          }

          request_struct_t *client_data = (request_struct_t *)data_buffer;

          if (client_data->a == 0 && client_data->b == 0)
          {
            close(comm_socket_fd);
            remove_from_monitored_fd_set(comm_socket_fd);
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
}

int main(int argc, char const *argv[])
{
  startMuxServer();
  return 0;
}
