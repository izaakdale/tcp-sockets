#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include "common.h"

#define SERVER_PORT 8080
#define SERVER_ADDR "127.0.0.1"

request_struct_t req;
response_struct_t resp;

void setup_tcp_communication()
{
  int sock_fd = 0;
  uint addrln = sizeof(struct sockaddr);

  struct sockaddr_in dest;

  dest.sin_family = AF_INET;
  dest.sin_port = htons(SERVER_PORT);

  struct hostent *host = (struct hostent *)gethostbyname(SERVER_ADDR);
  dest.sin_addr = *((struct in_addr *)host->h_addr);

  sock_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  connect(sock_fd, (struct sockaddr *)&dest, addrln);

PROMPT_USER:
  printf("Enter value for a = ");
  scanf("%u", &req.a);
  printf("Enter value for b = ");
  scanf("%u", &req.b);

  int sent_recv_bytes = sendto(sock_fd, &req, sizeof(request_struct_t), 0, (struct sockaddr *)&dest, sizeof(struct sockaddr));

  printf("sent %d bytes to server\n", sent_recv_bytes);

  sent_recv_bytes = recvfrom(sock_fd, (char *)&resp, sizeof(response_struct_t), 0, (struct sockaddr *)&dest, &addrln);
  printf("received %d bytes from server\n", sent_recv_bytes);

  printf("Result of is %d\n", resp.c);

  goto PROMPT_USER;
}

int main(int argc, char const *argv[])
{
  setup_tcp_communication();
  return 0;
}
