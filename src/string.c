#include <stdarg.h>

#include "string.h"

// MEMORY RELATED FUNCTIONS ====================================================================

void* memmove(void* dstptr, const void* srcptr, size_t size) {
	unsigned char* dst = (unsigned char*) dstptr;
	const unsigned char* src = (const unsigned char*) srcptr;
	if (dst < src) {
		for (size_t i = 0; i < size; i++) {
			dst[i] = src[i];
        }
	} else {
		for (size_t i = size; i != 0; i--) {
			dst[i-1] = src[i-1];
        }
	}
	return dstptr;
}

int memcmp(const void* aptr, const void* bptr, size_t size) {
	const unsigned char* a = (const unsigned char*) aptr;
	const unsigned char* b = (const unsigned char*) bptr;
	for (size_t i = 0; i < size; i++) {
		if (a[i] < b[i]) {
			return -1;
        } else if (b[i] < a[i]) {
			return 1;
        }
	}
	return 0;
}

void* memset(void* bufptr, int value, size_t size) {
	unsigned char* buf = (unsigned char*) bufptr;
	for (size_t i = 0; i < size; i++) {
		buf[i] = (unsigned char) value;
    }
	return bufptr;
}


void* memcpy(void* restrict dstptr, const void* restrict srcptr, size_t size) {
	unsigned char* dst = (unsigned char*) dstptr;
	const unsigned char* src = (const unsigned char*) srcptr;
	for (size_t i = 0; i < size; i++) {
		dst[i] = src[i];
    }
	return dstptr;
}

size_t strlen(const char* str) {
	size_t len = 0;
	while (str[len]) {
		len++;
    }
	return len;
}

void strcat(char *dest, const char *src)
{
    int dest_len = strlen(dest);
    int i = 0;
    while (src[i] != '\0')
    {
        dest[dest_len + i] = src[i];
        i++;
    }
    dest[dest_len + i] = '\0';
}

char strcmp(const char * s1, const char * s2) {
    for (size_t i = 0; s1[i] != '\0' || s2[i] != '\0'; i++) {
        if (((char*)s1)[i] != ((char*)s2)[i]) {
            if (((char*)s1)[i] < ((char*)s2)[i]) {
                return -1;
            }
            else {
                return 1;
            }
        }
    }
    return 0;
}

char *strtok(char *src, const char *delim) {
    static char * last_tok = NULL;
    if (src != NULL) {
        last_tok = src;
    }
    if (last_tok == NULL) { 
        return NULL;
    }

    // nothing more to parse
    if (*last_tok == '\0') {
        return NULL;
    }

    // find the next token
    for (size_t i = 0; last_tok[i] != '\0'; i++) {
        for (int j = 0; delim[j] != '\0'; j++) {
            if (last_tok[i] == delim[j]) {
                goto fail;
            }
        }
        last_tok += i;
        break;
        fail:
        continue;
    }

    // end the next token
    size_t i = 0;
    for (i = 0; last_tok[i] != '\0'; i++) {
        for (int j = 0; delim[j] != '\0'; j++) {
            if (last_tok[i] == delim[j]) {
                goto found;
            }
        }
    }
    found:
    last_tok[i] = '\0';

    char * ret = last_tok;
    last_tok += i + 1;
    return ret;
}

char* strcpy(char* dst, const char* src) {
    const size_t length = strlen(src);
    memcpy(dst, src, length + 1);
    return dst;
}

long strtol(const char *nptr, char **endptr, int base) {
    const char *s = nptr;
    unsigned long acc = 0;
    int c;
    uint8_t neg = 0;
    uint8_t any = 0;

    // 1. Skip whitespace
    while ((c = *s) == ' ' || (c == '\t') || (c == '\n') || 
           (c == '\v') || (c == '\f') || (c == '\r')) {
        s++;
    }

    // 2. Handle optional sign
    if (*s == '-') {
        neg = 1;
        s++;
    } else if (*s == '+') {
        s++;
    }

    // 3. Auto-detect base if 0
    if (base == 0) {
        if (*s == '0' && (s[1] == 'x' || s[1] == 'X')) {
            s += 2;
            base = 16;
        } else if (*s == '0') {
            s++;
            base = 8;
        } else {
            base = 10;
        }
    } else if (base == 16 && *s == '0' && (s[1] == 'x' || s[1] == 'X')) {
        s += 2;
    }

    // 4. Conversion loop
    for (c = *s; ; c = *++s) {
        int val = -1;
        if (c >= '0' && c <= '9') {
            val = c - '0';
        } else if (c >= 'a' && c <= 'z') {
            val = c - 'a' + 10;
        } else if (c >= 'A' && c <= 'Z') {
            val = c - 'A' + 10;
        }

        if (val < 0 || val >= base) {
            break;
        }

        any = 1;
        acc = acc * base + val;
    }

    // 5. Apply sign
    long res = (long)acc;
    if (neg) {
        res = -res;
    }

    // 6. Set endptr
    if (endptr != NULL) {
        *endptr = (char *)(any ? s : nptr);
    }

    return res;
}

int strncmp(const char *s1, const char *s2, size_t n) {
    while (n > 0) {
        if (*s1 != *s2) {
            return (unsigned char)*s1 - (unsigned char)*s2;
        }
        if (*s1 == '\0') {
            return 0;
        }
        s1++;
        s2++;
        n--;
    }
    return 0;
}

char *strncpy(char *dst, const char *src, size_t n) {
    if (n != 0) {
        char *d = dst;
        const char *s = src;

        // Copy characters from src to dst until 'n' is reached or src ends
        do {
            if ((*d++ = *s++) == '\0') {
                // If src ended early, pad the remaining 'n' bytes with zeros
                while (--n != 0) {
                    *d++ = '\0';
                }
                break;
            }
        } while (--n != 0);
    }
    return dst;
}

char *strchr(const char *s, int c) {
    while (*s != '\0') {
        if (*s == (char)c) {
            return (char *)s;
        }
        s++;
    }

    if (c == '\0') {
        return (char *)s;
    }

    return NULL;
}

char *strrchr(const char *s, int c) {
    char *last_match = NULL;
    while (*s != '\0') {
        if (*s == (char)c) {
            last_match = (char *)s;
        }
        s++;
    }
    if (c == '\0') {
        return (char *)s;
    }

    return last_match;
}


// STRING CONVERSION FUNCTIONS =================================================================

char *int_to_string(int64_t num)
{
    static char str[12]; // Enough to hold -2147483648 and null terminator
    int i = 0;
    int isNegative = 0;
    if (num < 0)
    {
        isNegative = 1;
        num = -num;
    } // Handle negative
    do
    {
        str[i++] = (num % 10) + '0'; // Get digit and convert to ASCII
    } while ((num /= 10) > 0);
    if (isNegative)
    {
        str[i++] = '-';
    }
    str[i] = '\0'; // Null-terminate
    // Reverse the string (i is the length)
    for (int j = 0; j < i / 2; j++)
    {
        char temp = str[j];
        str[j] = str[i - j - 1];
        str[i - j - 1] = temp;
    }
    return str;
}

char *hex_to_string(uint64_t value)
{
    static char str[19]; // "0x" + 16 hex digits + null

    const char *hex_digits = "0123456789ABCDEF";

    str[0] = '0';
    str[1] = 'x';

    if (value == 0)
    {
        str[2] = '0';
        str[3] = '\0';
        return str;
    }

    char temp[16];
    size_t i = 0;

    while (value != 0)
    {
        temp[i++] = hex_digits[value & 0xF];
        value >>= 4; // logical shift on unsigned type, always terminates
    }

    for (size_t j = 0; j < i; j++)
    {
        str[2 + j] = temp[i - 1 - j];
    }

    str[2 + i] = '\0';
    return str;
}

int atoi(const char *nptr)
{
    // Converts base 10 string to a long, then casts it to an int
    return (int)strtol(nptr, (char **)NULL, 10);
}

int vsnprintf(char *str, size_t size, const char *format, va_list args) {
    if (str == NULL || size == 0) {
        return 0;
    }

    size_t written = 0;

    for (size_t i = 0; format[i] != '\0'; i++) {
        // If we are out of space (leaving room for '\0'), stop writing but keep counting 
        // to return the correct standard total length.
        if (format[i] == '%') {
            i++;

            char pad = ' ';
            int width = 0;

            if (format[i] == '0') {
                pad = '0';
                i++;
            }

            while (format[i] >= '0' && format[i] <= '9') {
                width = width * 10 + (format[i] - '0');
                i++;
            }

            char *arg_str = NULL;
            char static_buf[19]; // Fallback if conversion strings aren't thread-safe

            if (format[i] == 'd') {
                int value = va_arg(args, int);
                arg_str = int_to_string(value);
            } 
            else if (format[i] == 'x') {
                uint64_t value = va_arg(args, uint64_t);
                arg_str = hex_to_string(value);
            } 
            else if (format[i] == 's') {
                arg_str = va_arg(args, char *);
                if (arg_str == NULL) {
                    arg_str = "(null)";
                }
            } 
            else if (format[i] == '%') {
                if (written < size - 1) {
                    str[written] = '%';
                }
                written++;
                continue;
            }

            if (arg_str != NULL) {
                int len = strlen(arg_str);
                
                // Process padding width
                while (len < width) {
                    if (written < size - 1) {
                        str[written] = pad;
                    }
                    written++;
                    width--;
                }

                // Copy token contents
                while (*arg_str) {
                    if (written < size - 1) {
                        str[written] = *arg_str;
                    }
                    written++;
                    arg_str++;
                }
            }
        } else {
            if (written < size - 1) {
                str[written] = format[i];
            }
            written++;
        }
    }

    // Always null-terminate safely within limits
    if (written < size) {
        str[written] = '\0';
    } else {
        str[size - 1] = '\0';
    }

    return (int)written;
}

int snprintf(char *str, size_t size, const char *format, ...) {
    va_list args;
    va_start(args, format);
    
    int result = vsnprintf(str, size, format, args);
    
    va_end(args);
    return result;
}