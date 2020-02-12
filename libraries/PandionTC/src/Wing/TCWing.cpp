#include "TCWing.h"

void TCWing::init(TCWingConfig config) {
    _config = config;

    pinMode(config.pin_motor_pulse, OUTPUT);
    pinMode(config.pin_motor_a, OUTPUT);
    pinMode(config.pin_motor_b, OUTPUT);

    char name_buffer[8];
    sprintf(name_buffer, "%s_E", config.name);
    _encoder.init((TCEncoderConfig){
        .name = name_buffer,
        .c1_pin = config.pin_encoder_c1,
        .c2_pin = config.pin_encoder_c2
    });

    digitalWrite(config.pin_motor_a, 1);
    // digitalWrite(config.pin_motor_a, 1);
    digitalWrite(config.pin_motor_b, 0);
    analogWrite(config.pin_motor_pulse, 255);

    alog_info("Blah", "Pulse pin: %d", config.pin_motor_pulse);

}

void TCWing::update(uint32_t delta_ms) {
    _encoder.update(delta_ms);
}
