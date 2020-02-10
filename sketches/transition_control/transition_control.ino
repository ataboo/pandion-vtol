//#include <AtaLogger.h>
#include <PandionTC.h>

#define DELTA_MS 1

PandionTC transition;

unsigned long last_tick_ms;
uint32_t delta_ms;
static const char* TAG = "INO_SCRIPT";

void setup() {
  Serial.begin(2000000);

  alog_set_default_level(INFO);
 
  transition = PandionTC();
  transition.init();
}

void loop() {
  unsigned long this_tick_ms = micros();
  delta_ms = (uint32_t)(this_tick_ms - last_tick_ms);
  last_tick_ms = this_tick_ms;
  
  transition.update(delta_ms);
}
