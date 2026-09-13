#include <iBleHID.h>
BleKeyboard keyboard("iBle Media");
void setup(){ keyboard.begin(); }
void loop(){ if(keyboard.isConnected()){ keyboard.write(KEY_MEDIA_PLAY_PAUSE); delay(100); keyboard.write(KEY_MEDIA_VOLUME_UP); } delay(3000); }
