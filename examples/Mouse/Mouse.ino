#include <iBleHID.h>
BleMouse mouse;
void setup(){ mouse.begin(); }
void loop(){ if(mouse.isConnected()){ mouse.move(20,0); delay(100); mouse.move(-20,0); mouse.click(1); } delay(1000); }
