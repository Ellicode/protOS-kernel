#include <stdint.h>

#ifndef STRING_H
#define STRING_H

int memcmp(const void*, const void*, size_t);
void* memcpy(void* __restrict, const void* __restrict, size_t);
void* memmove(void*, const void*, size_t);
void* memset(void*, int, size_t);
size_t strlen(const char*);
void strcat(char *dest, const char *src);

char *int_to_string(int64_t num);
char *hex_to_string(uint64_t value);

#endif // STRING_H