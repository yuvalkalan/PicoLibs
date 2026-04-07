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
    Logger(int level);

public:
    // Prevent copying or assigning the singleton instance
    Logger(const Logger &) = delete;
    Logger &operator=(const Logger &) = delete;

    static Logger &getInstance(int level = TRACE);
    void print(int level, const char *fmt, ...);
};
