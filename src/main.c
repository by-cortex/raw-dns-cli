#define _POSIX_C_SOURCE 200112L

#include "network.h"
#include "table_print.h"

#include <getopt.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
  uint8_t buffer[512] = {0};
  struct sockaddr_storage recv;
  socklen_t recv_len = sizeof(recv);

  int show_hex = 0;
  const char *dns_ip = "1.1.1.1";
  const char *domain = NULL;

  static struct option long_options[] = {{"hex", no_argument, 0, 'x'},
                                         {"help", no_argument, 0, 'h'},
                                         {0, 0, 0, 0}};

  int opt;
  while ((opt = getopt_long(argc, argv, "xh", long_options, NULL)) != -1) {
    switch (opt) {
    case 'x':
      show_hex = 1;
      break;
    case 'h':
      print_help(argv[0]);
      return 1;
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

  domain = argv[optind];

  if (optind + 1 < argc) {
    dns_ip = argv[optind + 1];
  }

  int sock = send_query(buffer, domain, dns_ip);
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

  if (show_hex == 1) {
    print_hex(buffer, bytes_received);
    printf("\n");
  }

  if (parse_recv(buffer, bytes_received) <= 0) {
    printf("Domain name not found!\n");
  }

  close(sock);
  return 0;
}
