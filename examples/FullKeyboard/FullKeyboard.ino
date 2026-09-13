#include <iBleHID.h>
BleKeyboard keyboard("iBle Full Keyboard");
void setup(){ keyboard.begin(); }
void loop(){ if(keyboard.isConnected()){ keyboard.press(KEY_LEFT_CTRL); keyboard.press(KEY_LEFT_ALT); keyboard.press(KEY_DELETE); delay(100); keyboard.releaseAll(); keyboard.print("Full keyboard API"); } delay(3000); }
