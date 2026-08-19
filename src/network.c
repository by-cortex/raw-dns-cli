#include "network.h"
#include "domain.h"
#include "table_print.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

int send_query(uint8_t buffer[], const char domain[], const char dns_ip[]) {
  struct sockaddr_in dest;
  size_t pos = 0;

  uint16_t id = htons(TRANSACTION_ID);
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
  dest.sin_port = htons(PORT);
  inet_pton(AF_INET, dns_ip, &dest.sin_addr);

  int sock = socket(AF_INET, SOCK_DGRAM, 0);
  if (sock < 0) {
    perror("Socket creation failed");
    return -1;
  }

  if (sendto(sock, buffer, pos, 0, (struct sockaddr *)&dest, sizeof(dest)) <
      0) {
    perror("Sendto failed");
    return -1;
  }

  return sock;
}

int parse_recv(uint8_t buffer[]) {
  size_t offset = 6;
  struct dns_record rec;
  char ip_str[INET6_ADDRSTRLEN];
  uint16_t ancount = (buffer[offset] << 8) | buffer[offset + 1];

  offset = 12;

  skip_name(buffer, &offset);

  offset += 4;

  print_table_header();
  for (int i = 0; i < ancount; i++) {
    rec.name = (buffer[offset] << 8) | buffer[offset + 1];
    skip_name(buffer, &offset);

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

    char domain_str[256];
    read_domain_with_pointer(buffer, domain_str, rec.name);

    char type_str[5];
    if (rec.type == 1)
      strcpy(type_str, "A");
    else if (rec.type == 28)
      strcpy(type_str, "AAAA");
    print_table_row(domain_str, type_str, rec.ttl, ip_str);
  }
  print_table_footer();

  return ancount;
}
