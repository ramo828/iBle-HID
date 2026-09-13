#include <iBleHID.h>
BleComposite hid("iBle Diagnostic");
void setup(){ Serial.begin(115200); hid.begin(); }
void loop(){ Serial.printf("connected=%s mouse=%s free=%u\n",hid.isConnected()?"yes":"no",hid.mouseConnected()?"yes":"no",ESP.getFreeHeap()); delay(1000); }
