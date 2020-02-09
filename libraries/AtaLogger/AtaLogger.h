#pragma once

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

class AtaLogger {
    public:
      AtaLogger();
      void setDefaultLogLevel(LogLevel level);
      void setTagLogLevel(const char* tag, enum LogLevel level);

      void debug(const char* tag, const char* format, ...);
      void info(const char* tag, const char* format, ...);
      void warning(const char* tag, const char* format, ...);
      void error(const char* tag, const char* format, ...);
      void critical(const char* tag, const char* format, ...);

    private:
      enum LogLevel getLogLevel(const char* tag);
      void logAtLevel(LogLevel level, const char* tag, const char* format, va_list args);
      void writeToSerial(const char* tag, const char * format, ...);
      void writeToSerial(const char* tag, const char* format, va_list args);
      char _lineBuffer[AtaLoggerLineLength];
      char _fmtBuffer[AtaLoggerLineLength];
      const char* _tag_level_tags[AtaLoggerMaxTagCount] = { nullptr };
      LogLevel _tag_level_levels[AtaLoggerMaxTagCount] = { INFO };
      LogLevel _default_log_level = INFO;
};
