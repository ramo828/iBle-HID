#include <iBleHID.h>
BleComposite hid;
void setup(){ hid.begin(); }
void loop(){ if(hid.isConnected()){ hid.print("Composite"); hid.write(KEY_RETURN); hid.move(10,10); } delay(2000); }
