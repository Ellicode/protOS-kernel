#include <proto.h>

#include "split.h"

int split_cmd(const char *path, char segments[16][64], int max_segs) {
    int count = 0;
    const char *p = path;
    while (*p && count < max_segs) {
        if (*p == ' ') { p++; continue; }
        int len = 0;
        while (p[len] && p[len] != ' ') { len++; }
        if (len > 0) {
            strncpy(segments[count++], p, len);
            segments[count-1][len] = '\0';
        }
        p += len;
    }
    return count;
}