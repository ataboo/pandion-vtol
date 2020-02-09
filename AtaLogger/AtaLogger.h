#ifndef AtaLogger_h
#define AtaLogger_h

#include "Arduino.h"
#include <stdarg.h>

#ifndef AtaLoggerLineLength
#define AtaLoggerLineLength 200
#endif

enum LogLevel {
    DEBUG = 0,
    INFO,
    WARNING,
    ERROR,
    CRITICAL
};

struct AtaLoggerConfig {
    LogLevel level;
    int baud_rate;
    int tag_count;
    const char* tags;

    AtaLoggerConfig() {
      level = INFO;
      tag_count = 0;
      baud_rate = 9600;
      tags = {};
    }
};

class AtaLogger {
    public:
      Logging(){};
      void Init(AtaLoggerConfig config);
      void Info(const char* tag, const char* format, ...);

    private:
      AtaLoggerConfig _config;
      bool tagActive(const char * tag);
      void writeToSerial(const char * tag, const char * format, ...);
      void writeToSerial(const char* tag, const char* format, va_list args);
      char _lineBuffer[AtaLoggerLineLength];
      char _fmtBuffer[AtaLoggerLineLength];
};

extern AtaLogger Log;
 
#endif