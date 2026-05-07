#include <stddef.h>
#include <stdint.h>

// MEMORY RELATED FUNCTIONS ================================================

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

// STRING CONVERSION FUNCTIONS =============================================


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