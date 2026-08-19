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
