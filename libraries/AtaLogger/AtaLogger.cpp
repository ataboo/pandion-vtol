#include "AtaLogger.h"

static char _lineBuffer[AtaLoggerLineLength];
static char _fmtBuffer[AtaLoggerLineLength];
static const char* _tag_level_tags[AtaLoggerMaxTagCount] = { nullptr };
static LogLevel _tag_level_levels[AtaLoggerMaxTagCount] = { INFO };
static LogLevel _default_log_level = INFO;


static void write_to_serial(const char* tag, const char* format, va_list args) {
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

static void write_to_serial(const char* tag, const char* format, ...) {
    va_list args;
    va_start(args, format);
    write_to_serial(tag, format, args);
    va_end(args);
}

static LogLevel get_log_level(const char* tag) {
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

static void log_at_level(LogLevel level, const char* tag, const char* format, va_list args) {
    enum LogLevel tag_level = get_log_level(tag);
    
    if (tag_level > level) {
        return;
    }

    write_to_serial(tag, format, args);
}

void alog_debug(const char* tag, const char* format, ...) {
    va_list args;
    va_start(args, format);
    log_at_level(DEBUG, tag, format, args);
    va_end(args);
}

void alog_info(const char* tag, const char* format, ...) {
    va_list args;
    va_start(args, format);
    log_at_level(INFO, tag, format, args);
    va_end(args);
}

void alog_warn(const char* tag, const char* format, ...) {
    va_list args;
    va_start(args, format);
    log_at_level(WARNING, tag, format, args);
    va_end(args);
}

void alog_error(const char* tag, const char* format, ...) {
    va_list args;
    va_start(args, format);
    log_at_level(ERROR, tag, format, args);
    va_end(args);
}

void alog_critical(const char* tag, const char* format, ...) {
    va_list args;
    va_start(args, format);
    log_at_level(CRITICAL, tag, format, args);
    va_end(args);
}

void alog_set_default_level(LogLevel level) {
    _default_log_level = level;
}

void alog_set_log_level(const char* tag, LogLevel level) {
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
