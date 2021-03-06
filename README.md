Indoor Climate Sensor
=======================

Simple [ESPHome](https://esphome.io/) based indoor climate sensor using Sensirion SCD41 CO2 sensor, with buzzer and color LED.

Usage
--------------

[Install esp command line tools](https://esphome.io/guides/installing_esphome.html)

Connect esp32 and run:
```
esphome run 0-sensorita.yaml
```
After first run OTA updates are possible.

Tested with esphome `2022.6.1`

Hardware used
---------------

* [FireBeetle ESP32-E](https://wiki.dfrobot.com/FireBeetle_Board_ESP32_E_SKU_DFR0654)
* [Sparkfun 8mm Adressable RGB LED (WS2812)](https://www.sparkfun.com/products/12877#reviews)
* [TDK PS1240P02BT Piezzo Buzzer](https://www.digikey.se/en/products/detail/tdk-corporation/PS1240P02BT/935930)
* [SCD41 Sensor](https://www.digikey.se/en/products/detail/sensirion-ag/SEK-SCD41-SENSOR/13684004)

Notes on LED wiring
--------------
Should probably put a resistor on the datapin. Didn't do so so far and things seem to work fine.
In a similar fashion, common wisdom seems to be to add a capacitor before (and smaller between) the LEDs.
Again, it seems I'm getting away with it just fine, but notably I'm also driving only a single LED so not too surprising.

(See guides by [Sparkfun](https://learn.sparkfun.com/tutorials/ws2812-breakout-hookup-guide#addressable-through-hole-led) and [Adafruit](https://learn.adafruit.com/adafruit-neopixel-uberguide))

WS2818 LEDs usually require 5V both on Data and Vin.
I tried with both 5V and 3.3V supply and it worked fine without any flickering.
The [specs](https://cdn.sparkfun.com/assets/a/b/1/e/1/DS-12877-LED_-_RGB_Addressable__PTH__8mm_Diffused__5_Pack_.pdf) that Sparkfun linked to the LEDs I bought actually says 2.5V - 7.5V which implies that I got lucky again? (admittedly I read up on all of this only after the fact). But it also says that the data voltage should be somewhere around the supply voltage, so going with that now (i.e. having input voltage on 3.3V since I can't do much about data without an extra voltage regulator).


Note on update interval
--------------

Response times to change of the sensor are 

* CO2: 60s
* Humidity: 90s
* Temperature: 120s

Response time define as "Time for achieving 63% of a respective step function".
(see [datasheet](https://sensirion.com/media/documents/C4B87CE6/61652F80/Sensirion_CO2_Sensors_SCD4x_Datasheet.pdf))

Periodic measurement time is 5 seconds.
There is also a power saving mode in which measurements are taken only every 30 seconds.

Note on calibration
--------------

The SCD41 calibrates itself by default, but for this to work it needs to be exposed to ~400PPM (i.e. outdoors) at least once a week. This [blog](https://emariete.com/en/sensor-co2-sensirion-scd40-scd41-2/#Calibracion_automatica) further notes that:

> Although, we generally talk about the sensor taking 7 days to calibrate, to be precise, what you need are 7 good measurements separated by at least 18 hours (that is, approximately 5 days).

Generally, any settings are not written to the EEPROM (which only lives for ~2000 write cycles!). Calibration history however is automatically stored in a separate EEPROM "dimensioned for the specified sensor lifetime".

Factory default for temperature offset is 4C because of assumed self-heating.
CO2 measurement accuracy is independent of temperature offset


As automatic calibration requires outdoor air every few days while the sensor is on, I disabled automatic self calibration and instead manually calibrated the sensor. (This requires esphome 2022.5.0 or newer which adds manual calibration to scd4x)
