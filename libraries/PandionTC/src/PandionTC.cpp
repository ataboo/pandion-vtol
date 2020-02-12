# include "PandionTC.h"

static const char* TAG = "PandionTC";

void PandionTC::init() 
{
    alog_info(TAG, "Tadaa!");
    // _leftWing.init(configureLeftWing());
    _rightWing.init(configureRightWing());
}

void PandionTC::update(uint32_t delta_us) {
    // _leftWing.update(delta_ms);
    _rightWing.update(delta_us);
}

void PandionTC::setAngle(float angle)
{

}

float PandionTC::currentAngle() 
{
    return 0;
}

// void PandionTC::initWingPins(TCWingConfig wingCfg) {
//     pinMode(wingCfg.pin_limit, INPUT_PULLUP);
//     pinMode(wingCfg.pin_encoder_c1, INPUT);
//     pinMode(wingCfg.pin_encoder_c2, INPUT);
//     pinMode(wingCfg.pin_motor_pulse, OUTPUT);
//     pinMode(wingCfg.pin_motor_a, OUTPUT);
//     pinMode(wingCfg.pin_motor_b, OUTPUT);
// }

// void PandionTC::updateWing(const TCWingConfig wingCfg, TCWingState* wingState, uint32_t delta_ms) {
//     bool limit_value = (bool)digitalRead(wingCfg.pin_limit);

//     if (limit_value != wingState->limit_on) {
//         _logger.debug(TAG, "Limit went from %d to %d", wingState->limit_on, limit_value);
//         wingState->limit_on = limit_value;
//     }
// }


