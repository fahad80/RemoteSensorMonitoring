# README #

A remote own-fabricated pressure sensor monitoring device
 
Last edited: May 4th , 2016 (May the 4th be with you!!!)

# Application #
  The code extracts ambient temperature data and epoch time from Openweather.com using there API.
  it also collect sensor environment data from HTU21D humidity and temperature sensor.
  Then Upload those to sparkfun webserver (data.sparkfun.com), which was later analyzed by analog.io.
  Also an app was built with Blynk App (www.blynk.cc) for android device.
  It exhibits data in the blynk app when requested and a relay can be controlled from the app. 

# Hardware #
  I used a Hardware module based on esp8266 (SParkfun Thing). Arduino IDE(1.6.8) was used to code esp8266 chip.
  It was powered from USB. HTU21D was powered from esp8266's 3.3V supply pin. 
  ESP8266's I2c was used to communicate with HTU21D.


### Author ###

* Fahad Mirza
* fahadmirza80@yahoo.com