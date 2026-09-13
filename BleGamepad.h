#pragma once

#include <Arduino.h>
#include <string>
#include <BLEServer.h>
#include <BLEHIDDevice.h>
#include "HidCore.h"

class BleGamepad : public BLEServerCallbacks {
public:
  explicit BleGamepad(std::string name = "iBle HID Gamepad",
                      std::string manufacturer = "iBle HID",
                      uint8_t battery = 100);
  void begin();
  void end();
  bool isConnected() const;
  HidStatus lastError() const;
  void clearError();
  void setLogging(bool enabled);
  void setReportDelay(uint32_t delayMs);
  void setSecurityEnabled(bool enabled);
  void setAdvertisingInterval(uint16_t minInterval, uint16_t maxInterval);
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
  void clearBonds();

protected:
  void onConnect(BLEServer*) override;
  void onDisconnect(BLEServer*) override;

private:
  struct __attribute__((packed)) Report {
    uint16_t buttons;
    uint8_t hat;
    int16_t x, y, z, rx, ry, rz;
  } report{};
  BLEHIDDevice* hid = nullptr;
  BLECharacteristic* input = nullptr;
  BLEAdvertising* advertising = nullptr;
  std::string name, manufacturer;
  uint8_t battery;
  HidCore core;
  void notify();
  void scheduleReleaseAll();
};
