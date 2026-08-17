#define _POSIX_C_SOURCE 200112L

#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
  struct sockaddr_in dest;
  uint8_t buffer[512] = {0};
  unsigned int pos = 0;

  buffer[pos++] = 0x67;
  buffer[pos++] = 0x67;

  buffer[pos++] = 0x01;
  pos++;
  buffer[pos++] = 0x00;
  buffer[pos++] = 0x01;

  pos = 12;

  buffer[pos++] = 0x06;
  buffer[pos++] = 'g';
  buffer[pos++] = 'o';
  buffer[pos++] = 'o';
  buffer[pos++] = 'g';
  buffer[pos++] = 'l';
  buffer[pos++] = 'e';
  buffer[pos++] = 0x03;
  buffer[pos++] = 'c';
  buffer[pos++] = 'o';
  buffer[pos++] = 'm';
  buffer[pos++] = 0x00;

  buffer[pos++] = 0x00;
  buffer[pos++] = 0x01;
  buffer[pos++] = 0x00;
  buffer[pos++] = 0x01;

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

  if (sendto(sock, buffer, pos, 0, (struct sockaddr *)&dest, sizeof(dest)) <
      0) {
    perror("Sendto failed");
    return 1;
  }

  close(sock);
  return 0;
}
