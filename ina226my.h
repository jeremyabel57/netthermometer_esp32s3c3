#ifndef INA226MY_H
#define INA226MY_H


//  ina226 temperature, humidity, pressure sensor

//#include <inttypes.h>         //
#include <Arduino.h>          //
//#include <Wire.h>             // Needed for legacy versions of Arduino.
#include <INA226.h>           // I2C communication to ina226 module



class ina226c {
  protected:

    INA226 *ina;

  public:

    bool present;               // true if INA226 module present
    float voltage;              // voltage reading (V)
    float current;              // current reading (mA)
    float power;                // power (mW)
    float shuntv;               // shunt voltage (mV)

    void begin();               // initialize (setup)
    void end();                 // shutdown INA226 device on powering down mcu
    void loop_1s();             // run every 1 second in loop

    void read();                // read ina226 sensor
    void print();               // print temperature, humidity, pressure to serial port

};


#ifdef INA226MY_C
class ina226c ina226;
#else
extern class ina226c ina226;
#endif


#endif

//
