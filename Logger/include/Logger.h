#pragma once
#include <stdio.h>
#include <stdarg.h>
#include "pico/stdlib.h"
#include "MultiCore.h"

enum class LogLevel
{
    TRACE = 0,
    DEBUG,
    WEAK_WARNING,
    INFO,
    WARNING,
    ERROR,
    CRITICAL

};

class Logger
{
private:
    Logger() = delete;
    static LogLevel m_level;

public:
    static void print(LogLevel level, const char *fmt, ...);
    static void set_level(LogLevel level);
};
