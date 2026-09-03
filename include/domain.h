#ifndef DOMAIN_H
#define DOMAIN_H

#include <stddef.h>
#include <stdint.h>

int write_domain_to_buffer(uint8_t buffer[], const char domain[], size_t ptr);
void read_domain_with_pointer(const uint8_t buffer[], char out_str[],
                              uint16_t ptr);
void skip_domain_name(const uint8_t buffer[], size_t *offset);

#endif // DOMAIN_H
