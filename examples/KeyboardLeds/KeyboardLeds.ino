#include <iBleHID.h>

BleKeyboard keyboard("iBle LED Keyboard");

void onKeyboardLeds(uint8_t leds) {
  Serial.printf("LED report: caps=%u num=%u scroll=%u\n",
                (leds & 0x02) != 0, (leds & 0x01) != 0,
                (leds & 0x04) != 0);
}

void setup() {
  Serial.begin(115200);
  keyboard.onLedState(onKeyboardLeds);
  keyboard.begin();
}

void loop() {
  if (keyboard.isConnected()) {
    keyboard.print("LED-aware keyboard");
    keyboard.write(KEY_RETURN);
  }
  delay(3000);
}
