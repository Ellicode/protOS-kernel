#include <stddef.h>
#include <stdint.h>

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

uint32_t str_len(const char *str)
{
    int len = 0;
    while (str[len] != '\0')
    {
        len++;
    }
    return len;
}

void str_cat(char *dest, const char *src)
{
    int dest_len = str_len(dest);
    int i = 0;
    while (src[i] != '\0')
    {
        dest[dest_len + i] = src[i];
        i++;
    }
    dest[dest_len + i] = '\0';
}