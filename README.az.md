# iBle HID

iBle HID Arduino-ESP32 3.3.x (ESP32-S3 daxil olmaqla) üçün Bluetooth LE HID
kitabxanasıdır. Klaviatura, siçan, gamepad və kompozit klaviatura+siçan
cihazlarını dəstəkləyir.

Bütün nümunələr unikal `<iBleHID.h>` umbrella başlığından istifadə edir. Bu,
Arduino-nun köhnə `BleKeyboard.h` kitabxanasını seçməsinin qarşısını alır.

## Quraşdırma

Bu qovluğu `Arduino/libraries/iBle HID` ünvanına köçürün (qovluq adı
`iBle HID` də qala bilər), Arduino IDE-ni yenidən başladın və ESP32
lövhəsi seçin. NimBLE-Arduino quraşdırmayın; kitabxana ESP32 nüvəsinin BLE
implementasiyasından istifadə edir. Arduino-ESP32 3.3.11 və sonrakı 3.x
versiyaları dəstəklənir.

## Sürətli başlanğıc

```cpp
#include <iBleHID.h>
BleKeyboard keyboard;
void setup() { keyboard.begin(); }
void loop() { if (keyboard.isConnected()) keyboard.println("Salam"); delay(1000); }
```

## API və nümunələr

`BleKeyboard` `print`, `println`, `write`, `press`, `release`, `releaseAll`,
media düymələri, batareya, ad və VID/PID sazlamalarını verir. `BleMouse`
üçün `<BleMouse.h>` daxil edin; `move`, `scrollUp`, `scrollDown`, `scrollLeft`,
`scrollRight`, `click`, `press`, `release` və `releaseAll` metodları
mövcuddur. `BleGamepad` 16 düymə, altı ox,
`setLeftThumb`, `setRightThumb` və `setHat`/`setDpad` verir. `BleComposite`
klaviatura və siçanı bir cihazda birləşdirir.

BasicKeyboard, FullKeyboard, MediaKeys, Mouse, DirectionalScrolling,
RotaryEncoderScroll, MPU6050MotionMouse, Composite, Diagnostic, Gamepad və
AdvancedControls nümunələri `examples` qovluğundadır.

## Problemlərin həlli və inkişaf etmiş sazlama

Cihaz görünmürsə `begin()` çağırıldığını və Bluetooth reklamının başladığını
yoxlayın. Giriş yoxdursa `isConnected()` yoxlayın və köhnə qoşmanı silib
yenidən qoşulun. Uyğun olmayan BLE kitabxanalarını və təkrarlanan iBle HID
qovluqlarını silin. Ad və istehsalçı maksimum 15 simvoldur; klaviatura üçün
`setDelay()`, `set_vendor_id()`, `set_product_id()` və `set_version()`
metodlarından istifadə edin. Batareya 0–100 aralığına məhdudlaşdırılır.
