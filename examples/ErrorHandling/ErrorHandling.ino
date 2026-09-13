#include <iBleHID.h>

BleMouse mouse("iBle Error Handling");

void setup() {
  Serial.begin(115200);
  mouse.setLogging(true);
  mouse.begin();
  if (mouse.lastError() != HidStatus::Ok) {
    Serial.printf("begin failed: %u\n",
                  static_cast<unsigned>(mouse.lastError()));
  }
}

void loop() {
  if (mouse.isConnected()) {
    mouse.move(4, 0);
  } else if (mouse.lastError() == HidStatus::NotConnected) {
    Serial.println("Waiting for a HID host");
    mouse.clearError();
  }
  delay(1000);
}
