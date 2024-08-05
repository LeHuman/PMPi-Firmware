#pragma once

#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <string>

#ifndef LOG_DISABLE_COLOR
#define __log_C_FATAL "\033[38;5;226;48;5;160m"
#define __log_C_CRITICAL "\033[38;5;231;48;5;124m"
#define __log_C_ERROR "\033[38;5;196m"
#define __log_C_WARNING "\033[38;5;202m"
#define __log_C_NOTICE "\033[38;5;57m"
#define __log_C_INFO "\033[38;5;28m"
#define __log_C_DEBUG "\033[38;5;163m"
#define __log_C_TRACE "\033[38;5;59m"
#define __log_C_LOG "\033[38;5;211m"
#define __log_CLEAR "\033[0m"
#else
#define __log_C_FATAL
#define __log_C_CRITICAL
#define __log_C_ERROR
#define __log_C_WARNING
#define __log_C_NOTICE
#define __log_C_INFO
#define __log_C_DEBUG
#define __log_C_TRACE
#define __log_C_LOG
#define __log_CLEAR
#endif

#define __log_FATAL     " FATL "
#define __log_CRITICAL  " CRIT "
#define __log_ERROR     " EROR "
#define __log_WARNING   " WARN "
#define __log_NOTICE    " NOTI "
#define __log_INFO      " INFO "
#define __log_DEBUG     " DEBG "
#define __log_TRACE     " TRCE "
#define __log_LOG       " LOGG "

#define __log_START "% 4u|"
#define __log_END " %s\n"
#define __log_IT __log_CLEAR "|" __log_END
#define __log_ME __log_CLEAR "| %s |" __log_END

#define __logit_FATAL __log_START __log_C_FATAL __log_FATAL __log_IT
#define __logit_CRITICAL __log_START __log_C_CRITICAL __log_CRITICAL __log_IT
#define __logit_ERROR __log_START __log_C_ERROR __log_ERROR __log_IT
#define __logit_WARNING __log_START __log_C_WARNING __log_WARNING __log_IT
#define __logit_NOTICE __log_START __log_C_NOTICE __log_NOTICE __log_IT
#define __logit_INFO __log_START __log_C_INFO __log_INFO __log_IT
#define __logit_DEBUG __log_START __log_C_DEBUG __log_DEBUG __log_IT
#define __logit_TRACE __log_START __log_C_TRACE __log_TRACE __log_IT
#define __logit_LOG __log_START __log_C_LOG __log_LOG __log_IT

#define __logme_FATAL __log_START __log_C_FATAL __log_FATAL __log_ME
#define __logme_CRITICAL __log_START __log_C_CRITICAL __log_CRITICAL __log_ME
#define __logme_ERROR __log_START __log_C_ERROR __log_ERROR __log_ME
#define __logme_WARNING __log_START __log_C_WARNING __log_WARNING __log_ME
#define __logme_NOTICE __log_START __log_C_NOTICE __log_NOTICE __log_ME
#define __logme_INFO __log_START __log_C_INFO __log_INFO __log_ME
#define __logme_DEBUG __log_START __log_C_DEBUG __log_DEBUG __log_ME
#define __logme_TRACE __log_START __log_C_TRACE __log_TRACE __log_ME
#define __logme_LOG __log_START __log_C_LOG __log_LOG __log_ME

namespace Log {

enum class Level {
    FATAL,
    CRITICAL,
    ERROR,
    WARNING,
    NOTICE,
    INFO,
    DEBUG,
    TRACE,
};

static inline const char *logitString(const Log::Level &lvl) {
    using enum Log::Level;

    switch (lvl) {
        case FATAL:
            return __logit_FATAL;
        case CRITICAL:
            return __logit_CRITICAL;
        case ERROR:
            return __logit_ERROR;
        case WARNING:
            return __logit_WARNING;
        case NOTICE:
            return __logit_NOTICE;
        case INFO:
            return __logit_INFO;
        case DEBUG:
            return __logit_DEBUG;
        case TRACE:
            return __logit_TRACE;
        default:
            return __logit_LOG;
    }
}

static inline const char *logmeString(const Log::Level &lvl) {
    using enum Log::Level;

    switch (lvl) {
        case FATAL:
            return __logme_FATAL;
        case CRITICAL:
            return __logme_CRITICAL;
        case ERROR:
            return __logme_ERROR;
        case WARNING:
            return __logme_WARNING;
        case NOTICE:
            return __logme_NOTICE;
        case INFO:
            return __logme_INFO;
        case DEBUG:
            return __logme_DEBUG;
        case TRACE:
            return __logme_TRACE;
        default:
            return __logme_LOG;
    }
}

static int32_t logit(const Level level, const char *format, ...) {
    static int32_t count = 0;
    char buffer[512];
    va_list args;

    va_start(args, format);
    (void)vsnprintf(buffer, 512, format, args);
    va_end(args);

    return printf(logitString(level), count++, buffer);
};

static int32_t logme(const Level level, const char *const name, const char *format, ...) {
    static int32_t count = 0;
    char buffer[512];
    va_list args;

    va_start(args, format);
    (void)vsnprintf(buffer, 512, format, args);
    va_end(args);

    return printf(logmeString(level), count++, name, buffer);
};

// static void logit(const char *format, ...) {
//     va_list args;
//     va_start(args, format);
//     (void)logit(Level::INFO, format, args);
//     va_end(args);
// };

// static void logme(const char *const name, const char *format, ...) {
//     va_list args;
//     va_start(args, format);
//     (void)logme(Level::INFO, name, format, args);
//     va_end(args);
// };

} // namespace Log
