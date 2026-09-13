#include <iBleHID.h>

BleGamepad gamepad;

void setup() { gamepad.begin(); }

void loop() {
  if (gamepad.isConnected()) {
    gamepad.setLeftThumb(16000, 0);
    gamepad.setHat(2);       // up
    delay(250);
    gamepad.setLeftThumb(0, 0);
    gamepad.setHat(8);       // released
    gamepad.press(1);
    delay(100);
    gamepad.release(1);
  }
  delay(1000);
}
