
#define BME280MY_C
#include "bme280my.h"

#include "serialq.h"        // queue buffer for serial output
#ifndef SERIALQ             //
#define SERIALQ Serial      //
#endif                      //

#include <Arduino.h>
#include <inttypes.h>         //
#include <Wire.h>             // Needed for legacy versions of Arduino.
#include <BME280I2C.h>        // I2C communication to BME280 module
#include "general.h"
#include <ctype.h>            // character types
#include "rtctime.h"





void BME280c::begin() {

  BME280I2C::Settings settings(
    BME280::OSR_X1,
    BME280::OSR_X1,
    BME280::OSR_X1,
    BME280::Mode_Forced,
    BME280::StandbyTime_1000ms,
    BME280::Filter_Off,
    BME280::SpiEnable_False,
    BME280I2C::I2CAddr_0x76 // I2C address. I2C specific.
  );

  bme = new BME280I2C(settings);

  for (uint8_t i = 1; (i > 0) && (!(present = bme->begin())); i--) {
    Serial.println("Could not find BME280I2C sensor!");
    delay(1000);
  }

  if (present) {
    switch (bme->chipModel()) {
      case BME280::ChipModel_BME280:
        Serial.println("Found BME280 sensor! Success.");
        break;
      case BME280::ChipModel_BMP280:
        Serial.println("Found BMP280 sensor! No Humidity available.");
        break;
      default:
        Serial.println("Found UNKNOWN sensor! Error!");
    }

    settings.tempOSR = BME280::OSR_X4;      // Change some settings before using.

    bme->setSettings(settings);
  }
  tempUnit = BME280::TempUnit_Fahrenheit;
  presUnit = BME280::PresUnit_inHg;

  temp = hum = pres = 0;

}



void BME280c::loop_1s() {		  // execute this once per second in the main loop
  read();                     // read temperature, pressure, humidity from BME280
  wuReq.received();           // clear processed flag, we do not need to check for responses to wu update requests
}



void BME280c::read() {
  if (present)
    bme->read(pres, temp, hum, tempUnit, presUnit);    // read from BME280 (temperature, pressure, humidity)
}



void BME280c::print() {
  if (present) {
    read();                         // read temperature, pressure, humidity from BME280
    SERIALQ.print("Temp: ");        // print info to serial port
    SERIALQ.printf("%.2f", temp);
    SERIALQ.print("°");
    SERIALQ.print((char) (tempUnit == BME280::TempUnit_Celsius ? 'C' : 'F'));
    SERIALQ.print("  \tHumidity: ");
    SERIALQ.printf("%.2f", hum);
    SERIALQ.print("% RH");
    SERIALQ.print("  \tPressure: ");
    SERIALQ.printf("%.2f", pres);
    SERIALQ.println(" inHg");
  } else
    SERIALQ.println("BME280 sensor not present, no temperature/humidity/pressure readings.");
}




// update weather undergound
/*
  Weather underground
  wunderground.com
  jeremyabel57@gmail.com
  Gogojuice1!
  ID:KWAMARYS275
  KEY:79ZmB1sE
  https://weatherstation.wunderground.com/weatherstation/updateweatherstation.php?ID=XXXXX&PASSWORD=YYYYYYY&dateutc=now&humidity=59&baromin=inches&action=updateraw
*/
#define WUID "KWAMARYS275"
#define WUPW "Gogojuice1%21"
void BME280c::updateWUnderground(const char *sid) {
  char *id = (char*) sid;
  if (!id) id = WUID;
  char s[250];
  sprintf(s, "https://weatherstation.wunderground.com/weatherstation/updateweatherstation.php?ID=%s&PASSWORD=%s&dateutc=now&tempf=%f&humidity=%f&baromin=%f&action=updateraw",
          id, WUPW, bme280.temp, bme280.hum, bme280.pres);
  wuReq.send(s);
}





/* Recommended Modes -
   Based on Bosch BME280I2C environmental sensor data sheet.

  Weather Monitoring :
   forced mode, 1 sample/minute
   pressure ×1, temperature ×1, humidity ×1, filter off
   Current Consumption =  0.16 μA
   RMS Noise = 3.3 Pa/30 cm, 0.07 %RH
   Data Output Rate 1/60 Hz

  Humidity Sensing :
   forced mode, 1 sample/second
   pressure ×0, temperature ×1, humidity ×1, filter off
   Current Consumption = 2.9 μA
   RMS Noise = 0.07 %RH
   Data Output Rate =  1 Hz

  Indoor Navigation :
   normal mode, standby time = 0.5ms
   pressure ×16, temperature ×2, humidity ×1, filter = x16
   Current Consumption = 633 μA
   RMS Noise = 0.2 Pa/1.7 cm
   Data Output Rate = 25Hz
   Filter B
   andwidth = 0.53 Hz
   Response Time (75%) = 0.9 s


  Gaming :
   normal mode, standby time = 0.5ms
   pressure ×4, temperature ×1, humidity ×0, filter = x16
   Current Consumption = 581 μA
   RMS Noise = 0.3 Pa/2.5 cm
   Data Output Rate = 83 Hz
   Filter Bandwidth = 1.75 Hz
   Response Time (75%) = 0.3 s

*/

//
