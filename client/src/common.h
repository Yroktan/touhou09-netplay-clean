

#pragma once

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define ASSERT_SIZE(type, size) typedef char assert_size_##type[sizeof(type) == (size) ? 1 : -1]


#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))



#define GLOBAL_H "./system\\global.h"


class MemoryManager
{
public:

    MemoryManager();

    ~MemoryManager();


    void *Alloc(int size, const char *name)
    {
        return malloc(size);
    }
    void Free(void *p)
    {
        free(p);
    }


    void *Register(void *p, int size, const char *name)
    {
        return p;
    }


    void *blocks[0x1000];

    int freeBlocksOnExit;
};
ASSERT_SIZE(MemoryManager, 0x4004);
extern MemoryManager g_MemoryTracker;
