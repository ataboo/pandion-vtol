#include "AtaLogger.h"

AtaLogger::AtaLogger() {
#ifndef ATALOGGER_IOSTREAM
    Serial.begin(9600);
#endif
}

void AtaLogger::debug(const char* tag, const char* format, ...) {
    va_list args;
    va_start(args, format);
    logAtLevel(DEBUG, tag, format, args);
    va_end(args);
}

void AtaLogger::info(const char* tag, const char* format, ...) {
    va_list args;
    va_start(args, format);
    logAtLevel(INFO, tag, format, args);
    va_end(args);
}

void AtaLogger::warning(const char* tag, const char* format, ...) {
    va_list args;
    va_start(args, format);
    logAtLevel(WARNING, tag, format, args);
    va_end(args);
}

void AtaLogger::error(const char* tag, const char* format, ...) {
    va_list args;
    va_start(args, format);
    logAtLevel(ERROR, tag, format, args);
    va_end(args);
}

void AtaLogger::critical(const char* tag, const char* format, ...) {
    va_list args;
    va_start(args, format);
    logAtLevel(CRITICAL, tag, format, args);
    va_end(args);
}

void AtaLogger::logAtLevel(LogLevel level, const char* tag, const char* format, va_list args) {
    enum LogLevel tag_level = getLogLevel(tag);
    
    if (tag_level > level) {
        return;
    }

    writeToSerial(tag, format, args);
}

void AtaLogger::setDefaultLogLevel(LogLevel level) {
    _default_log_level = level;
}

LogLevel AtaLogger::getLogLevel(const char* tag) {
    for(int i=0; i<AtaLoggerMaxTagCount; i++) {
        if (_tag_level_tags[i] == nullptr) {
            break;
        }

        if (strcmp(_tag_level_tags[i], tag) == 0) {
            return _tag_level_levels[i];
        }
    }

    return _default_log_level;
}

void AtaLogger::setTagLogLevel(const char* tag, LogLevel level) {
    for(int i=0; i<AtaLoggerMaxTagCount; i++) {
        if (_tag_level_tags[i] == nullptr) {
            _tag_level_levels[i] = level;
            _tag_level_tags[i] = tag;
            return;
        }

        if (strcmp(_tag_level_tags[i], tag) == 0) {
            _tag_level_levels[i] = level;
            return;
        }
    }

    #ifdef ATALOGGER_IOSTREAM
        std::cerr<<"Failed to set level! Too many tags?\n";
    #endif
}

void AtaLogger::writeToSerial(const char* tag, const char* format, ...) {
    va_list args;
    va_start(args, format);
    writeToSerial(tag, format, args);
    va_end(args);
}

void AtaLogger::writeToSerial(const char* tag, const char* format, va_list args) {
    vsprintf(_fmtBuffer, format, args);

    sprintf(_lineBuffer, "[%s]  ", tag);
    strcat(_lineBuffer, _fmtBuffer);
    strcat(_lineBuffer, "\n");

    #ifdef ATALOGGER_IOSTREAM
        std::cout<<_lineBuffer;
    #else
        Serial.print(_lineBuffer);
    #endif
}