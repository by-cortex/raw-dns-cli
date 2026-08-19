#include "table_print.h"

#include <stdio.h>

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

void print_hex(uint8_t buffer[], ssize_t bytes_received) {
  for (ssize_t i = 0; i < bytes_received; i++) {
    printf("%02X ", (unsigned char)buffer[i]);
    if ((i + 1) % HEX_LINE_LENGTH == 0 && i != 0)
      printf("\n");
  }
  if (bytes_received % HEX_LINE_LENGTH != 0)
    printf("\n");
}
