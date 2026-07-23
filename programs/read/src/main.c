#include <proto/core.h>

enum DisplayMode {
    READ_CHAR,
    READ_HEX,
    READ_DECIMAL,
    READ_HEX_CHAR,
};

#define ROW_WIDTH 8

char *hex8_to_string_noprefix(uint64_t value)
{
    value &= 0xFF; // clamp to 1 byte

    static char str[3];
    const char *hex = "0123456789ABCDEF";

    str[0] = hex[(value >> 4) & 0xF];
    str[1] = hex[value & 0xF];
    str[2] = '\0';

    return str;
}


int pmain(char argv[16][64], int argc) {
    if (argc < 2) {
        fprintf(STDERR, "[ERROR] Too few arguments! Usage: `read {path} {-x|-d|-c|-xc}`\n");
        return 1;
    }

    char *path = argv[1];
    int mode;

    if (argc == 3) {
        char *mode_arg = argv[2];
        if (strcmp(mode_arg, "-c") == 0) { mode = READ_CHAR; }
        else if (strcmp(mode_arg, "-x") == 0) { mode = READ_HEX; }
        else if (strcmp(mode_arg, "-d") == 0) { mode = READ_DECIMAL; }
        else if (strcmp(mode_arg, "-xc") == 0) { mode = READ_HEX_CHAR; }
        else {
            fprintf(STDERR, "[ERROR] Unknown mode \"%s\".\n");
            return 1;
        }
    } else {
        mode = READ_CHAR;
    }

    int fd = open(path, "r");
    if (fd < PROTO_OK) {
        fprintf(STDERR, "[ERROR] Could not open file.\n");
        return 1;
    }
    dentry_t *file_meta = malloc(sizeof(dentry_t));
    int stat_res = stat(fd, file_meta);
    if (stat_res != PROTO_OK) {
        fprintf(STDERR, "[ERROR] Could stat file data.\n");
        return 1;
    }
    char *buf = malloc(file_meta->size);
    int read_res = read(fd, file_meta->size, buf);
    if (read_res < PROTO_OK) {
        fprintf(STDERR, "[ERROR] Could read file data.\n");
        return 1;
    }

    switch (mode)
    {
        case READ_CHAR:
            printf("%s\n", buf);
            break;
        case READ_HEX:
            for (int r = 0; r < file_meta->size / ROW_WIDTH; r++) {
                for (int c = 0; c < ROW_WIDTH; c++) {
                    printf("%s ", hex8_to_string_noprefix(buf[r * ROW_WIDTH + c]));
                }
                printf("\n");
            }
            for (int i = 0; i < file_meta->size % ROW_WIDTH; i++) {
                printf("%s ", hex8_to_string_noprefix(buf[(file_meta->size / ROW_WIDTH) * ROW_WIDTH + i]));
            }
            printf("\n");
            break;
        case READ_DECIMAL:
            for (int r = 0; r < file_meta->size / ROW_WIDTH; r++) {
                for (int c = 0; c < ROW_WIDTH; c++) {
                    printf("%3d ", buf[r * ROW_WIDTH + c]);
                }
                printf("\n");
            }
            for (int i = 0; i < file_meta->size % ROW_WIDTH; i++) {
                printf("%3d ", buf[(file_meta->size / ROW_WIDTH) * ROW_WIDTH + i]);
            }
            printf("\n");
        case READ_HEX_CHAR:
            int full_rows = file_meta->size / ROW_WIDTH;
            int rem = file_meta->size % ROW_WIDTH;
            for (int r = 0; r < full_rows; r++) {
                for (int c = 0; c < ROW_WIDTH; c++) {
                    printf("%s ", hex8_to_string_noprefix((uint8_t)buf[r * ROW_WIDTH + c]));
                }
                printf(" | ");
                for (int c = 0; c < ROW_WIDTH; c++) {
                    char ch = buf[r * ROW_WIDTH + c];
                    printf("%c", (ch >= 32 && ch <= 126) ? ch : '.');
                }
                printf("\n");
            }
            if (rem > 0) {
                int base = full_rows * ROW_WIDTH;
                for (int c = 0; c < ROW_WIDTH; c++) {
                    if (c < rem) {
                        printf("%s ", hex8_to_string_noprefix((uint8_t)buf[base + c]));
                    } else {
                        printf("   ");
                    }
                }
                printf(" | ");
                for (int c = 0; c < ROW_WIDTH; c++) {
                    if (c < rem) {
                        char ch = buf[base + c];
                        printf("%c", (ch >= 32 && ch <= 126) ? ch : '.');
                    } else {
                        printf(" ");
                    }
                }
                printf("\n");
            }
        default:
            break;
    }
    
    return 0;
}