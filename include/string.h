#include <stdint.h>

#ifndef STRING_H
#define STRING_H

char *int_to_string(int64_t num);
char *hex_to_string(uint64_t value);

uint32_t str_len(const char *str);
void str_cat(char *dest, const char *src);

#endif // STRING_H