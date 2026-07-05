#include "protofetch.h"

#include <proto.h>

int pmain(char argv[16][64]) {
    int about_fd = open("/Devices/about", "r");
    about_data_t about;

    int about_status = read(about_fd, 0, &about);

    if (about_status != PROTO_OK) {
        fprintf(STDERR, "(err) could not open required devices\n");
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
                printf(ANSI_BLUE "OS" ANSI_RESET ":   %s v%s", about.os_name, about.os_version);
                break;
            case 1:
                printf(ANSI_BLUE "Arch" ANSI_RESET": %s", about.arch);
                break;
            default:
                break;
        }
        printf("\n");

        tok = strtok(NULL, "\n");
    }
    printf("\n\n");

    close(about_fd);
    return 0;
}