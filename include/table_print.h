#ifndef TABLE_PRINT_H
#define TABLE_PRINT_H

void print_table_header(void);
void print_table_row(const char *domain, const char *type, int ttl,
                     const char *ip);
void print_table_footer(void);

#endif // TABLE_PRINT_H
