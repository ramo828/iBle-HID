#include <iBleHID.h>

BleKeyboard keyboard("iBle Configured Keyboard");

void setup() {
  Serial.begin(115200);
  keyboard.setLogging(true);
  keyboard.setReportDelay(4);
  keyboard.setSecurityEnabled(true);
  keyboard.setAdvertisingInterval(160, 320);
  keyboard.updateBattery(87);
  keyboard.begin();
}

void loop() {
  if (keyboard.isConnected()) {
    keyboard.println("Configured HID");
  }
  delay(2000);
}
