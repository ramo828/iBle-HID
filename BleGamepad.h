#pragma once

#include <Arduino.h>
#include <string>
#include <BLEServer.h>
#include <BLEHIDDevice.h>

class BleGamepad : public BLEServerCallbacks {
public:
  explicit BleGamepad(std::string name = "iBle HID Gamepad",
                      std::string manufacturer = "iBle HID",
                      uint8_t battery = 100);
  void begin();
  void end();
  bool isConnected() const;
  void press(uint8_t button);
  void release(uint8_t button);
  void releaseAll();
  void setAxes(int16_t x, int16_t y, int16_t z = 0, int16_t rx = 0,
               int16_t ry = 0, int16_t rz = 0);
  void setLeftThumb(int16_t x, int16_t y);
  void setRightThumb(int16_t x, int16_t y);
  void setHat(uint8_t direction);
  void setDpad(uint8_t direction) { setHat(direction); }
  void setBatteryLevel(uint8_t level);
  void setName(std::string name);

protected:
  void onConnect(BLEServer*) override;
  void onDisconnect(BLEServer*) override;

private:
  struct Report {
    uint16_t buttons;
    uint8_t hat;
    int16_t x, y, z, rx, ry, rz;
  } report{};
  BLEHIDDevice* hid = nullptr;
  BLECharacteristic* input = nullptr;
  BLEAdvertising* advertising = nullptr;
  std::string name, manufacturer;
  uint8_t battery;
  volatile bool connected = false;
  void notify();
};
