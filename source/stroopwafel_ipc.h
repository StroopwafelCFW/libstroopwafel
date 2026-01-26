#pragma once
#include <stdint.h>

#define ALIGN(align)                 __attribute__((aligned(align)))
#define ALIGN_0x40                   ALIGN(0x40)
#define ROUNDUP(x, align)            (((x) + ((align) -1)) & ~((align) -1))