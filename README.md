# Remote sensor monitoring
[![License](https://img.shields.io/badge/license-MIT-red)](https://opensource.org/licenses/MIT)
[![Blog](https://img.shields.io/badge/blog-post-yellow)](https://mirzafahad.github.io/)

:star: Star me on GitHub — it helps!

A remote fabricated-pressure-sensor monitoring system.

### Application
  The code extracts ambient temperature data and epoch time from Openweather.com using there API.
  it also collect sensor environment data from HTU21D humidity and temperature sensor.
  Then Upload those to sparkfun webserver (data.sparkfun.com), which was later analyzed by analog.io.
  Also an app was built with Blynk App (www.blynk.cc) for android device.
  It exhibits data in the blynk app when requested and a relay can be controlled from the app. 

### Hardware 
  I used Sparkfun Thing dev board based on esp8266. Arduino framework was used to develop the firmware.
  I2C was used to communicate with HTU21D.

