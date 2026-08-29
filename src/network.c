#include "network.h"
#include "domain.h"
#include "table_print.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#define MIN(a, b) ((a) < (b) ? (a) : (b))

int create_socket(struct timeval sock_timeout) {
  int sock = socket(AF_INET, SOCK_DGRAM, 0);
  if (sock < 0) {
    perror("Socket creation failed");
    return 1;
  }

  int sockopt = setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &sock_timeout,
                           sizeof(sock_timeout));

  if (sockopt < 0) {
    perror("Error: Failed to set socket timeout");
    close(sock);
    return 1;
  }

  return sock;
}

int send_query(uint32_t sock, uint8_t buffer[], const char domain[],
               const char dns_ip[], int port) {
  struct sockaddr_in dest;
  size_t pos = 0;

  uint16_t id = ntohl(TRANSACTION_ID);
  memcpy(&buffer[0], &id, 2);

  uint16_t flags = htons(FLAGS);
  memcpy(&buffer[2], &flags, 2);

  uint16_t qdcount = htons(1);
  memcpy(&buffer[4], &qdcount, 2);

  pos = write_domain_to_buffer(buffer, domain, HEADER_OFFSET);

  uint16_t qtype = htons(QTYPE);
  memcpy(&buffer[pos], &qtype, 2);
  pos += 2;

  uint16_t qclass = htons(QCLASS);
  memcpy(&buffer[pos], &qclass, 2);
  pos += 2;

  memset(&dest, 0, sizeof(dest));
  dest.sin_family = AF_INET;
  dest.sin_port = htons(port);
  inet_pton(AF_INET, dns_ip, &dest.sin_addr);

  if (sendto(sock, buffer, pos, 0, (struct sockaddr *)&dest, sizeof(dest)) <
      0) {
    perror("Sendto failed");
    return -1;
  }

  return 0;
}

int parse_recv(uint8_t buffer[], ssize_t bytes_received) {
  size_t offset = 6;
  struct dns_record rec;
  char domain_str[256];

  if (bytes_received < 12) {
    fprintf(stderr, "Error: Packet too short\n");
    return -1;
  }

  uint16_t ancount = (buffer[offset] << 8) | buffer[offset + 1];
  if (ancount <= 0)
    return 0;

  offset = 12;

  skip_domain_name(buffer, &offset);

  offset += 4;

  print_table_header();
  for (int i = 0; i < ancount; i++) {
    char ip_str[INET6_ADDRSTRLEN] = "-";
    char type_str[10] = "OTHER";

    rec.name = (buffer[offset] << 8) | buffer[offset + 1];
    skip_domain_name(buffer, &offset);

    if (offset + 10 > (size_t)bytes_received)
      break;

    rec.type = (buffer[offset] << 8) | buffer[offset + 1];
    offset += 2;
    rec.class = (buffer[offset] << 8) | buffer[offset + 1];
    offset += 2;

    memcpy(&rec.ttl, &buffer[offset], 4);
    rec.ttl = ntohl(rec.ttl);
    offset += 4;

    rec.rdlen = (buffer[offset] << 8) | buffer[offset + 1];
    offset += 2;
    if (offset + rec.rdlen > (size_t)bytes_received)
      break;

    size_t rdata_offset = offset;
    memcpy(&rec.rdata.raw, &buffer[offset], MIN(rec.rdlen, 16));
    offset += rec.rdlen;

    if (rec.type == 1 && rec.rdlen == 4) {
      inet_ntop(AF_INET, &rec.rdata.v4, ip_str, sizeof(ip_str));
      strcpy(type_str, "A");
    } else if (rec.type == 28 && rec.rdlen == 16) {
      inet_ntop(AF_INET6, &rec.rdata.v6, ip_str, sizeof(ip_str));
      strcpy(type_str, "AAAA");
    } else if (rec.type == 5) {
      read_domain_with_pointer(buffer, ip_str, rdata_offset);
      strcpy(type_str, "CNAME");
    }

    read_domain_with_pointer(buffer, domain_str, rec.name);

    print_table_row(domain_str, type_str, rec.ttl, ip_str);
  }
  print_table_footer();

  if (offset > (size_t)bytes_received) {
    fprintf(stderr, "Error: Out of bytes_received in parsing module\n");
    return -1;
  }

  return ancount;
}
