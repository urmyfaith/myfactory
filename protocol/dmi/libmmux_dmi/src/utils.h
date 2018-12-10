#ifndef __UTILS_H__
#define __UTILS_H__

#include "typesdefined.h"

int64_t  getSystemTime();

int is_little_endian(void);

uint64_t __ntohll2(uint64_t val);

uint64_t __htonll2(uint64_t val);

#endif