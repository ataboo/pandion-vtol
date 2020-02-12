#ifndef PandionTC_h
#define PandionTC_h

#include "Arduino.h"
#include <AtaLogger.h>
#include "Wing/Encoder/TCEncoderCounter.h"
#include "Wing/TCWing.h"

#define PANDIONTC_PROTOBOARD_PINS


class PandionTC {
  public:
    PandionTC(){};

    void init();
    void update(uint32_t delta_us);
    void setAngle(float angle);
    float currentAngle();

  private:
    TCWing _leftWing;
    TCWing _rightWing;

    TCWingConfig configureLeftWing();
    TCWingConfig configureRightWing();
};

#endif