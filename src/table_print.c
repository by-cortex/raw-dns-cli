#include "table_print.h"

#include <stdio.h>
#include <string.h>

void print_table_header(void) {
  printf(COLOR_GRAY "┌──────────────────────────┬───────┬─────────┬────────────"
                    "──────────────┐\n" COLOR_RESET);
  printf(COLOR_GRAY "│ " COLOR_BOLD "%-24.24s" COLOR_RESET COLOR_GRAY
                    " │ " COLOR_BOLD "%-5.5s" COLOR_RESET COLOR_GRAY
                    " │ " COLOR_BOLD "%-7.7s" COLOR_RESET COLOR_GRAY
                    " │ " COLOR_BOLD "%-24.24s" COLOR_RESET COLOR_GRAY
                    " │\n" COLOR_RESET,
         "DOMAIN", "TYPE", "TTL", "VALUE");
  printf(COLOR_GRAY "├──────────────────────────┼───────┼─────────┼────────────"
                    "──────────────┤\n" COLOR_RESET);
}

void print_table_row(const char *domain, const char *type, int ttl,
                     const char *ip) {
  char ttl_str[10];
  snprintf(ttl_str, sizeof(ttl_str), "%ds", ttl);

  const char *type_color = COLOR_RESET;
  if (strcmp(type, "A") == 0) {
    type_color = COLOR_GREEN;
  } else if (strcmp(type, "AAAA") == 0) {
    type_color = COLOR_BLUE;
  } else if (strcmp(type, "CNAME") == 0) {
    type_color = COLOR_YELLOW;
  }

  printf(COLOR_GRAY "│ " COLOR_RESET "%-24.24s" COLOR_GRAY " │ %s"
                    "%-5.5s" COLOR_RESET COLOR_GRAY " │ " COLOR_CYAN
                    "%-7.7s" COLOR_RESET COLOR_GRAY " │ " COLOR_RESET
                    "%-24.24s" COLOR_GRAY " │\n" COLOR_RESET,
         domain, type_color, type, ttl_str, ip);
}

void print_table_footer(void) {
  printf(COLOR_GRAY "└──────────────────────────┴───────┴─────────┴────────────"
                    "──────────────┘\n" COLOR_RESET);
}

void print_hex(uint8_t buffer[], ssize_t bytes_received) {
  printf(COLOR_GRAY);
  for (ssize_t i = 0; i < bytes_received; i++) {
    printf("%02X ", (unsigned char)buffer[i]);
    if ((i + 1) % HEX_LINE_LENGTH == 0 && i != 0)
      printf("\n");
  }
  if (bytes_received % HEX_LINE_LENGTH != 0)
    printf("\n");
  printf(COLOR_RESET);
}
