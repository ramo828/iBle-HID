#ifndef IBLE_HID_BLE_KEYBOARD_H
#define IBLE_HID_BLE_KEYBOARD_H
#include <Arduino.h>
#include <Print.h>
#include <string>
#include "sdkconfig.h"
#if defined(CONFIG_BT_ENABLED)
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLECharacteristic.h>
#include <BLEHIDDevice.h>
#include <BLE2902.h>

#define IBLE_HID_VERSION "1.0.0"
#define BLE_KEYBOARD_VERSION IBLE_HID_VERSION
#define BLE_KEYBOARD_VERSION_MAJOR 1
#define BLE_KEYBOARD_VERSION_MINOR 0
#define BLE_KEYBOARD_VERSION_REVISION 0

#define IBLE_KEY(name, value) static constexpr uint8_t name = value
IBLE_KEY(KEY_LEFT_CTRL,0x80); IBLE_KEY(KEY_LEFT_SHIFT,0x81); IBLE_KEY(KEY_LEFT_ALT,0x82); IBLE_KEY(KEY_LEFT_GUI,0x83);
IBLE_KEY(KEY_RIGHT_CTRL,0x84); IBLE_KEY(KEY_RIGHT_SHIFT,0x85); IBLE_KEY(KEY_RIGHT_ALT,0x86); IBLE_KEY(KEY_RIGHT_GUI,0x87);
IBLE_KEY(KEY_UP_ARROW,0xDA); IBLE_KEY(KEY_DOWN_ARROW,0xD9); IBLE_KEY(KEY_LEFT_ARROW,0xD8); IBLE_KEY(KEY_RIGHT_ARROW,0xD7);
IBLE_KEY(KEY_BACKSPACE,0xB2); IBLE_KEY(KEY_TAB,0xB3); IBLE_KEY(KEY_RETURN,0xB0); IBLE_KEY(KEY_ESC,0xB1); IBLE_KEY(KEY_INSERT,0xD1);
IBLE_KEY(KEY_PRTSC,0xCE); IBLE_KEY(KEY_DELETE,0xD4); IBLE_KEY(KEY_PAGE_UP,0xD3); IBLE_KEY(KEY_PAGE_DOWN,0xD6); IBLE_KEY(KEY_HOME,0xD2); IBLE_KEY(KEY_END,0xD5); IBLE_KEY(KEY_CAPS_LOCK,0xC1);
IBLE_KEY(KEY_F1,0xC2); IBLE_KEY(KEY_F2,0xC3); IBLE_KEY(KEY_F3,0xC4); IBLE_KEY(KEY_F4,0xC5); IBLE_KEY(KEY_F5,0xC6); IBLE_KEY(KEY_F6,0xC7); IBLE_KEY(KEY_F7,0xC8); IBLE_KEY(KEY_F8,0xC9); IBLE_KEY(KEY_F9,0xCA); IBLE_KEY(KEY_F10,0xCB); IBLE_KEY(KEY_F11,0xCC); IBLE_KEY(KEY_F12,0xCD);
IBLE_KEY(KEY_F13,0xF0); IBLE_KEY(KEY_F14,0xF1); IBLE_KEY(KEY_F15,0xF2); IBLE_KEY(KEY_F16,0xF3); IBLE_KEY(KEY_F17,0xF4); IBLE_KEY(KEY_F18,0xF5); IBLE_KEY(KEY_F19,0xF6); IBLE_KEY(KEY_F20,0xF7); IBLE_KEY(KEY_F21,0xF8); IBLE_KEY(KEY_F22,0xF9); IBLE_KEY(KEY_F23,0xFA); IBLE_KEY(KEY_F24,0xFB);
IBLE_KEY(KEY_NUM_0,0xEA); IBLE_KEY(KEY_NUM_1,0xE1); IBLE_KEY(KEY_NUM_2,0xE2); IBLE_KEY(KEY_NUM_3,0xE3); IBLE_KEY(KEY_NUM_4,0xE4); IBLE_KEY(KEY_NUM_5,0xE5); IBLE_KEY(KEY_NUM_6,0xE6); IBLE_KEY(KEY_NUM_7,0xE7); IBLE_KEY(KEY_NUM_8,0xE8); IBLE_KEY(KEY_NUM_9,0xE9); IBLE_KEY(KEY_NUM_SLASH,0xDC); IBLE_KEY(KEY_NUM_ASTERISK,0xDD); IBLE_KEY(KEY_NUM_MINUS,0xDE); IBLE_KEY(KEY_NUM_PLUS,0xDF); IBLE_KEY(KEY_NUM_ENTER,0xE0); IBLE_KEY(KEY_NUM_PERIOD,0xEB);
using MediaKeyReport = uint8_t[2];
static const MediaKeyReport KEY_MEDIA_NEXT_TRACK={1,0}, KEY_MEDIA_PREVIOUS_TRACK={2,0}, KEY_MEDIA_STOP={4,0}, KEY_MEDIA_PLAY_PAUSE={8,0}, KEY_MEDIA_MUTE={16,0}, KEY_MEDIA_VOLUME_UP={32,0}, KEY_MEDIA_VOLUME_DOWN={64,0}, KEY_MEDIA_WWW_HOME={128,0}, KEY_MEDIA_LOCAL_MACHINE_BROWSER={0,1}, KEY_MEDIA_CALCULATOR={0,2}, KEY_MEDIA_WWW_BOOKMARKS={0,4}, KEY_MEDIA_WWW_SEARCH={0,8}, KEY_MEDIA_WWW_STOP={0,16}, KEY_MEDIA_WWW_BACK={0,32}, KEY_MEDIA_CONSUMER_CONTROL_CONFIGURATION={0,64}, KEY_MEDIA_EMAIL_READER={0,128};
struct KeyReport { uint8_t modifiers; uint8_t reserved; uint8_t keys[6]; };

class BleKeyboard : public Print, public BLEServerCallbacks, public BLECharacteristicCallbacks {
public:
 BleKeyboard(std::string name="iBle Keyboard", std::string manufacturer="iBle HID", uint8_t battery=100);
 void begin(); void end(); bool isConnected() const; void setBatteryLevel(uint8_t); void setName(std::string); void setDelay(uint32_t); void set_vendor_id(uint16_t); void set_product_id(uint16_t); void set_version(uint16_t);
 void sendReport(KeyReport*); void sendReport(MediaKeyReport*); size_t press(uint8_t); size_t press(const MediaKeyReport); size_t release(uint8_t); size_t release(const MediaKeyReport); size_t write(uint8_t); size_t write(const MediaKeyReport); size_t write(const uint8_t*,size_t); void releaseAll();
protected: void onConnect(BLEServer*) override; void onDisconnect(BLEServer*) override; void onWrite(BLECharacteristic*) override;
protected: BLEHIDDevice* hid=nullptr; BLECharacteristic* keyboard=nullptr; BLECharacteristic* media=nullptr; BLECharacteristic* output=nullptr; BLEAdvertising* advertising=nullptr; KeyReport report{}; MediaKeyReport mediaReport{}; std::string name,manufacturer; uint8_t battery; uint32_t delayMs=7; uint16_t vid=0x05ac,pid=0x820a,version=0x0210; volatile bool connected=false; void setup(const uint8_t*,size_t,uint8_t); void notify(BLECharacteristic*,const uint8_t*,size_t);
};

static constexpr uint8_t MOUSE_LEFT=1, MOUSE_RIGHT=2, MOUSE_MIDDLE=4, MOUSE_BACK=8, MOUSE_FORWARD=16;
struct MouseReport { uint8_t buttons; int8_t x; int8_t y; int8_t wheel; int8_t hWheel; };
class BleMouse : public Print, public BLEServerCallbacks { public: BleMouse(std::string name="iBle Mouse",std::string manufacturer="iBle HID",uint8_t battery=100); void begin(); void end(); bool isConnected() const; void click(uint8_t button=1); void move(signed char x,signed char y,signed char wheel=0,signed char hWheel=0); void scrollUp(signed char amount=1); void scrollDown(signed char amount=1); void scrollLeft(signed char amount=1); void scrollRight(signed char amount=1); void press(uint8_t); void release(uint8_t); void releaseAll(); void setBatteryLevel(uint8_t); void setName(std::string); size_t write(uint8_t) override { return 0; } protected: void onConnect(BLEServer*) override; void onDisconnect(BLEServer*) override; private: BLEHIDDevice* hid=nullptr; BLECharacteristic* input=nullptr; BLEAdvertising* advertising=nullptr; MouseReport report{}; std::string name,manufacturer; uint8_t battery; volatile bool connected=false; void notify(); };

class BleComposite : public BleKeyboard { public: BleComposite(std::string name="iBle Composite",std::string manufacturer="iBle HID",uint8_t battery=100); void begin(); void move(signed char x,signed char y,signed char wheel=0,signed char hWheel=0); void scrollUp(signed char amount=1); void scrollDown(signed char amount=1); void scrollLeft(signed char amount=1); void scrollRight(signed char amount=1); void click(uint8_t button=1); void mousePress(uint8_t); void mouseRelease(uint8_t); void mouseReleaseAll(); bool mouseConnected() const; private: BLECharacteristic* mouseInput=nullptr; MouseReport mouseReport{}; void mouseNotify(); };
using BleKeyboardMouse = BleComposite;
#endif
#endif
