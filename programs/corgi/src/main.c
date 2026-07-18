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

int pmain(char argv[16][64], int argc)
{
    chdir("/");
    printf("protOS corgi v%s\n", CORGI_VERSION);

    char *pathbuf = malloc(128);
    char *cmdbuf = malloc(256);
    char _argv[16][64];

    int pid = getpid();

    while (1)
    {
        getcwd(pathbuf, 128);
        printf("[" ANSI_BLUE "%s" ANSI_RESET "]& ", pathbuf);

        input(cmdbuf);
        int _argc = split_cmd(cmdbuf, _argv, 64);
        if (_argc > 0) {
            if (strcmp(_argv[0], "cd") == 0) {
                crg_cd(_argv);
            } else if (strcmp(_argv[0], "exit") == 0) {
                if (pid > 1) {
                    exit();
                }
            } else {
                char path[256];
                snprintf(path, 256, "/System/Programs/%s", _argv[0]);
                int pid;
                int ret = create_process(path, _argv, _argc);

                if (ret == PROTO_ERR_FILE_NOT_FOUND) {
                    fprintf(STDERR, "[ERROR] Cannot find command \"%s\"\n", _argv[0]);
                } else {
                    fprintf(STDERR, "[ERROR] Failed to launch program \"%s\" (status=%d)\n", _argv[0], ret);
                }
            }
        }
    }

    free(pathbuf);
    free(cmdbuf);
}