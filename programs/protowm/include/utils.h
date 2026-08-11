#ifndef UTILS_H
#define UTILS_H

#include <stddef.h>

static int collide_point(int cx, int cy, int x, int y, int w, int h) {
    return (cx >= x && cx < x + w && cy >= y && cy < y + h);
}

static int collide_rect(int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2) {
    return (x1 < x2 + w2 &&
            x1 + w1 > x2 &&
            y1 < y2 + h2 &&
            y2 + h1 > y2);
}

#define LL_UNLINK(item, list) {                     \
    if ((item)->next != NULL) {                     \
        (item)->next->prev = (item)->prev;          \
    } else {                                        \
        (list)->prev = (item)->prev;                \
    }                                               \
    if ((item) != (list)) {                         \
        (item)->prev->next = (item)->next;          \
    } else {                                        \
        (list) = (item)->next;                      \
    }                                               \
}

#define LL_APPEND(item, list) {                     \
    (item)->next = NULL;                            \
    if ((list) == NULL) {                           \
        (list) = item;                              \
    } else {                                        \
        (item)->prev = (list)->prev;                \
        (list)->prev->next = item;                  \
    }                                               \
    (list)->prev = item;                            \
}                                                   \

#endif // UTILS_H
