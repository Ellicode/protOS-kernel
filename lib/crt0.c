#include <proto/core.h>

extern int pmain(char argv[16][64], int argc);

void _start(char argv[16][64], int argc)
{
    heap_init();
    
    pmain(argv, argc);

    exit();
}