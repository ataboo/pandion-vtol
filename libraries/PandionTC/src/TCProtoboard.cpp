#pragma once

#include <PandionTC.h>
#include <Arduino.h>

#ifdef PANDIONTC_PROTOBOARD_PINS
    #define PIN_RIGHT_LIMIT A2
    #define PIN_RIGHT_ENCODER_C2 A3
    #define PIN_RIGHT_ENCODER_C1 A4
    #define PIN_RIGHT_MOTOR_P 5
    #define PIN_RIGHT_MOTOR_A 3
    #define PIN_RIGHT_MOTOR_B 4

    #define PIN_LEFT_LIMIT A5
    #define PIN_LEFT_ENCODER_C2 A6
    #define PIN_LEFT_ENCODER_C1 A7
    #define PIN_LEFT_MOTOR_P 10
    #define PIN_LEFT_MOTOR_A 9
    #define PIN_LEFT_MOTOR_B 8

    TCWingConfig PandionTC::configureRightWing() {
        return TCWingConfig {
            .name = "TCRW",
            .pin_limit = PIN_RIGHT_LIMIT,
            .pin_encoder_c1 = PIN_RIGHT_ENCODER_C1,
            .pin_encoder_c2 = PIN_RIGHT_ENCODER_C2,
            .pin_motor_pulse = PIN_RIGHT_MOTOR_P,
            .pin_motor_a = PIN_RIGHT_MOTOR_A,
            .pin_motor_b = PIN_RIGHT_MOTOR_B
        };
    }

    TCWingConfig PandionTC::configureLeftWing() {
        return TCWingConfig {
            .name = "TCLW",
            .pin_limit = PIN_LEFT_LIMIT,
            .pin_encoder_c1 = PIN_LEFT_ENCODER_C1,
            .pin_encoder_c2 = PIN_LEFT_ENCODER_C2,
            .pin_motor_pulse = PIN_LEFT_MOTOR_P,
            .pin_motor_a = PIN_LEFT_MOTOR_A,
            .pin_motor_b = PIN_LEFT_MOTOR_B
        };
    }
#endif