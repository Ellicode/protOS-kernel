#ifndef USERSPACE_H
#define USERSPACE_H

#include <stdint.h>

extern void enter_userspace(uint64_t entry, uint64_t user_rsp);

#endif // USERSPACE_H