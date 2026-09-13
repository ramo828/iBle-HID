#include <iBleHID.h>
BleKeyboard keyboard;
void setup(){ keyboard.begin(); }
void loop(){ if(keyboard.isConnected()){ keyboard.println("Hello from iBle HID"); keyboard.write(KEY_RETURN); } delay(2000); }
