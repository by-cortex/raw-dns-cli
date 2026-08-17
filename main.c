#define _POSIX_C_SOURCE 200112L

#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

int write_domain(uint8_t buffer[], char domain[], unsigned int pos);
void print_hex(uint8_t buffer[], ssize_t bytes_received);

int main(int argc, char *argv[]) {
  struct sockaddr_in dest;
  uint8_t buffer[512] = {0};
  unsigned int pos = 0;

  struct sockaddr_storage recv;
  unsigned int recv_len = sizeof(recv);

  buffer[pos++] = 0x67;
  buffer[pos++] = 0x67;

  buffer[pos++] = 0x01;
  pos++;
  buffer[pos++] = 0x00;
  buffer[pos++] = 0x01;

  pos = 12;
  pos = write_domain(buffer, "google.com", pos);

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

  // RECV

  memset(&recv, 0, sizeof(recv));
  ssize_t bytes_received = recvfrom(sock, buffer, sizeof(buffer), 0,
                                    (struct sockaddr *)&recv, &recv_len);

  if (recv.ss_family == AF_INET) {
    struct sockaddr_in *ipv4 = (struct sockaddr_in *)&recv;
    char ip_str[INET_ADDRSTRLEN];

    inet_ntop(AF_INET, &ipv4->sin_addr, ip_str, sizeof(ip_str));
  } else {
    struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)&recv;
    char ip_str[INET6_ADDRSTRLEN];

    inet_ntop(AF_INET, &ipv6->sin6_addr, ip_str, sizeof(ip_str));
  }

  print_hex(buffer, bytes_received);

  close(sock);
  return 0;
}

void print_hex(uint8_t buffer[], ssize_t bytes_received) {
  for (ssize_t i = 0; i < bytes_received; i++) {
    printf("%02X ", (unsigned char)buffer[i]);
    if ((i + 1) % 12 == 0 && i != 0)
      printf("\n");
  }
  if (bytes_received % 12 != 0)
    printf("\n");
}

int write_domain(uint8_t buffer[], char domain[], unsigned int pos) {
  unsigned int char_len = 0;
  unsigned int tmp = pos;

  pos++;
  for (int i = 0; domain[i] != '\0'; i++) {
    if (domain[i] != '.') {
      char_len++;
      buffer[pos++] = domain[i];
    } else {
      buffer[tmp] = char_len;
      char_len = 0;
      tmp = pos;
      pos++;
    }
  }
  buffer[tmp] = char_len;
  buffer[pos++] = 0x00;
  return pos;
}
