#include "BleGamepad.h"

#include <BLE2902.h>
#include <BLEDevice.h>
#include <algorithm>

static const uint8_t gamepadMap[] = {
  USAGE_PAGE(1), 0x01, USAGE(1), 0x05, COLLECTION(1), 0x01,
  REPORT_ID(1), 0x04, USAGE_PAGE(1), 0x09, USAGE_MINIMUM(1), 0x01,
  USAGE_MAXIMUM(1), 0x10, LOGICAL_MINIMUM(1), 0, LOGICAL_MAXIMUM(1), 1,
  REPORT_COUNT(1), 16, REPORT_SIZE(1), 1, HIDINPUT(1), 2,
  USAGE_PAGE(1), 0x01, USAGE(1), 0x39, LOGICAL_MINIMUM(1), 0,
  LOGICAL_MAXIMUM(1), 8, REPORT_SIZE(1), 4, REPORT_COUNT(1), 1,
  HIDINPUT(1), 2, REPORT_SIZE(1), 4, REPORT_COUNT(1), 1, HIDINPUT(1), 3,
  USAGE(1), 0x30, USAGE(1), 0x31, USAGE(1), 0x32, USAGE(1), 0x33,
  USAGE(1), 0x34, USAGE(1), 0x35, LOGICAL_MINIMUM(2), 0x00, 0x80,
  LOGICAL_MAXIMUM(2), 0xff, 0x7f, REPORT_SIZE(1), 16, REPORT_COUNT(1), 6,
  HIDINPUT(1), 2, END_COLLECTION(0)
};

static void configureSecurity() {
  static BLESecurity security;
  security.setAuthenticationMode(ESP_LE_AUTH_BOND);
  security.setCapability(ESP_IO_CAP_NONE);
  security.setInitEncryptionKey(ESP_BLE_ENC_KEY_MASK | ESP_BLE_ID_KEY_MASK);
  security.setRespEncryptionKey(ESP_BLE_ENC_KEY_MASK | ESP_BLE_ID_KEY_MASK);
}

BleGamepad::BleGamepad(std::string n, std::string m, uint8_t b)
    : name(n.substr(0, 15)), manufacturer(m.substr(0, 15)),
      battery(std::min<uint8_t>(b, 100)) {
  report.hat = 8;
}

void BleGamepad::begin() {
  if (core.beginGuard() != HidStatus::Ok) {
    return;
  }
  BLEDevice::init(String(name.c_str()));
  BLEServer* server = BLEDevice::createServer();
  if (!server) {
    core.setError(HidStatus::BluetoothUnavailable);
    core.endGuard();
    return;
  }
  server->setCallbacks(this);
  hid = new BLEHIDDevice(server);
  input = hid->inputReport(4);
  hid->manufacturer()->setValue(String(manufacturer.c_str()));
  hid->pnp(2, 0x05AC, 0x820C, 0x0210);
  hid->hidInfo(0, 1);
  if (core.configuration().securityEnabled) {
    configureSecurity();
  }
  hid->reportMap((uint8_t*)gamepadMap, sizeof(gamepadMap));
  hid->startServices();
  advertising = server->getAdvertising();
  advertising->setAppearance(0x03C4);
  advertising->addServiceUUID(hid->hidService()->getUUID());
  advertising->setScanResponse(false);
  core.configureAdvertising(advertising);
  advertising->start();
  hid->setBatteryLevel(battery);
}

void BleGamepad::end() {
  if (!core.started()) {
    core.setError(HidStatus::NotStarted);
    return;
  }
  if (advertising) {
    advertising->stop();
  }
  releaseAll();
  core.endGuard();
}

bool BleGamepad::isConnected() const { return core.connected(); }
HidStatus BleGamepad::lastError() const { return core.lastError(); }
void BleGamepad::clearError() { core.clearError(); }
void BleGamepad::setLogging(bool enabled) { core.setLogging(enabled); }
void BleGamepad::setReportDelay(uint32_t delayMs) {
  core.configuration().reportDelayMs = delayMs;
}
void BleGamepad::setSecurityEnabled(bool enabled) {
  core.configuration().securityEnabled = enabled;
}
void BleGamepad::setAdvertisingInterval(uint16_t minInterval,
                                        uint16_t maxInterval) {
  core.configuration().advertisingMinInterval = minInterval;
  core.configuration().advertisingMaxInterval = maxInterval;
}

void BleGamepad::setBatteryLevel(uint8_t level) {
  battery = std::min<uint8_t>(level, 100);
  if (hid) {
    hid->setBatteryLevel(battery);
  }
}

void BleGamepad::setName(std::string n) { name = n.substr(0, 15); }

void BleGamepad::press(uint8_t button) {
  if (button < 1 || button > 16) {
    core.setError(HidStatus::InvalidArgument);
    return;
  }
  report.buttons |= static_cast<uint16_t>(1) << (button - 1);
  notify();
}

void BleGamepad::release(uint8_t button) {
  if (button < 1 || button > 16) {
    core.setError(HidStatus::InvalidArgument);
    return;
  }
  report.buttons &= ~(static_cast<uint16_t>(1) << (button - 1));
  notify();
}

void BleGamepad::releaseAll() {
  report.buttons = 0;
  report.hat = 8;
  notify();
}

void BleGamepad::setAxes(int16_t x, int16_t y, int16_t z, int16_t rx,
                         int16_t ry, int16_t rz) {
  report.x = x;
  report.y = y;
  report.z = z;
  report.rx = rx;
  report.ry = ry;
  report.rz = rz;
  notify();
}

void BleGamepad::setLeftThumb(int16_t x, int16_t y) {
  report.x = x;
  report.y = y;
  notify();
}

void BleGamepad::setRightThumb(int16_t x, int16_t y) {
  report.rx = x;
  report.ry = y;
  notify();
}

void BleGamepad::setHat(uint8_t direction) {
  if (direction > 8) {
    core.setError(HidStatus::InvalidArgument);
    return;
  }
  report.hat = direction;
  notify();
}

void BleGamepad::notify() {
  core.notify(input, reinterpret_cast<uint8_t*>(&report), sizeof(report));
}

void BleGamepad::onConnect(BLEServer*) {
  core.setConnected(true);
  core.setRestartPending(false);
  releaseAll();
}

void BleGamepad::onDisconnect(BLEServer*) {
  core.setConnected(false);
  if (advertising && core.started() && !core.takeRestartPending()) {
    core.setRestartPending(true);
    advertising->start();
  }
}
