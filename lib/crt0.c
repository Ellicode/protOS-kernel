#include "proto.h"

extern int pmain(char argv[16][64]);

void _start(char argv[16][64])
{
    pmain(argv);

    exit();
}