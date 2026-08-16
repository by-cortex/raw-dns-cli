#define _POSIX_C_SOURCE 200112L

#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
  struct sockaddr_in dest;
  char req[] = "Test";

  if (argc != 2) {
    printf("\nUsage: %s <domain name>\n", argv[0]);
    return 0;
  }

  memset(&dest, 0, sizeof(dest));
  dest.sin_family = AF_INET;
  dest.sin_port = htons(53);
  inet_pton(AF_INET, "1.1.1.1", &dest.sin_addr);

  int sock = socket(AF_INET, SOCK_DGRAM, 0);
  if (sock < 0) {
    perror("Socket creation failed");
    return 1;
  }

  if (sendto(sock, req, sizeof(req), 0, (struct sockaddr *)&dest,
             sizeof(dest)) < 0) {
    perror("Sendto failed");
    return 1;
  }

  close(sock);
  return 0;
}
