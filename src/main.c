#define _POSIX_C_SOURCE 200112L

#include "network.h"
#include "table_print.h"

#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
  uint8_t buffer[512] = {0};
  struct sockaddr_storage recv;
  socklen_t recv_len = sizeof(recv);

  if (argc != 2) {
    fprintf(stderr, "Usage: %s <domain name>\n", argv[0]);
    return 1;
  }

  int sock = send_query(buffer, argv[1], "1.1.1.1");
  if (sock < 0) {
    fprintf(stderr, "Error: Failed to send DNS query\n");
    return 1;
  }

  memset(&recv, 0, sizeof(recv));
  ssize_t bytes_received = recvfrom(sock, buffer, sizeof(buffer), 0,
                                    (struct sockaddr *)&recv, &recv_len);
  if (bytes_received <= 0) {
    perror("Error receiving DNS response");
    close(sock);
    return 1;
  }

  print_hex(buffer, bytes_received);
  printf("\n");

  if (parse_recv(buffer) <= 0) {
    printf("Domain name not found!\n");
  }

  close(sock);
  return 0;
}
