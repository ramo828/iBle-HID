#include <iBleHID.h>

BleMouse mouse("iBle Directional Scroll");

void setup() {
  mouse.begin();
}

void loop() {
  if (mouse.isConnected()) {
    mouse.scrollUp();
    delay(500);
    mouse.scrollDown();
    delay(500);
    mouse.scrollLeft();
    delay(500);
    mouse.scrollRight();
  }
  delay(1000);
}
