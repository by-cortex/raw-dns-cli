#include "domain.h"

int write_domain_to_buffer(uint8_t buffer[], const char domain[], size_t ptr) {
  unsigned int char_len = 0;
  unsigned int tmp = ptr;

  ptr++;
  for (int i = 0; domain[i] != '\0'; i++) {
    if (domain[i] != '.') {
      char_len++;
      buffer[ptr++] = domain[i];
    } else {
      buffer[tmp] = char_len;
      char_len = 0;
      tmp = ptr;
      ptr++;
    }
  }
  buffer[tmp] = char_len;
  buffer[ptr++] = 0x00;
  return ptr;
}

void read_domain_with_pointer(uint8_t buffer[], char out_str[], uint16_t ptr) {
  int curr_ptr = 0;
  int len = 0;
  int jump_count = 0;

  uint16_t ptr_offset = ptr & 0x3FFF;
  int src_ptr = ptr_offset;

  while (buffer[src_ptr] != 0x00) {
    if (jump_count > 10)
      break;

    if ((buffer[src_ptr] & 0xC0) == 0xC0) {
      src_ptr = ((buffer[src_ptr] & 0x3F) << 8) | buffer[src_ptr + 1];
      jump_count++;
      continue;
    }

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
