#pragma once
#include "pico/mutex.h"

struct ScopedMutexRecursive
{
    recursive_mutex_t *m;
    ScopedMutexRecursive(recursive_mutex_t *mut);
    ~ScopedMutexRecursive();
};

struct ScopedMutex
{
    mutex_t *m;
    ScopedMutex(mutex_t *mut);
    ~ScopedMutex();
};
