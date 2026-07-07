/**
 * ProtOS Corgi
 * 
 * Corgi is a very simple and lightweight shell for protOS, featuring
 * program execution, history and more!
 *
 * */

#include <stdint.h>
#include <proto.h>

#include "tools.h"
#include "split.h"
#include "corgi.h"

int pmain()
{
    chdir("/");
    printf("protOS corgi v%s\n", CORGI_VERSION);

    char *pathbuf = malloc(128);
    char *cmdbuf = malloc(256);
    char argv[16][64];

    while (1)
    {
        getcwd(pathbuf, 128);
        printf("[" ANSI_BLUE "%s" ANSI_RESET "]& ", pathbuf);

        input(cmdbuf);
        split_cmd(cmdbuf, argv, 64);

        if (strcmp(argv[0], "cd") == 0) {
            crg_cd(argv);
        } else {
            char path[256];
            snprintf(path, 256, "/System/Programs/%s", argv[0]);
            int pid;
            int ret = create_process(path, argv);
            if (ret == PROTO_ERR_FILE_NOT_FOUND) {
                fprintf(STDERR, "(err) no such program \"%s\"\n", argv[0]);
            }
        }
    }

    free(pathbuf);
    free(cmdbuf);
}