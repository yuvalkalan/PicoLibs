#pragma once
#include <stdio.h>
#include <stdarg.h>
#include "pico/stdlib.h"

enum LogLevel
{
    TRACE = 0,
    DEBUG,
    INFO,
    WARNING,
    ERROR,
    CRITICAL

};

class Logger
{
private:
    int m_level;

public:
    Logger(int level);
    void print(int level, const char *fmt, ...);
};
