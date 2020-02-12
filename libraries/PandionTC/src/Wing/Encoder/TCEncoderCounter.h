#pragma once

#include "Arduino.h"
#include "AtaLogger.h"

struct TCEncoderConfig {
    char* name;
    unsigned c1_pin;
    unsigned c2_pin;
};

class TCEncoderCounter {
    public:
        void init(TCEncoderConfig config);
        void update(uint32_t delta_us);
        // float currentAngle();
        void zero();
    private:
        TCEncoderConfig _config;
        int _frame_count_on;
        int _frame_count_off;
        bool _last_frame_on;
};

