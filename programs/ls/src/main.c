#include <proto.h>
#include "ls.h"

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
    dentry_t entries[64] = { 0 };
    int num_entries = 0;

    int cwd_fd;

    if (argc == 2) {
        cwd_fd = open(argv[1], "r");
    } else {
        cwd_fd = open(".", "r");
    }

    if (cwd_fd < PROTO_OK) {
        fprintf(STDERR, "[ERROR] No such directory.\n");
        return 1;
    }

    int read_status = read_dir(cwd_fd, entries, &num_entries);
    if (read_status != PROTO_OK) {
        fprintf(STDERR, "[ERROR] Cannot read directory (status %d)\n", read_status);
        return 1;
    }

    if (num_entries == 0) {
        printf("\n" ANSI_YELLOW);
        printf("              '^'               \n" ANSI_RESET);
        printf("  There doesnt seem to be any   \n");
        printf("      files here for now...     \n\n");
    }

    int biggest_name = 0;
    int biggest_size = 0;
    
    for (int i = 0; i < num_entries; i++) {
        int name_len = strlen(entries[i].name);
        int size_len = strlen(int_to_string(format_size(entries[i].size, NULL)));
        if (name_len > biggest_name) {
            biggest_name = name_len;
        }
        if (size_len > biggest_size) {
            biggest_size = size_len;
        }
    }

    for (int i = 0; i < num_entries; i++) {
        char *name = entries[i].name;
        inode_type_t type = entries[i].type;

        char *size_suffix = "B  ";
        int size = format_size(entries[i].size, &size_suffix);

        if (type == INODE_FILE) {
            printf(ANSI_YELLOW "f ");
        } else if (type == INODE_FOLDER) {
            printf(ANSI_CYAN "F ");
        }

        printf("%s    ", name);
        for (int s = 0; s < biggest_name - strlen(name); s++) { printf(" "); }

        printf(ANSI_RESET);

        printf("%d%s", size, size_suffix);
        for (int s = 0; s < biggest_size - strlen(int_to_string(size)); s++) { printf(" "); }

        printf("\n");
    }

    close(cwd_fd);

    return 0;
}