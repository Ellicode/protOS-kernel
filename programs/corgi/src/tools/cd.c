#include <proto.h>

void crg_cd(char argv[16][64]) {
    int res = chdir(argv[1]);
    if (res == PROTO_ERR_FILE_NOT_FOUND) {
        fprintf(STDERR, "(err) folder \"%s\" not found\n", argv[1]);
    } else if (res == PROTO_ERR_NOT_A_DIRECTORY) {
        fprintf(STDERR, "(err) path element \"%s\" is not a directory\n", argv[1]);
    }
}