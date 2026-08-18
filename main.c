#define _POSIX_C_SOURCE 200112L

#include <arpa/inet.h>
#include <netinet/in.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define HEX_LINE_LENGTH 12

int write_domain(uint8_t buffer[], char domain[], unsigned int pos);
void print_hex(uint8_t buffer[], ssize_t bytes_received);
void parse_recv(uint8_t buffer[]);

struct dns_record {
  uint16_t name;
  uint16_t type;
  uint16_t class;
  uint32_t ttl;
  uint16_t rdlen;

  union {
    struct in_addr v4;
    struct in6_addr v6;
    uint8_t raw[16];
  } rdata;
};

int main(int argc, char *argv[]) {
  struct sockaddr_in dest;
  uint8_t buffer[512] = {0};
  unsigned int pos = 0;

  struct sockaddr_storage recv;
  unsigned int recv_len = sizeof(recv);

  if (argc != 2) {
    printf("\nUsage: %s <domain name>\n", argv[0]);
    return 0;
  }

  buffer[pos++] = 0x67;
  buffer[pos++] = 0x67;

  buffer[pos++] = 0x01;
  pos++;
  buffer[pos++] = 0x00;
  buffer[pos++] = 0x01;

  pos = 12;
  pos = write_domain(buffer, argv[1], pos);

  buffer[pos++] = 0x00;
  buffer[pos++] = 0x01;
  buffer[pos++] = 0x00;
  buffer[pos++] = 0x01;

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
  parse_recv(buffer);

  close(sock);
  return 0;
}

void parse_recv(uint8_t buffer[]) {
  size_t offset = 6; // ANCOUNT index
  uint16_t ancount = (buffer[offset] << 8) | buffer[offset + 1];
  struct dns_record rec;
  char ip_str[INET6_ADDRSTRLEN];
  offset = 12; // headers end index

  while (buffer[offset] != 0x00) {
    offset += buffer[offset] + 1;
  }
  offset++;

  offset += 4;

  for (int i = 0; i < ancount; i++) {
    rec.name = (buffer[offset] << 8) | buffer[offset + 1];
    if ((buffer[offset] & 0xC0) == 0xC0) // TODO: skip name if not pointer
      offset += 2;

    rec.type = (buffer[offset] << 8) | buffer[offset + 1];
    offset += 2;
    rec.class = (buffer[offset] << 8) | buffer[offset + 1];
    offset += 2;

    memcpy(&rec.ttl, &buffer[offset], 4);
    rec.ttl = htonl(rec.ttl);
    offset += 4;

    rec.rdlen = (buffer[offset] << 8) | buffer[offset + 1];
    offset += 2;

    memcpy(&rec.rdata.raw, &buffer[offset], rec.rdlen);
    offset += rec.rdlen;

    if (rec.type == 1 && rec.rdlen == 4) {
      inet_ntop(AF_INET, &rec.rdata.v4, ip_str, sizeof(ip_str));
    } else if (rec.type == 28 && rec.rdlen == 16) {
      inet_ntop(AF_INET6, &rec.rdata.v6, ip_str, sizeof(ip_str));
    }

    printf("name: %d type: %d class: %d ttl: %d rdlen: %d\nIP: %s\n", rec.name,
           rec.type, rec.class, rec.ttl, rec.rdlen, ip_str);
  }
}

void print_hex(uint8_t buffer[], ssize_t bytes_received) {
  for (ssize_t i = 0; i < bytes_received; i++) {
    printf("%02X ", (unsigned char)buffer[i]);
    if ((i + 1) % HEX_LINE_LENGTH == 0 && i != 0)
      printf("\n");
  }
  if (bytes_received % HEX_LINE_LENGTH != 0)
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
