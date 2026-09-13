#include <iBleHID.h>

BleComposite hid("iBle HID Advanced");

void setup() { hid.begin(); }

void loop() {
  if (hid.isConnected()) {
    hid.press(KEY_LEFT_CTRL);
    hid.press('a');
    hid.releaseAll();
    hid.move(8, 0);
    hid.click(MOUSE_LEFT);
  }
  delay(2000);
}
