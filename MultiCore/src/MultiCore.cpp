#include "MultiCore.h"

ScopedMutexRecursive::ScopedMutexRecursive(recursive_mutex_t *mut) : m(mut) { recursive_mutex_enter_blocking(m); }
ScopedMutexRecursive::~ScopedMutexRecursive() { recursive_mutex_exit(m); }

ScopedMutex::ScopedMutex(mutex_t *mut) : m(mut) { mutex_enter_blocking(m); }
ScopedMutex::~ScopedMutex() { mutex_exit(m); }
