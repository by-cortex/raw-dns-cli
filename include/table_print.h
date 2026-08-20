#ifndef TABLE_PRINT_H
#define TABLE_PRINT_H

#include <stdint.h>
#include <sys/types.h>

#define HEX_LINE_LENGTH 12

#define COLOR_RESET "\033[0m"
#define COLOR_BOLD "\033[1m"
#define COLOR_GRAY "\033[90m"
#define COLOR_GREEN "\033[32m"
#define COLOR_BLUE "\033[34m"
#define COLOR_YELLOW "\033[33m"
#define COLOR_CYAN "\033[36m"

void print_table_header(void);
void print_table_row(const char *domain, const char *type, int ttl,
                     const char *ip);
void print_table_footer(void);
void print_hex(uint8_t buffer[], ssize_t bytes_received);
void print_help(const char *prog_name);

#endif // TABLE_PRINT_H
