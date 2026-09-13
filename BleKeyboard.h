#ifndef IBLE_HID_BLE_KEYBOARD_H
#define IBLE_HID_BLE_KEYBOARD_H

#include <Arduino.h>
#include <Print.h>
#include <BLECharacteristic.h>
#include <BLEDevice.h>
#include <BLEHIDDevice.h>
#include <BLEServer.h>

#include <functional>
#include <string>

#include "HidCore.h"

#define IBLE_HID_VERSION "1.1.1"
#define BLE_KEYBOARD_VERSION IBLE_HID_VERSION
#define BLE_KEYBOARD_VERSION_MAJOR 1
#define BLE_KEYBOARD_VERSION_MINOR 1
#define BLE_KEYBOARD_VERSION_REVISION 1

#define IBLE_KEY(name, value) static constexpr uint8_t name = value
IBLE_KEY(KEY_LEFT_CTRL, 0x80);
IBLE_KEY(KEY_LEFT_SHIFT, 0x81);
IBLE_KEY(KEY_LEFT_ALT, 0x82);
IBLE_KEY(KEY_LEFT_GUI, 0x83);
IBLE_KEY(KEY_RIGHT_CTRL, 0x84);
IBLE_KEY(KEY_RIGHT_SHIFT, 0x85);
IBLE_KEY(KEY_RIGHT_ALT, 0x86);
IBLE_KEY(KEY_RIGHT_GUI, 0x87);
IBLE_KEY(KEY_UP_ARROW, 0xDA);
IBLE_KEY(KEY_DOWN_ARROW, 0xD9);
IBLE_KEY(KEY_LEFT_ARROW, 0xD8);
IBLE_KEY(KEY_RIGHT_ARROW, 0xD7);
IBLE_KEY(KEY_BACKSPACE, 0xB2);
IBLE_KEY(KEY_TAB, 0xB3);
IBLE_KEY(KEY_RETURN, 0xB0);
IBLE_KEY(KEY_ESC, 0xB1);
IBLE_KEY(KEY_INSERT, 0xD1);
IBLE_KEY(KEY_PRTSC, 0xCE);
IBLE_KEY(KEY_DELETE, 0xD4);
IBLE_KEY(KEY_PAGE_UP, 0xD3);
IBLE_KEY(KEY_PAGE_DOWN, 0xD6);
IBLE_KEY(KEY_HOME, 0xD2);
IBLE_KEY(KEY_END, 0xD5);
IBLE_KEY(KEY_CAPS_LOCK, 0xC1);
IBLE_KEY(KEY_F1, 0xC2);
IBLE_KEY(KEY_F2, 0xC3);
IBLE_KEY(KEY_F3, 0xC4);
IBLE_KEY(KEY_F4, 0xC5);
IBLE_KEY(KEY_F5, 0xC6);
IBLE_KEY(KEY_F6, 0xC7);
IBLE_KEY(KEY_F7, 0xC8);
IBLE_KEY(KEY_F8, 0xC9);
IBLE_KEY(KEY_F9, 0xCA);
IBLE_KEY(KEY_F10, 0xCB);
IBLE_KEY(KEY_F11, 0xCC);
IBLE_KEY(KEY_F12, 0xCD);

using MediaKeyReport = uint8_t[2];
static const MediaKeyReport KEY_MEDIA_NEXT_TRACK = {1, 0};
static const MediaKeyReport KEY_MEDIA_PREVIOUS_TRACK = {2, 0};
static const MediaKeyReport KEY_MEDIA_STOP = {4, 0};
static const MediaKeyReport KEY_MEDIA_PLAY_PAUSE = {8, 0};
static const MediaKeyReport KEY_MEDIA_MUTE = {16, 0};
static const MediaKeyReport KEY_MEDIA_VOLUME_UP = {32, 0};
static const MediaKeyReport KEY_MEDIA_VOLUME_DOWN = {64, 0};

struct KeyReport {
  uint8_t modifiers;
  uint8_t reserved;
  uint8_t keys[6];
};

using HidLedCallback = std::function<void(uint8_t)>;

class BleKeyboard : public Print,
                    public BLEServerCallbacks,
                    public BLECharacteristicCallbacks {
 public:
  BleKeyboard(std::string name = "iBle Keyboard",
              std::string manufacturer = "iBle HID", uint8_t battery = 100);
  void begin();
  void end();
  bool isConnected() const;
  HidStatus lastError() const;
  void clearError();
  void setLogging(bool enabled);
  bool loggingEnabled() const;
  void setReportDelay(uint32_t delayMs);
  void setSecurityEnabled(bool enabled);
  void setAdvertisingInterval(uint16_t minInterval, uint16_t maxInterval);
  void setBatteryLevel(uint8_t level);
  void updateBattery(uint8_t level) { setBatteryLevel(level); }
  void setName(std::string name);
  void setDelay(uint32_t delayMs) { setReportDelay(delayMs); }
  void set_vendor_id(uint16_t value);
  void set_product_id(uint16_t value);
  void set_version(uint16_t value);
  void onLedState(HidLedCallback callback);
  uint8_t ledState() const;
  void sendReport(KeyReport* report);
  void sendReport(MediaKeyReport* report);
  size_t press(uint8_t key);
  size_t press(const MediaKeyReport report);
  size_t release(uint8_t key);
  size_t release(const MediaKeyReport report);
  size_t write(uint8_t value) override;
  size_t write(const MediaKeyReport report);
  size_t write(const uint8_t* buffer, size_t size) override;
  void releaseAll();

 protected:
  void onConnect(BLEServer*) override;
  void onDisconnect(BLEServer*) override;
  void onWrite(BLECharacteristic*) override;

  BLEHIDDevice* hid = nullptr;
  BLECharacteristic* keyboard = nullptr;
  BLECharacteristic* media = nullptr;
  BLECharacteristic* output = nullptr;
  BLEAdvertising* advertising = nullptr;
  KeyReport report{};
  MediaKeyReport mediaReport{};
  std::string name;
  std::string manufacturer;
  uint8_t battery;
  uint16_t vid = 0x05AC;
  uint16_t pid = 0x820A;
  uint16_t version = 0x0210;
  HidCore core;
  void setup(const uint8_t* map, size_t length, uint16_t appearance);
  bool notify(BLECharacteristic* characteristic, const uint8_t* data,
              size_t length);
  HidLedCallback ledCallback;
  uint8_t ledStateValue = 0;
};

static constexpr uint8_t MOUSE_LEFT = 1;
static constexpr uint8_t MOUSE_RIGHT = 2;
static constexpr uint8_t MOUSE_MIDDLE = 4;
static constexpr uint8_t MOUSE_BACK = 8;
static constexpr uint8_t MOUSE_FORWARD = 16;

struct MouseReport {
  uint8_t buttons;
  int8_t x;
  int8_t y;
  int8_t wheel;
  int8_t hWheel;
};

class BleMouse : public Print, public BLEServerCallbacks {
 public:
  BleMouse(std::string name = "iBle Mouse",
           std::string manufacturer = "iBle HID", uint8_t battery = 100);
  void begin();
  void end();
  bool isConnected() const;
  HidStatus lastError() const;
  void clearError();
  void setLogging(bool enabled);
  void setReportDelay(uint32_t delayMs);
  void setSecurityEnabled(bool enabled);
  void setAdvertisingInterval(uint16_t minInterval, uint16_t maxInterval);
  void setBatteryLevel(uint8_t level);
  void updateBattery(uint8_t level) { setBatteryLevel(level); }
  void setName(std::string name);
  void click(uint8_t button = MOUSE_LEFT);
  void move(signed char x, signed char y, signed char wheel = 0,
            signed char hWheel = 0);
  void scrollUp(signed char amount = 1);
  void scrollDown(signed char amount = 1);
  void scrollLeft(signed char amount = 1);
  void scrollRight(signed char amount = 1);
  void press(uint8_t button);
  void release(uint8_t button);
  void releaseAll();
  size_t write(uint8_t) override { return 0; }

 protected:
  void onConnect(BLEServer*) override;
  void onDisconnect(BLEServer*) override;
  BLEHIDDevice* hid = nullptr;
  BLECharacteristic* input = nullptr;
  BLEAdvertising* advertising = nullptr;
  MouseReport report{};
  std::string name;
  std::string manufacturer;
  uint8_t battery;
  HidCore core;
  bool notify();
};

class BleComposite : public BleKeyboard {
 public:
  BleComposite(std::string name = "iBle Composite",
               std::string manufacturer = "iBle HID", uint8_t battery = 100);
  void begin();
  void move(signed char x, signed char y, signed char wheel = 0,
            signed char hWheel = 0);
  void scrollUp(signed char amount = 1);
  void scrollDown(signed char amount = 1);
  void scrollLeft(signed char amount = 1);
  void scrollRight(signed char amount = 1);
  void click(uint8_t button = MOUSE_LEFT);
  void mousePress(uint8_t button);
  void mouseRelease(uint8_t button);
  void mouseReleaseAll();
  bool mouseConnected() const;

 private:
  BLECharacteristic* mouseInput = nullptr;
  MouseReport mouseReport{};
  void mouseNotify();
};

using BleKeyboardMouse = BleComposite;

#endif
