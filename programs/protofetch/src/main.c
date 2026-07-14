#include "protofetch.h"

#include <proto.h>

char *names[] = {
    "KiB",
    "MiB",
    "GiB",
    "TiB",
    "PiB",
    "EiB",
    "ZiB",
    "YiB"
};

int format_size(int size, char **suffix) {
    int new_size = size;
    for (int i = 0; i < sizeof(names)/sizeof(char**); i++) {
        if (new_size < 1024) { break; }
        new_size /= 1024;
        if (suffix != NULL) {
            *suffix = names[i];
        }
    }
    return new_size;
}

int pmain(char argv[16][64], int argc) {
    int about_fd = open("/Devices/about", "r");
    about_data_t *about = malloc(sizeof(about_data_t));
    int about_status = read(about_fd, 0, about);

    if (about_status != PROTO_OK) {
        fprintf(STDERR, "(err) could not open required devices (%d)\n", about_status);
        return 1;
    }
    
    char *tok = strtok(PROTOGEN_ASCII, "\n");
    for (int i = 0; i < PROTOFETCH_ROWS; i++) {
        if (tok == NULL) {
            for (int j = 0; j < PROTOGEN_ASCII_WIDTH + 1; j++) {
                printf(" ");
            }
        } else {
            printf("%s ", tok);
        }

        switch (i) {
            case 0:
            OS:     
            ARCH:   
            MEMORY: 
                printf(ANSI_BLUE "OS" ANSI_RESET ":     %s v%s", about->os_name, about->os_version);
                break;
            case 1:
                printf(ANSI_BLUE "Arch" ANSI_RESET":   %s", about->os_arch);
                break;
            case 2:
                char *used_suffix = "B";
                char *size_suffix = "B";

                int used = format_size(about->mem_used, &used_suffix);
                int size = format_size(about->mem_size, &size_suffix);
                int percentage = about->mem_used / (about->mem_size / 100);

                printf(ANSI_BLUE "Memory" ANSI_RESET": %d%s / %d%s (%d%% used)", used, used_suffix, size, size_suffix, percentage);
                break;

            // ...

            case PROTOFETCH_ROWS - 1:
                printf(ANSI_RED SWATCH_SYMBOL ANSI_GREEN SWATCH_SYMBOL ANSI_YELLOW SWATCH_SYMBOL ANSI_BLUE SWATCH_SYMBOL ANSI_MAGENTA SWATCH_SYMBOL ANSI_CYAN SWATCH_SYMBOL ANSI_RESET SWATCH_SYMBOL);

            default:
                break;
        }
        printf("\n");

        tok = strtok(NULL, "\n");
    }
    printf("\n\n");
    free(about);
    close(about_fd);
    return 0;
}