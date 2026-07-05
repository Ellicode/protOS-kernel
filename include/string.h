#include <stddef.h>
#include <stdarg.h>
#include <stdint.h>

#ifndef STRING_H
#define STRING_H

int memcmp(const void*, const void*, size_t);
void* memcpy(void* __restrict, const void* __restrict, size_t);
void* memmove(void*, const void*, size_t);
void* memset(void*, int, size_t);
size_t strlen(const char*);
void strcat(char *dest, const char *src);
char *strtok(char *src, const char *delim);
char strcmp(const char * s1, const char * s2);
char* strcpy(char* dst, const char* src);
long strtol(const char *nptr, char **endptr, int base);
int strncmp(const char *s1, const char *s2, size_t n);
char *strncpy(char *dst, const char *src, size_t n);
char *strchr(const char *s, int c);
char *strrchr(const char *s, int c);

char *int_to_string(int64_t num);
char *hex_to_string(uint64_t value);
int atoi(const char *nptr);

int vsnprintf(char *str, size_t size, const char *format, va_list args);
int snprintf(char *str, size_t size, const char *format, ...);

#endif // STRING_H