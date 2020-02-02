#define M1A_PIN 3
#define M1B_PIN 4
#define M1P_PIN 5

#define M2A_PIN 6
#define M2B_PIN 7
#define M2P_PIN 8

char write_buffer[32];

void setup() {
  Serial.begin(19200);
  
  pinMode(M1A_PIN, OUTPUT);
  pinMode(M1B_PIN, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
  digitalWrite(LED_BUILTIN, HIGH);
  digitalWrite(M1A_PIN, HIGH);
  digitalWrite(M1B_PIN, LOW);
  for(int i=0; i<11; i++) {
    sprintf(write_buffer, "Duty: %d", i * 10);
    
    int duty = i * 255 /10;
    analogWrite(M1P_PIN, duty);
    Serial.print(write_buffer);
    delay(1000);
  }

  analogWrite(M1P_PIN, 0);
  digitalWrite(LED_BUILTIN, LOW);
  delay(1000);
}
