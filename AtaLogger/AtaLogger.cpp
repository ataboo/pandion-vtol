# include "AtaLogger.h"


void AtaLogger::Init(AtaLoggerConfig config) {
    _config = config;
    Serial.begin(config.baud_rate);
}

void AtaLogger::Info(const char* tag, const char* format, ...) {
    if (_config.level > INFO || !tagActive(tag)) {
        return;
    }

    va_list args;
    va_start(args, format);
    writeToSerial(tag, format, args);
    va_end(args);
}

bool AtaLogger::tagActive(const char* tag) 
{
    if (_config.tags == NULL || _config.tag_count == 0) {
        return true;
    }

    for(int i=0; i<_config.tag_count; i++) {
        writeToSerial("blah", "Checking tag %s", _config.tags[i]);
        if (strcmp(_config.tags[i], tag) == 0) {
            return true;
        }
    }

    return false;
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
    strcat(_lineBuffer, "\n\r");

    Serial.write(_lineBuffer);
}