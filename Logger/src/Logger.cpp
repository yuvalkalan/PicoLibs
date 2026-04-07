#include "Logger.h"

LogLevel Logger::m_level = LogLevel::INFO;

void Logger::print(LogLevel level, const char *fmt, ...)
{
    // print data based on the level
    if (level < m_level)
    {
        return;
    }

    uint32_t now = to_ms_since_boot(get_absolute_time());
    printf("[%lu ms] ", now);
    switch (level)
    {
    case LogLevel::TRACE:
        printf("\033[90m[TRACE] "); // Gray
        break;
    case LogLevel::DEBUG:
        printf("\033[36m[DEBUG] "); // Cyan
        break;
    case LogLevel::INFO:
        printf("\033[32m[INFO] "); // Green
        break;
    case LogLevel::WARNING:
        printf("\033[33m[WARN] "); // Yellow
        break;
    case LogLevel::ERROR:
        printf("\033[31m[ERROR] "); // Red
        break;
    case LogLevel::CRITICAL:
        printf("\033[1;31m[CRIT] "); // Bold Red
        break;
    default:
        break;
    }

    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
    printf("\033[0m");
}

void Logger::set_level(LogLevel level)
{
    m_level = level;
}
