#include "BleGamepad.h"
#include <BLEDevice.h>
#include <BLE2902.h>
#include <algorithm>

static const uint8_t gamepadMap[] = {
  USAGE_PAGE(1), 0x01, USAGE(1), 0x05, COLLECTION(1), 0x01,
  REPORT_ID(1), 0x04,
  USAGE_PAGE(1), 0x09, USAGE_MINIMUM(1), 0x01, USAGE_MAXIMUM(1), 0x10,
  LOGICAL_MINIMUM(1), 0, LOGICAL_MAXIMUM(1), 1, REPORT_COUNT(1), 16,
  REPORT_SIZE(1), 1, HIDINPUT(1), 2,
  USAGE_PAGE(1), 0x01, USAGE(1), 0x39, LOGICAL_MINIMUM(1), 0,
  LOGICAL_MAXIMUM(1), 8, REPORT_SIZE(1), 4, REPORT_COUNT(1), 1,
  HIDINPUT(1), 2, REPORT_SIZE(1), 4, REPORT_COUNT(1), 1, HIDINPUT(1), 3,
  USAGE(1), 0x30, USAGE(1), 0x31, USAGE(1), 0x32, USAGE(1), 0x33,
  USAGE(1), 0x34, USAGE(1), 0x35,
  LOGICAL_MINIMUM(2), 0x00, 0x80, LOGICAL_MAXIMUM(2), 0xff, 0x7f,
  REPORT_SIZE(1), 16, REPORT_COUNT(1), 6, HIDINPUT(1), 2,
  END_COLLECTION(0)
};

static void gamepadSecurity() {
  auto* sec = new BLESecurity();
  sec->setAuthenticationMode(ESP_LE_AUTH_BOND);
  sec->setCapability(ESP_IO_CAP_NONE);
  sec->setInitEncryptionKey(ESP_BLE_ENC_KEY_MASK | ESP_BLE_ID_KEY_MASK);
  sec->setRespEncryptionKey(ESP_BLE_ENC_KEY_MASK | ESP_BLE_ID_KEY_MASK);
}

BleGamepad::BleGamepad(std::string n, std::string m, uint8_t b)
    : name(n.substr(0, 15)), manufacturer(m.substr(0, 15)), battery(b) {
  report.hat = 8;
}

void BleGamepad::begin() {
  BLEDevice::init(String(name.c_str()));
  BLEServer* server = BLEDevice::createServer();
  server->setCallbacks(this);
  hid = new BLEHIDDevice(server);
  input = hid->inputReport(4);
  hid->manufacturer()->setValue(String(manufacturer.c_str()));
  hid->pnp(2, 0x05AC, 0x820C, 0x0210);
  hid->hidInfo(0, 1);
  gamepadSecurity();
  hid->reportMap((uint8_t*)gamepadMap, sizeof(gamepadMap));
  hid->startServices();
  advertising = server->getAdvertising();
  advertising->setAppearance(0x03C4);
  advertising->addServiceUUID(hid->hidService()->getUUID());
  advertising->setScanResponse(false);
  advertising->start();
  hid->setBatteryLevel(battery);
}

void BleGamepad::end() { if (advertising) advertising->stop(); connected = false; }
bool BleGamepad::isConnected() const { return connected; }
void BleGamepad::setBatteryLevel(uint8_t level) {
  battery = std::min<uint8_t>(level, 100);
  if (hid) hid->setBatteryLevel(battery);
}
void BleGamepad::setName(std::string n) { name = n.substr(0, 15); }
void BleGamepad::press(uint8_t button) {
  if (button >= 1 && button <= 16) { report.buttons |= (uint16_t)1 << (button - 1); notify(); }
}
void BleGamepad::release(uint8_t button) {
  if (button >= 1 && button <= 16) { report.buttons &= ~((uint16_t)1 << (button - 1)); notify(); }
}
void BleGamepad::releaseAll() { report.buttons = 0; report.hat = 8; notify(); }
void BleGamepad::setAxes(int16_t x, int16_t y, int16_t z, int16_t rx, int16_t ry, int16_t rz) {
  report.x=x; report.y=y; report.z=z; report.rx=rx; report.ry=ry; report.rz=rz; notify();
}
void BleGamepad::setLeftThumb(int16_t x, int16_t y) { report.x=x; report.y=y; notify(); }
void BleGamepad::setRightThumb(int16_t x, int16_t y) { report.rx=x; report.ry=y; notify(); }
void BleGamepad::setHat(uint8_t direction) { report.hat = direction <= 8 ? direction : 8; notify(); }
void BleGamepad::notify() {
  if (connected && input) { input->setValue((uint8_t*)&report, sizeof(report)); input->notify(); }
}
void BleGamepad::onConnect(BLEServer*) { connected = true; }
void BleGamepad::onDisconnect(BLEServer*) { connected = false; if (advertising) advertising->start(); }
