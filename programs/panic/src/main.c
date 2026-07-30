#include <proto/core.h>

int pmain(char argv[16][64], int argc) {
    int retryCount = 0;
    char *ynbuf = malloc(256);
    printf("This command will crash your computer. Do you still want to proceed? [y/n]: ");
    while (retryCount < 5)
    {
        input(ynbuf);

        if (strcmp(ynbuf, "y") == 0 || strcmp(ynbuf, "Y") == 0) {
            printf("Proceeding with the command...\n");
            char *msg;
            msg = (argc > 1) ? argv[1] : "User-Invoked Error";
            panic(msg);
            break;
        } 
        else if (strcmp(ynbuf, "n") == 0 || strcmp(ynbuf, "N") == 0) {
            printf("Aborted.\n");
            break;
        } 
        else {
            retryCount++;
            if (retryCount < 5) {
                printf("This command will crash your computer. Do you still want to proceed? [y/n]: ");
            } else {
                printf("Just read bro.\n");
            }
        }
    }
    return 0;
}