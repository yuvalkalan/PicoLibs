#include "Logger.h"

Logger::Logger(int level) : m_level(level)
{
}

Logger &Logger::getInstance(int level)
{
    static Logger instance(level);
    return instance;
}

void Logger::print(int level, const char *fmt, ...)
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
    case TRACE:
        printf("\033[90m[TRACE] "); // Gray
        break;
    case DEBUG:
        printf("\033[36m[DEBUG] "); // Cyan
        break;
    case INFO:
        printf("\033[32m[INFO] "); // Green
        break;
    case WARNING:
        printf("\033[33m[WARN] "); // Yellow
        break;
    case ERROR:
        printf("\033[31m[ERROR] "); // Red
        break;
    case CRITICAL:
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