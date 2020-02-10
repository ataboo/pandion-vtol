#ifndef AtaLogger_h
#define AtaLogger_h

#include <stdarg.h>
#include <string.h>

#ifdef ATALOGGER_IOSTREAM
#include <iostream>
#else
#include "Arduino.h"
#endif

#ifndef AtaLoggerLineLength
#define AtaLoggerLineLength 200
#endif

#ifndef AtaLoggerMaxTagCount
#define AtaLoggerMaxTagCount 20
#endif

enum LogLevel {
    DEBUG = 0,
    INFO,
    WARNING,
    ERROR,
    CRITICAL
};

void alog_debug(const char* tag, const char* format, ...);
void alog_info(const char* tag, const char* format, ...);
void alog_warn(const char* tag, const char* format, ...);
void alog_error(const char* tag, const char* format, ...);
void alog_critical(const char* tag, const char* format, ...);

void alog_set_log_level(const char* tag, LogLevel level);
void alog_set_default_level(LogLevel level);

#endif
