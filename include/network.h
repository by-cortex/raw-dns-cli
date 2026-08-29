#ifndef NETWORK_H
#define NETWORK_H

#include <netinet/in.h>
#include <stdint.h>
#include <sys/time.h>
#include <sys/types.h>

#define TRANSACTION_ID 0x6767
#define FLAGS 0x0100
#define HEADER_OFFSET 12
#define QTYPE 0x0001
#define QCLASS 0x0001

#define PORT 53

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

int create_socket(struct timeval sock_timeout);
int send_query(uint32_t sock, uint8_t buffer[], const char domain[],
               const char dns_ip[]);
int parse_recv(uint8_t buffer[], ssize_t bytes_received);

#endif // NETWORK_H
