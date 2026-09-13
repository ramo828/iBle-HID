#include <Wire.h>
#include <iBleHID.h>

constexpr uint8_t MPU6050_ADDRESS = 0x68;
BleMouse mouse("iBle Motion Mouse");

void setup() {
  Wire.begin();
  Wire.beginTransmission(MPU6050_ADDRESS);
  Wire.write(0x6B);
  Wire.write(0);
  Wire.endTransmission(true);
  mouse.begin();
}

int16_t readAxis(uint8_t reg) {
  Wire.beginTransmission(MPU6050_ADDRESS);
  Wire.write(reg);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU6050_ADDRESS, static_cast<uint8_t>(2), true);
  return static_cast<int16_t>((Wire.read() << 8) | Wire.read());
}

void loop() {
  const int8_t x = static_cast<int8_t>(constrain(readAxis(0x3B) / 4096, -127, 127));
  const int8_t y = static_cast<int8_t>(constrain(readAxis(0x3D) / 4096, -127, 127));
  if (mouse.isConnected() && (x || y)) mouse.move(x, y);
  delay(10);
}
