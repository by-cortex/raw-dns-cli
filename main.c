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

int write_domain(uint8_t buffer[], char domain[], size_t pos);
void read_domain_pointer(uint8_t buffer[], uint16_t ptr, char out_str[]);
void print_hex(uint8_t buffer[], ssize_t bytes_received);
int dns_request(uint8_t buffer[], char domain[], char dns_ip[]);
int parse_recv(uint8_t buffer[]);
void skip_name(uint8_t buffer[], size_t *offset);

void print_table_header(void);
void print_table_row(const char *domain, const char *type, int ttl,
                     const char *ip);
void print_table_footer(void);

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
  uint8_t buffer[512] = {0};
  struct sockaddr_storage recv;
  unsigned int recv_len = sizeof(recv);

  if (argc != 2) {
    printf("\nUsage: %s <domain name>\n", argv[0]);
    return 0;
  }

  int sock = dns_request(buffer, argv[1], "1.1.1.1");

  memset(&recv, 0, sizeof(recv));
  ssize_t bytes_received = recvfrom(sock, buffer, sizeof(buffer), 0,
                                    (struct sockaddr *)&recv, &recv_len);

  print_hex(buffer, bytes_received);
  printf("\n");
  if (parse_recv(buffer) <= 0) {
    printf("Domain name not found!\n");
  }

  close(sock);
  return 0;
}

int dns_request(uint8_t buffer[], char domain[], char dns_ip[]) {
  struct sockaddr_in dest;
  size_t pos = 0;

  buffer[pos++] = 0x67;
  buffer[pos++] = 0x67;

  buffer[pos++] = 0x01;
  pos++;
  buffer[pos++] = 0x00;
  buffer[pos++] = 0x01;

  pos = 12;
  pos = write_domain(buffer, domain, pos);

  pos++;
  buffer[pos++] = 0x01;
  buffer[pos++] = 0x00;
  buffer[pos++] = 0x01;

  memset(&dest, 0, sizeof(dest));
  dest.sin_family = AF_INET;
  dest.sin_port = htons(53);
  inet_pton(AF_INET, dns_ip, &dest.sin_addr);

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
    read_domain_pointer(buffer, rec.name, domain_str);

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

void skip_name(uint8_t buffer[], size_t *offset) {
  while (buffer[*offset] != 0x00) {
    if ((buffer[*offset] & 0xC0) == 0xC0) {
      *offset += 2;
      return;
    }
    *offset += buffer[*offset] + 1;
  }
  *offset += 1;
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

int write_domain(uint8_t buffer[], char domain[], size_t pos) {
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

void read_domain_pointer(uint8_t buffer[], uint16_t ptr, char out_str[]) {
  int curr_ptr = 0;
  int len = 0;

  uint16_t ptr_offset = ptr & 0x3FFF;
  int src_ptr = ptr_offset;

  while (buffer[src_ptr] != 0x00) {
    len = buffer[src_ptr];

    if (curr_ptr != 0)
      out_str[curr_ptr++] = '.';

    src_ptr++;

    for (int i = 0; i < len; i++) {
      out_str[curr_ptr++] = buffer[src_ptr++];
    }
  }
  out_str[curr_ptr] = '\0';
}

void print_table_header(void) {
  printf("┌─────────────────┬──────┬────────┬─────────────────┐\n");
  printf("│ %-15s │ %-4s │ %-6s │ %-15s │\n", "DOMAIN", "TYPE", "TTL", "VALUE");
  printf("├─────────────────┼──────┼────────┼─────────────────┤\n");
}

void print_table_row(const char *domain, const char *type, int ttl,
                     const char *ip) {
  char ttl_str[10];
  snprintf(ttl_str, sizeof(ttl_str), "%ds", ttl);
  printf("│ %-15s │ %-4s │ %-6s │ %-15s │\n", domain, type, ttl_str, ip);
}

void print_table_footer(void) {
  printf("└─────────────────┴──────┴────────┴─────────────────┘\n");
}
