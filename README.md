
Modified for multi URL support by n24bass
(merged several new functions, URL scrolling!)

## For the boards with ESP32-PICO-D4, please swap control switch from GPIO16 to GPIO0, since GPIO16 in PICO-D4 is used for internal SPI Flash RAM connection (pre-occupied). Swap can be done in components/controls/controls.c

Add web interface. You can add (up to 10), change or remove URL of the internet radio station. 

```
GET /  - list stations
GET /P - change to previous station
GET /N - change to next station
GET /0..9 - select station
GET /0..9+URL - set station URL
GET /0..-URL - remove station URL
```

Push 'GPIO-16' (chaned from 'boot') switch to change next station.

It starts up only web interface when GPIO-16 is keeped low level at boot time.

----

Modified for OLED display support by kodera2t

Please use latest esp-idf environment (envorinment just before will make lots error)

original code (w/o OLED) is
https://github.com/MrBuddyCasino/ESP32_MP3_Decoder

OLED display mode for WiFi Radio/Bluetooth spaker will be set by menuconfig (select BT speaker or Wifi radio)

Bluetooth device name is defined in bt_config.h in include file folder. (default: "hogehoge_mont")

----
Wiring is same as original, as
ESP pin   - I2S signal
```
----------------------
GPIO25/DAC1   - LRCK
GPIO26/DAC2   - BCLK
GPIO22        - DATA
```
and GPIO25/26 are fixed but GPIO22 can be re-arranged as you wish.
(defined in components/audio_renderer.c)

I2C OLED is connected, as
ESP pin   - I2C signal
```
----------------------
GPIO14   - SCL
GPIO13   - SDA
```
,which defined in app_main.c Please change as you wish...


More details can be found in the original author's explanation at
https://github.com/MrBuddyCasino/ESP32_MP3_Decoder
