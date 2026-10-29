#include "network.h"
#include "domain.h"
#include "table_print.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
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

int serialize_message(uint16_t *id, uint8_t buffer[], size_t buffer_size,
                      const char domain[]) {
  if (buffer_size < 12) {
    return -1;
  }

  size_t pos = 0;

  *id = htons(get_random_id());
  memcpy(&buffer[0], id, 2);

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

  return pos;
}

int send_query(uint32_t sock, uint8_t buffer[], size_t buffer_size,
               const char dns_ip[], int port) {
  struct sockaddr_in dest;
  memset(&dest, 0, sizeof(dest));
  dest.sin_family = AF_INET;
  dest.sin_port = htons(port);
  inet_pton(AF_INET, dns_ip, &dest.sin_addr);

  if (sendto(sock, buffer, buffer_size, 0, (struct sockaddr *)&dest,
             sizeof(dest)) < 0) {
    perror("Sendto failed");
    return -1;
  }

  return 0;
}

int tcp_fallback(uint16_t query_id, const char dns_ip[], int port,
                 uint8_t *buffer, size_t buffer_size) {
  struct sockaddr_in dest;
  memset(&dest, 0, sizeof(dest));
  dest.sin_family = AF_INET;
  dest.sin_port = htons(port);
  inet_pton(AF_INET, dns_ip, &dest.sin_addr);

  int sock_fd = socket(AF_INET, SOCK_STREAM, 0);

  if (connect(sock_fd, (struct sockaddr *)&dest, sizeof(dest)) != 0) {
    fprintf(stderr, "TCP Fallback failed to connect server\n");
    return -1;
  }

  uint16_t net_len = htons((uint16_t)buffer_size);
  if (send(sock_fd, &net_len, sizeof(net_len), 0) < 0) {
    perror("Failed send len TCP segment");
    close(sock_fd);
    return -1;
  }
  if (send(sock_fd, buffer, buffer_size, 0) < 0) {
    perror("Failed send len TCP segment");
    close(sock_fd);
    return -1;
  }

  uint16_t resp_len = 0;
  recv(sock_fd, &resp_len, 2, MSG_WAITALL);
  resp_len = ntohs(resp_len);

  uint8_t recv_buffer[4096] = {0};
  ssize_t bytes_received = recv(sock_fd, recv_buffer, resp_len, MSG_WAITALL);
  if (bytes_received < 0) {
    perror("Failed with recv TCP");
    close(sock_fd);
    return -1;
  }
  print_hex(recv_buffer, bytes_received);
  parse_recv(query_id, recv_buffer, bytes_received);

  close(sock_fd);
  return bytes_received;
}

int parse_recv(uint16_t query_id, uint8_t buffer[], ssize_t bytes_received) {
  size_t offset = 0;
  struct dns_record rec;
  char domain_str[256];

  if (bytes_received < 12) {
    fprintf(stderr, "Error: Packet too short\n");
    return -1;
  }

  uint16_t answer_id = (buffer[offset] << 8) | buffer[offset + 1];
  if (ntohs(answer_id) != query_id) {
    return 0;
  }
  offset = 6;

  uint16_t ancount = (buffer[offset] << 8) | buffer[offset + 1];
  if (ancount == 0)
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
    // rec.class = (buffer[offset] << 8) | buffer[offset + 1];
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

uint16_t get_random_id(void) {
  return (uint16_t)((rand() & 0xFF) | ((rand() & 0xFF) << 8));
}
