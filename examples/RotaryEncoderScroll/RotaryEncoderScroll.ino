#include <iBleHID.h>

const uint8_t encoderA = 4;
const uint8_t encoderB = 5;
BleMouse mouse("iBle Encoder Scroll");
int lastA;

void setup() {
  pinMode(encoderA, INPUT_PULLUP);
  pinMode(encoderB, INPUT_PULLUP);
  lastA = digitalRead(encoderA);
  mouse.begin();
}

void loop() {
  const int a = digitalRead(encoderA);
  if (a != lastA) {
    if (mouse.isConnected()) {
      if (digitalRead(encoderB) != a) mouse.scrollUp();
      else mouse.scrollDown();
    }
    lastA = a;
  }
  delay(1);
}
