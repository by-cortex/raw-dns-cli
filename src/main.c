#define _POSIX_C_SOURCE 200112L

#include "network.h"
#include "table_print.h"

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
  uint8_t buffer[512] = {0};
  struct sockaddr_storage recv;
  socklen_t recv_len = sizeof(recv);

  int timeout_sec = 2;
  int show_hex = 0;
  const char *dns_ip = "1.1.1.1";
  const char *domain = NULL;

  static struct option long_options[] = {
      {"hex", no_argument, 0, 'x'},
      {"help", no_argument, 0, 'h'},
      {"timeout", required_argument, 0, 't'},
      {0, 0, 0, 0},
  };

  int opt;
  while ((opt = getopt_long(argc, argv, "xht:", long_options, NULL)) != -1) {
    switch (opt) {
    case 'x':
      show_hex = 1;
      break;
    case 'h':
      print_help(argv[0]);
      return 1;
    case 't':
      timeout_sec = atoi(optarg);
      break;
    default:
      print_help(argv[0]);
      return 1;
    }
  }

  if (optind >= argc) {
    fprintf(stderr, "Error: Domain name is required\n");
    print_help(argv[0]);
    return 1;
  }
  if (timeout_sec <= 0) {
    fprintf(stderr, "Error: -t argument must be greater than 0\n");
    return 1;
  }
  domain = argv[optind];

  if (optind + 1 < argc) {
    dns_ip = argv[optind + 1];
  }

  ssize_t bytes_received = -1;

  struct timeval sock_timeout = {0};
  sock_timeout.tv_sec = timeout_sec;

  int max_attempts = 5;

  for (int attempt = 1; attempt <= max_attempts; attempt++) {

    int sock = send_query(buffer, domain, dns_ip);
    if (sock < 0) {
      fprintf(stderr, "Error: Failed to send DNS query\n");
      continue;
    }
    int sockopt = setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &sock_timeout,
                             sizeof(sock_timeout));

    if (sockopt < 0) {
      perror("Error: Failed to set socket timeout");
      close(sock);
      return 1;
    }
    memset(&recv, 0, sizeof(recv));
    recv_len = sizeof(recv);

    bytes_received = recvfrom(sock, buffer, sizeof(buffer), 0,
                              (struct sockaddr *)&recv, &recv_len);

    if (bytes_received < 0) {

      fprintf(stderr, "Attempt %d/%d failed\n", attempt, max_attempts);
      close(sock);
      if (attempt == max_attempts) {
        fprintf(stderr, "No response after %d attempts\n", attempt);

        return 1;
      }

      continue;
    }

    if (show_hex == 1) {
      print_hex(buffer, bytes_received);
      printf("\n");
    }

    if (parse_recv(buffer, bytes_received) <= 0) {
      printf("Domain name not found!\n");
    }

    close(sock);
    break;
  }
  return 0;
}
