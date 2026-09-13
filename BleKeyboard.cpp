#include "BleKeyboard.h"
#include "HIDTypes.h"
#include <BLEDescriptor.h>
#include <algorithm>
#include <cstring>

#define KID 1
#define MID 2
#define MOID 3
static const uint8_t keyboardMap[] PROGMEM={USAGE_PAGE(1),1,USAGE(1),6,COLLECTION(1),1,REPORT_ID(1),KID,USAGE_PAGE(1),7,USAGE_MINIMUM(1),0xe0,USAGE_MAXIMUM(1),0xe7,LOGICAL_MINIMUM(1),0,LOGICAL_MAXIMUM(1),1,REPORT_SIZE(1),1,REPORT_COUNT(1),8,HIDINPUT(1),2,REPORT_COUNT(1),1,REPORT_SIZE(1),8,HIDINPUT(1),1,REPORT_COUNT(1),6,REPORT_SIZE(1),8,LOGICAL_MINIMUM(1),0,LOGICAL_MAXIMUM(1),0x65,USAGE_MINIMUM(1),0,USAGE_MAXIMUM(1),0x65,HIDINPUT(1),0,END_COLLECTION(0),USAGE_PAGE(1),0x0c,USAGE(1),1,COLLECTION(1),1,REPORT_ID(1),MID,USAGE_PAGE(1),0x0c,USAGE_MINIMUM(1),0,USAGE_MAXIMUM(1),1,REPORT_SIZE(1),1,REPORT_COUNT(1),16,USAGE(1),0xb5,USAGE(1),0xb6,USAGE(1),0xb7,USAGE(1),0xcd,USAGE(1),0xe2,USAGE(1),0xe9,USAGE(1),0xea,USAGE(2),0x23,2,USAGE(2),0x94,1,USAGE(2),0x92,1,USAGE(2),0x2a,2,USAGE(2),0x21,2,USAGE(2),0x26,2,USAGE(2),0x24,2,USAGE(2),0x83,1,USAGE(2),0x8a,1,HIDINPUT(1),2,END_COLLECTION(0)};
// Windows expects the output report and LED usages to match the HID descriptor.
static const uint8_t windowsKeyboardMap[] PROGMEM = {
  USAGE_PAGE(1), 1, USAGE(1), 6, COLLECTION(1), 1,
  REPORT_ID(1), KID, USAGE_PAGE(1), 7, USAGE_MINIMUM(1), 0xe0,
  USAGE_MAXIMUM(1), 0xe7, LOGICAL_MINIMUM(1), 0, LOGICAL_MAXIMUM(1), 1,
  REPORT_SIZE(1), 1, REPORT_COUNT(1), 8, HIDINPUT(1), 2,
  REPORT_COUNT(1), 1, REPORT_SIZE(1), 8, HIDINPUT(1), 1,
  REPORT_COUNT(1), 5, REPORT_SIZE(1), 1, USAGE_PAGE(1), 8,
  USAGE_MINIMUM(1), 1, USAGE_MAXIMUM(1), 5, HIDOUTPUT(1), 2,
  REPORT_COUNT(1), 1, REPORT_SIZE(1), 3, HIDOUTPUT(1), 1,
  REPORT_COUNT(1), 6, REPORT_SIZE(1), 8, LOGICAL_MINIMUM(1), 0,
  LOGICAL_MAXIMUM(1), 0x65, USAGE_PAGE(1), 7, USAGE_MINIMUM(1), 0,
  USAGE_MAXIMUM(1), 0x65, HIDINPUT(1), 0, END_COLLECTION(0),
  USAGE_PAGE(1), 0x0c, USAGE(1), 1, COLLECTION(1), 1,
  REPORT_ID(1), MID, USAGE_PAGE(1), 0x0c, USAGE_MINIMUM(1), 0,
  USAGE_MAXIMUM(1), 1, REPORT_SIZE(1), 1, REPORT_COUNT(1), 16,
  USAGE(1), 0xb5, USAGE(1), 0xb6, USAGE(1), 0xb7, USAGE(1), 0xcd,
  USAGE(1), 0xe2, USAGE(1), 0xe9, USAGE(1), 0xea, USAGE(2), 0x23, 2,
  USAGE(2), 0x94, 1, USAGE(2), 0x92, 1, USAGE(2), 0x2a, 2,
  USAGE(2), 0x21, 2, USAGE(2), 0x26, 2, USAGE(2), 0x24, 2,
  USAGE(2), 0x83, 1, USAGE(2), 0x8a, 1, HIDINPUT(1), 2,
  END_COLLECTION(0)
};
static const uint8_t compositeMap[] PROGMEM = {
#include "composite_descriptor.inc"
};
static const uint8_t mouseMap[]={USAGE_PAGE(1),1,USAGE(1),2,COLLECTION(1),1,REPORT_ID(1),MOID,USAGE(1),1,COLLECTION(1),0,USAGE_PAGE(1),9,USAGE_MINIMUM(1),1,USAGE_MAXIMUM(1),5,LOGICAL_MINIMUM(1),0,LOGICAL_MAXIMUM(1),1,REPORT_COUNT(1),5,REPORT_SIZE(1),1,HIDINPUT(1),2,REPORT_COUNT(1),1,REPORT_SIZE(1),3,HIDINPUT(1),1,USAGE_PAGE(1),1,USAGE(1),0x30,USAGE(1),0x31,USAGE(1),0x38,USAGE_PAGE(1),0x0c,USAGE(2),0x38,2,LOGICAL_MINIMUM(1),0x81,LOGICAL_MAXIMUM(1),0x7f,REPORT_SIZE(1),8,REPORT_COUNT(1),4,HIDINPUT(1),6,END_COLLECTION(0),END_COLLECTION(0)};

static uint8_t asciiToKey(uint8_t value, bool& shifted) {
  shifted = false;
  if (value >= 'a' && value <= 'z') return value - 'a' + 4;
  if (value >= 'A' && value <= 'Z') { shifted = true; return value - 'A' + 4; }
  if (value >= '1' && value <= '9') return value - '1' + 0x1e;
  if (value == '0') return 0x27;
  if (value == ' ') return 0x2c;
  const char characters[] = "-=[]\\;'/.,`";
  const uint8_t codes[] = {0x2d, 0x2f, 0x30, 0x31, 0x31, 0x33, 0x34, 0x38, 0x36, 0x37, 0x35};
  for (uint8_t i = 0; i < sizeof(codes); ++i) if (value == characters[i]) return codes[i];
  if (value == '\n') return 0x28;
  if (value == '\t') return 0x2b;
  return 0;
}

static void configureSecurity() {
  static BLESecurity security;
  security.setAuthenticationMode(ESP_LE_AUTH_BOND);
  security.setCapability(ESP_IO_CAP_NONE);
  security.setInitEncryptionKey(ESP_BLE_ENC_KEY_MASK | ESP_BLE_ID_KEY_MASK);
  security.setRespEncryptionKey(ESP_BLE_ENC_KEY_MASK | ESP_BLE_ID_KEY_MASK);
}

static void clearBondStore() {
#if defined(CONFIG_BLUEDROID_ENABLED)
  int count = esp_ble_get_bond_device_num();
  if (count <= 0) return;
  esp_ble_bond_dev_t* devices = static_cast<esp_ble_bond_dev_t*>(
      malloc(sizeof(esp_ble_bond_dev_t) * count));
  if (!devices) return;
  if (esp_ble_get_bond_device_list(&count, devices) == ESP_OK) {
    for (int i = 0; i < count; ++i) {
      esp_ble_remove_bond_device(devices[i].bd_addr);
    }

  }
  free(devices);
#endif
}

static BLECharacteristic* createReportCharacteristic(BLEHIDDevice* hid,
                                                      uint8_t reportId) {
  BLEService* service = hid ? hid->hidService() : nullptr;
  if (!service) return nullptr;
  const uint32_t properties = BLECharacteristic::PROPERTY_READ |
                              BLECharacteristic::PROPERTY_WRITE |
                              BLECharacteristic::PROPERTY_WRITE_NR |
                              BLECharacteristic::PROPERTY_NOTIFY;
  BLECharacteristic* report =
      service->createCharacteristic((uint16_t)0x2a4d, properties);
  if (!report) return nullptr;
  BLEDescriptor* reference = new BLEDescriptor(BLEUUID((uint16_t)0x2908));
  uint8_t value[] = {reportId, 0x01};
  reference->setValue(value, sizeof(value));
  report->addDescriptor(reference);
  return report;
}

BleKeyboard::BleKeyboard(std::string n, std::string m, uint8_t b)
    : name(n.substr(0, 15)), manufacturer(m.substr(0, 15)), battery(std::min<uint8_t>(b, 100)) {}

void BleKeyboard::setup(const uint8_t* map, size_t length, uint16_t appearance) {
  if (core.beginGuard() != HidStatus::Ok) return;
  BLEDevice::init(String(name.c_str()));
  BLEServer* server = BLEDevice::createServer();
  if (!server) { core.setError(HidStatus::BluetoothUnavailable); core.endGuard(); return; }
  core.setServer(server);
  server->setCallbacks(this);
  hid = new BLEHIDDevice(server);
  // Use one report characteristic: NimBLE detaches duplicate 0x2A4D objects.
  keyboard = createReportCharacteristic(hid, KID);
  output = keyboard;
  media = keyboard;
  if (!keyboard) {
    core.setError(HidStatus::BluetoothUnavailable);
    core.endGuard();
    return;
  }
  output->setCallbacks(this);
  hid->manufacturer()->setValue(String(manufacturer.c_str()));
  hid->pnp(2, vid, pid, version);
  hid->hidInfo(0, 1);
  if (BLECharacteristic* protocol = hid->protocolMode()) {
    uint8_t reportProtocol = 1;
    protocol->setValue(&reportProtocol, sizeof(reportProtocol));
  }
  if (core.configuration().securityEnabled) configureSecurity();
  hid->reportMap((uint8_t*)map, length);
  hid->startServices();
  advertising = server->getAdvertising();
  advertising->setAppearance(appearance);
  advertising->addServiceUUID(hid->hidService()->getUUID());
  advertising->setScanResponse(false);
  core.configureAdvertising(advertising);
  advertising->start();
  hid->setBatteryLevel(battery);
}

void BleKeyboard::begin() { setup(windowsKeyboardMap, sizeof(windowsKeyboardMap), HID_KEYBOARD); }
void BleKeyboard::end() {
  if (!core.started()) { core.setError(HidStatus::NotStarted); return; }
  if (advertising) advertising->stop();
  releaseAll();
  core.endGuard();
}
bool BleKeyboard::isConnected() const { return core.connected(); }
HidStatus BleKeyboard::lastError() const { return core.lastError(); }
void BleKeyboard::clearError() { core.clearError(); }
void BleKeyboard::setLogging(bool enabled) { core.setLogging(enabled); }
bool BleKeyboard::loggingEnabled() const { return core.loggingEnabled(); }
void BleKeyboard::setReportDelay(uint32_t delayMs) { core.configuration().reportDelayMs = delayMs; }
void BleKeyboard::setSecurityEnabled(bool enabled) { core.configuration().securityEnabled = enabled; }
void BleKeyboard::setAdvertisingInterval(uint16_t min, uint16_t max) { core.configuration().advertisingMinInterval = min; core.configuration().advertisingMaxInterval = max; }
void BleKeyboard::setBatteryLevel(uint8_t level) { battery = std::min<uint8_t>(level, 100); if (hid) hid->setBatteryLevel(battery); }
void BleKeyboard::setName(std::string n) { name = n.substr(0, 15); }
void BleKeyboard::clearBonds() {
  clearBondStore();
}
void BleKeyboard::set_vendor_id(uint16_t value) { vid = value; }
void BleKeyboard::set_product_id(uint16_t value) { pid = value; }
void BleKeyboard::set_version(uint16_t value) { version = value; }
void BleKeyboard::onLedState(HidLedCallback callback) { ledCallback = callback; }
uint8_t BleKeyboard::ledState() const { return ledStateValue; }

bool BleKeyboard::notify(BLECharacteristic* characteristic, const uint8_t* data, size_t length) { return core.notify(characteristic, data, length); }
void BleKeyboard::sendReport(KeyReport* value) { if (value) notify(keyboard, (uint8_t*)value, sizeof(*value)); }
void BleKeyboard::sendReport(MediaKeyReport* value) { if (value) notify(media, (uint8_t*)value, sizeof(*value)); }
size_t BleKeyboard::press(uint8_t key) {
  bool shifted = false;
  if (key >= 128) { if (key < 136) report.modifiers |= 1 << (key - 128); else key -= 136; }
  else { key = asciiToKey(key, shifted); if (!key) { setWriteError(); return 0; } if (shifted) report.modifiers |= 2; }
  if (!key) { sendReport(&report); return 1; }
  for (uint8_t current : report.keys) if (current == key) return 1;
  for (uint8_t& current : report.keys) if (!current) { current = key; sendReport(&report); return 1; }
  core.setError(HidStatus::InvalidArgument); setWriteError(); return 0;
}
size_t BleKeyboard::release(uint8_t key) {
  bool shifted = false;
  if (key >= 128) { if (key < 136) report.modifiers &= ~(1 << (key - 128)); else key -= 136; }
  else { key = asciiToKey(key, shifted); if (!key) return 0; if (shifted) report.modifiers &= ~2; }
  for (uint8_t& current : report.keys) if (current == key) current = 0;
  sendReport(&report); return 1;
}
size_t BleKeyboard::press(const MediaKeyReport key) { mediaReport[0] |= key[0]; mediaReport[1] |= key[1]; sendReport(&mediaReport); return 1; }
size_t BleKeyboard::release(const MediaKeyReport key) { mediaReport[0] &= ~key[0]; mediaReport[1] &= ~key[1]; sendReport(&mediaReport); return 1; }
void BleKeyboard::releaseAll() {
  memset(&report, 0, sizeof(report));
  memset(mediaReport, 0, sizeof(mediaReport));
  if (core.connected()) {
    sendReport(&report);
    sendReport(&mediaReport);
  }
}
void BleKeyboard::scheduleReleaseAll() {
  core.defer([this]() {
    if (core.started() && core.connected()) {
      releaseAll();
    }
  }, 200);
}
size_t BleKeyboard::write(uint8_t value) {
  size_t result = press(value);
  release(value);
  return result;
}
size_t BleKeyboard::write(const MediaKeyReport value) {
  size_t result = press(value);
  release(value);
  return result;
}
size_t BleKeyboard::write(const uint8_t* buffer, size_t size) {
  size_t written = 0;
  while (size--) {
    if (*buffer != '\r') written += write(*buffer);
    ++buffer;
  }
  return written;
}
void BleKeyboard::onConnect(BLEServer*) {
  core.setConnected(true);
  core.setRestartPending(false);
  // Windows may finish HID service discovery after the connect callback.
  scheduleReleaseAll();
}
void BleKeyboard::onDisconnect(BLEServer*) {
  core.setConnected(false);
  if (advertising && core.started() && !core.takeRestartPending()) {
    core.setRestartPending(true);
    core.defer([this]() {
      if (advertising && core.started() && !core.connected()) {
        advertising->start();
      }
    }, 200);
  }
}
void BleKeyboard::onWrite(BLECharacteristic* characteristic) {
  if (!characteristic) return;
  String value = characteristic->getValue();
  if (value.length() != 0) {
    ledStateValue = static_cast<uint8_t>(value[0]);
    if (ledCallback) ledCallback(ledStateValue);
  }
}

BleMouse::BleMouse(std::string n, std::string m, uint8_t b)
    : name(n.substr(0, 15)), manufacturer(m.substr(0, 15)),
      battery(std::min<uint8_t>(b, 100)) {}
void BleMouse::begin() {
  if (core.beginGuard() != HidStatus::Ok) return;
  BLEDevice::init(String(name.c_str())); BLEServer* server = BLEDevice::createServer();
  if (!server) { core.setError(HidStatus::BluetoothUnavailable); core.endGuard(); return; }
  server->setCallbacks(this); core.setServer(server); hid = new BLEHIDDevice(server); input = hid->inputReport(MOID);
  hid->manufacturer()->setValue(String(manufacturer.c_str())); hid->pnp(2, 0x05ac, 0x820b, 0x0210); hid->hidInfo(0, 1);
  if (core.configuration().securityEnabled) configureSecurity(); hid->reportMap((uint8_t*)mouseMap, sizeof(mouseMap)); hid->startServices();
  advertising = server->getAdvertising(); advertising->setAppearance(HID_MOUSE); advertising->addServiceUUID(hid->hidService()->getUUID()); advertising->setScanResponse(false); core.configureAdvertising(advertising); advertising->start(); hid->setBatteryLevel(battery);
}
void BleMouse::end() { if (!core.started()) { core.setError(HidStatus::NotStarted); return; } if (advertising) advertising->stop(); core.endGuard(); }
bool BleMouse::isConnected() const { return core.connected(); } HidStatus BleMouse::lastError() const { return core.lastError(); }
void BleMouse::clearError() { core.clearError(); }
void BleMouse::setLogging(bool enabled) { core.setLogging(enabled); } void BleMouse::setReportDelay(uint32_t d) { core.configuration().reportDelayMs = d; }
void BleMouse::setSecurityEnabled(bool enabled) { core.configuration().securityEnabled = enabled; } void BleMouse::setAdvertisingInterval(uint16_t min, uint16_t max) { core.configuration().advertisingMinInterval = min; core.configuration().advertisingMaxInterval = max; }
void BleMouse::setBatteryLevel(uint8_t level) { battery = std::min<uint8_t>(level, 100); if (hid) hid->setBatteryLevel(battery); } void BleMouse::setName(std::string n) { name = n.substr(0, 15); }
void BleMouse::clearBonds() { clearBondStore(); }
bool BleMouse::notify() { return core.notify(input, (uint8_t*)&report, sizeof(report)); }
void BleMouse::move(signed char x, signed char y, signed char wheel, signed char hWheel) { report.x=x; report.y=y; report.wheel=wheel; report.hWheel=hWheel; notify(); report.x=report.y=report.wheel=report.hWheel=0; }
void BleMouse::scrollUp(signed char a) { move(0,0,a,0); } void BleMouse::scrollDown(signed char a) { move(0,0,-a,0); } void BleMouse::scrollLeft(signed char a) { move(0,0,0,-a); } void BleMouse::scrollRight(signed char a) { move(0,0,0,a); }
void BleMouse::click(uint8_t b) { press(b); release(b); } void BleMouse::press(uint8_t b) { report.buttons |= b & 0x1f; notify(); } void BleMouse::release(uint8_t b) { report.buttons &= ~(b & 0x1f); notify(); } void BleMouse::releaseAll() { report.buttons=0; notify(); }
void BleMouse::onConnect(BLEServer*) {
  core.setConnected(true);
  core.setRestartPending(false);
  scheduleReleaseAll();
}
void BleMouse::onDisconnect(BLEServer*) {
  core.setConnected(false);
  if (advertising && core.started() && !core.takeRestartPending()) {
    core.setRestartPending(true);
    core.defer([this]() {
      if (advertising && core.started() && !core.connected()) {
        advertising->start();
      }
    }, 200);
  }
}
void BleMouse::scheduleReleaseAll() {
  core.defer([this]() {
    if (core.started() && core.connected()) {
      releaseAll();
    }
  }, 200);
}

BleComposite::BleComposite(std::string n, std::string m, uint8_t b) : BleKeyboard(n, m, b) {}
void BleComposite::begin() {
  if (core.beginGuard() != HidStatus::Ok) return;
  BLEDevice::init(String(name.c_str())); BLEServer* server = BLEDevice::createServer(); if (!server) { core.setError(HidStatus::BluetoothUnavailable); core.endGuard(); return; }
  server->setCallbacks(this); core.setServer(server); hid = new BLEHIDDevice(server); keyboard=createReportCharacteristic(hid, KID); output=keyboard; media=keyboard; mouseInput=keyboard; if (!keyboard) { core.setError(HidStatus::BluetoothUnavailable); core.endGuard(); return; } output->setCallbacks(this);
  if (BLECharacteristic* protocol = hid->protocolMode()) {
    uint8_t reportProtocol = 1;
    protocol->setValue(&reportProtocol, sizeof(reportProtocol));
  }
  hid->manufacturer()->setValue(String(manufacturer.c_str())); hid->pnp(2, vid, pid, version); hid->hidInfo(0,1); if (core.configuration().securityEnabled) configureSecurity();
  hid->reportMap((uint8_t*)compositeMap, sizeof(compositeMap)); hid->startServices(); advertising=server->getAdvertising(); advertising->setAppearance(HID_KEYBOARD); advertising->addServiceUUID(hid->hidService()->getUUID()); advertising->setScanResponse(false); core.configureAdvertising(advertising); advertising->start(); hid->setBatteryLevel(battery);
}
void BleComposite::mouseNotify() { core.notify(mouseInput, (uint8_t*)&mouseReport, sizeof(mouseReport)); }
void BleComposite::move(signed char x,signed char y,signed char w,signed char h) { mouseReport.x=x;mouseReport.y=y;mouseReport.wheel=w;mouseReport.hWheel=h;mouseNotify();mouseReport.x=mouseReport.y=mouseReport.wheel=mouseReport.hWheel=0; }
void BleComposite::scrollUp(signed char a) { move(0,0,a,0); } void BleComposite::scrollDown(signed char a) { move(0,0,-a,0); } void BleComposite::scrollLeft(signed char a) { move(0,0,0,-a); } void BleComposite::scrollRight(signed char a) { move(0,0,0,a); }
void BleComposite::click(uint8_t b) { mousePress(b); mouseRelease(b); } void BleComposite::mousePress(uint8_t b) { mouseReport.buttons |= b & 0x1f; mouseNotify(); } void BleComposite::mouseRelease(uint8_t b) { mouseReport.buttons &= ~(b & 0x1f); mouseNotify(); } void BleComposite::mouseReleaseAll() { mouseReport.buttons=0; mouseNotify(); } bool BleComposite::mouseConnected() const { return isConnected(); }
