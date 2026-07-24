#ifndef UTILS_H
#define UTILS_H


#include <stddef.h>

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