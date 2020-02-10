#include "TCEncoderCounter.h"

#define MAX_STEP 100
#define MIN_STEP 0
#define PIN_LEVEL_THRESHOLD 500
#define ON_STEP_THRESHOLD 4
#define OFF_STEP_THRESHOLD 4

unsigned long loop_count = 0;
unsigned long sum = 0;

void TCEncoderCounter::init(TCEncoderConfig config) {
    _config = config;
    
    pinMode(_config.c1_pin, INPUT);
    pinMode(_config.c2_pin, INPUT);
    zero();
}

void TCEncoderCounter::update(uint32_t delta_us) {
    int c1_high = analogRead(_config.c1_pin);
    int c2_high = analogRead(_config.c2_pin);

    sum += delta_us;

    if (loop_count++ > 10000) {
        alog_info("TCEncoder", "Mean delta,%d", sum / loop_count);
        loop_count = 0;
        sum = 0;
    }

    alog_info("c1high_low", ",%d,%d,%d", c1_high, c2_high, delta_us);
    // alog_info("c2high", "%d", c2_high);
    // Serial.println(c1_high + c2_high);

    // if (encoder_high) {
    //     _frame_count_off = 0;
    //     if (++_frame_count_on == ON_STEP_THRESHOLD) {
    //         _last_frame_on = true;
    //     }
    // } else {
    //     _frame_count_on = 0;
    //     if (++_frame_count_off == OFF_STEP_THRESHOLD) {
    //         _last_frame_on = false;
    //     }
    // }

    // if (encoder_on) {
    //     _logger.info("blah", "Encoder on!");
    //     _frame_count_on++;
    // } else {
    //     _logger.info("blah", "Encoder off!");
    //     _frame_count_off++;
    // }
}

// void TCEncoderCounter::isOn() {
//     float angle = (float)_current_step / (MAX_STEP - MIN_STEP);
//     if (angle > 1 || angle < 0) {
//         alog_warn(_config.name, "TCEncoder angle out of range -- Step: %d, Angle: %.2f", _current_step, angle);
//     }

//     return angle;
// }

void TCEncoderCounter::zero() {
    _last_frame_on = 0;
    _frame_count_off = 0;
    _frame_count_on = 0;
}
