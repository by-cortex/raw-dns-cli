#ifndef TABLE_PRINT_H
#define TABLE_PRINT_H

#include <stdint.h>
#include <sys/types.h>

#define HEX_LINE_LENGTH 12

void print_table_header(void);
void print_table_row(const char *domain, const char *type, int ttl,
                     const char *ip);
void print_table_footer(void);
void print_hex(uint8_t buffer[], ssize_t bytes_received);

#endif // TABLE_PRINT_H
