#include "AtaLogger.h"

#define M1A_PIN 3
#define M1B_PIN 4
#define M1P_PIN 5

#define M2A_PIN 6
#define M2B_PIN 7
#define M2P_PIN 8

#define TAG "TC"
#define TAG2 "TC2"

char write_buffer[32];
AtaLogger logger;

void setup() {
  AtaLoggerConfig logConfig;
  logConfig.baud_rate = 19200;
  logConfig.tags = (const char*){"tag1","tag2","TC"};
  logConfig.tag_count = 1;
  logger.Init(logConfig);
  
  pinMode(M1A_PIN, OUTPUT);
  pinMode(M1B_PIN, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
  digitalWrite(LED_BUILTIN, HIGH);
//  digitalWrite(M1A_PIN, HIGH);
//  digitalWrite(M1B_PIN, LOW);
  for(int i=0; i<11; i++) {
    logger.Info(TAG, "Duty: %d", i * 10);
    logger.Info(TAG2, "other tag!");
    
    int duty = i * 255 /10;
    analogWrite(M1P_PIN, duty);
    delay(1000);
  }

//  analogWrite(M1P_PIN, 0);
  digitalWrite(LED_BUILTIN, LOW);
  delay(1000);
}
