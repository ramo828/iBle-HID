# iBle HID

iBle HID is a Bluetooth LE HID library for Arduino-ESP32 3.3.x (including
ESP32-S3). It provides keyboard, mouse, gamepad, and keyboard+mouse composite
devices using the BLE implementation included in the ESP32 core.

All examples include the unique `<iBleHID.h>` umbrella header. This prevents
Arduino from selecting an older library that happens to provide
`BleKeyboard.h`; sketches should use the same include when combining devices.

## Installation

Copy this directory to `Arduino/libraries/iBle_HID`, restart the Arduino IDE,
and select an ESP32
board. Do not install NimBLE-Arduino for this library. The library targets the
ESP32 core 3.3.11 and later 3.x releases.

## Quick start

```cpp
#include <iBleHID.h>
BleKeyboard keyboard;
void setup() { keyboard.begin(); }
void loop() {
  if (keyboard.isConnected()) keyboard.println("Hello");
  delay(1000);
}
```

## API

All devices expose `lastError()` and `clearError()` using `HidStatus`, plus
`setLogging(bool)`, guarded idempotent `begin()`/`end()`, and
`updateBattery(0..100)`. Reports attempted while disconnected return
`HidStatus::NotConnected` instead of failing silently. `setReportDelay()`,
`setSecurityEnabled()`, and `setAdvertisingInterval(min, max)` can be used
before `begin()` to tune production deployments.

`BleKeyboard` supports `print`, `println`, `write`, `press`, `release`,
`releaseAll`, media key constants, `isConnected`, `setBatteryLevel`, `setName`,
`setDelay`, `clearBonds`, and VID/PID/version setters. Register an output-report callback
with `onLedState()` to receive Num Lock, Caps Lock, and Scroll Lock state.
`BleMouse` (include
`<BleMouse.h>`) provides `move(x,y,wheel,hWheel)`, `scrollUp`, `scrollDown`,
`scrollLeft`, `scrollRight`, `click`, `press`, `release`, `releaseAll`,
battery, name, and connection methods. Scroll amounts default to one HID
wheel unit.

`BleGamepad` (include `<BleGamepad.h>`) provides 16 buttons (`press(1)` through
`press(16)`), six signed 16-bit axes through `setAxes`, thumb helpers,
`setHat`/`setDpad` (0–7 directions, 8 released), battery, name, and connection
methods. Reports are sent only while connected.

`BleComposite` combines the keyboard and mouse APIs. Mouse button masks are
`MOUSE_LEFT`, `MOUSE_RIGHT`, `MOUSE_MIDDLE`, `MOUSE_BACK`, and
`MOUSE_FORWARD`. Keyboard constants include `KEY_LEFT_CTRL`, arrows, function
keys, keypad keys, and media keys.

## Examples

BasicKeyboard, FullKeyboard, MediaKeys, Mouse, DirectionalScrolling,
RotaryEncoderScroll, MPU6050MotionMouse, Composite, Diagnostic, Gamepad,
AdvancedControls, AdvancedConfiguration, ErrorHandling, and KeyboardLeds are
included. Pair the board in the host's
Bluetooth settings; sketches automatically restart advertising after a
disconnect.

## Troubleshooting

* **No device:** call `begin()` once in `setup()` and wait for advertising.
* **No input:** check `isConnected()` and remove an old bonded device before
  pairing again.
* **Mouse type not found:** include `<BleMouse.h>` and ensure the iBle HID
  folder is in the Arduino libraries directory.
* **Compile errors:** use Arduino-ESP32 3.3.11, select an ESP32 target, and
  remove duplicate copies of this library or incompatible BLE libraries.
* **Pairing problems:** call `clearBonds()` before `begin()` when the host has
  stale pairing data, then forget the device on the host and reset the board.
  The library uses bonded Just Works pairing without a passkey. Advertising
  restarts from deferred work after disconnect, rather than from the BLE
  callback, to avoid reconnect loops.

## Advanced configuration

Constructors accept device name, manufacturer, and battery level. Device names
and manufacturer strings are limited to 15 characters. Keyboard reports have
stable report IDs, and composite devices expose keyboard, media, and mouse
reports in one HID service. Battery levels are clamped to 0–100. Configure
custom keyboard timing with `setDelay()` and identity with
`set_vendor_id()`, `set_product_id()`, and `set_version()`.

## License

See the source distribution for licensing terms.
