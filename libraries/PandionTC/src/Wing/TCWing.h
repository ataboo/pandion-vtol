#ifndef TCWing_h
#define TCWing_h

#include <AtaLogger.h>
#include "Encoder/TCEncoderCounter.h"

struct TCWingConfig {
    const char* name;
    int pin_limit;
    unsigned pin_encoder_c1;
    unsigned pin_encoder_c2;
    unsigned pin_motor_pulse;
    unsigned pin_motor_a;
    unsigned pin_motor_b;
};

class TCWing {
  public:
    TCWing(){};
    void init(TCWingConfig config);
    void update(uint32_t delta_us);

  private:
    bool _limit_on;
    TCEncoderCounter _encoder;
    TCWingConfig _config;
};

#endif