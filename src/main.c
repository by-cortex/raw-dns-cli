#define _POSIX_C_SOURCE 200112L

#include "args.h"
#include "network.h"
#include "table_print.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
  uint16_t id = 0;
  uint8_t buffer[512] = {0};
  struct sockaddr_storage recv;
  socklen_t recv_len = sizeof(recv);
  ssize_t bytes_received = -1;

  srand((unsigned int)time(NULL));

  struct options run_options;
  run_options.show_hex = 0;
  run_options.timeout_sec = 2;
  run_options.domain = NULL;
  run_options.dns_ip = "1.1.1.1";
  run_options.port = 53;

  if (check_args(argc, argv, &run_options) == 1) {
    return 1;
  }

  struct timeval sock_timeout = {0};
  sock_timeout.tv_sec = run_options.timeout_sec;
  int max_attempts = 5;

  int sock = create_socket(sock_timeout);

  for (int attempt = 1; attempt <= max_attempts; attempt++) {
    int result = send_query(sock, &id, buffer, run_options.domain,
                            run_options.dns_ip, run_options.port);
    if (result < 0) {
      fprintf(stderr, "Error: Failed to send DNS query\n");
      sleep(run_options.timeout_sec);
      continue;
    }

    memset(&recv, 0, sizeof(recv));
    memset(buffer, 0, sizeof(buffer));
    recv_len = sizeof(recv);

    bytes_received = recvfrom(sock, buffer, sizeof(buffer), 0,
                              (struct sockaddr *)&recv, &recv_len);

    if (bytes_received < 0) {
      fprintf(stderr, "Attempt %d/%d failed\n", attempt, max_attempts);

      if (attempt == max_attempts) {
        fprintf(stderr, "No response after %d attempts\n", attempt);
        close(sock);
        return 1;
      }
      continue;
    }

    if (run_options.show_hex == 1) {
      print_hex(buffer, bytes_received);
      printf("\n");
    }

    if (parse_recv(id, buffer, bytes_received) <= 0) {
      printf("Domain name not found!\n");
    }
    break;
  }

  close(sock);
  return 0;
}
