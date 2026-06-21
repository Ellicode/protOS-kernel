/**
 * COQI
 * Coqi is a very simple and lightweight shell for protOS, featuring
 * program execution, history and more!
 * */

#include <stdint.h>
#include <stdlib.h>

#include <proto.h>

char cmdbuf[1024]; // if you do a command greater than this, good job!

int _start()
{
    while (1)
    {
        write(1, 0, "$ ");
        read(0, 0, cmdbuf);
    }
}