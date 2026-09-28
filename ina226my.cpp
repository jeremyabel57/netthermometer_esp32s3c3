
#define INA226MY_C
#include "ina226my.h"

#include <Arduino.h>
#include <inttypes.h>         //
#include <Wire.h>             // Needed for legacy versions of Arduino.
#include "general.h"
#include <ctype.h>            // character types



#include "serialq.h"        // queue buffer for serial output
#ifndef SERIALQ             //
#define SERIALQ Serial      //
#endif                      //



void ina226c::begin() {
  ina = new INA226(0x40);     // i2c address 0x40

  present = ina->begin();
  if (present) {
    Serial.println("Found INA226 voltage and current sensor module.");
    ina->setMaxCurrentShunt(0.8192, 0.1);                 // module with R100 resistor (0.1 ohm), current limit 819.2mA
    ina->setAverage(INA226_16_SAMPLES);                   // average 16 samples per reading
    ina->setBusVoltageConversionTime(INA226_1100_us);     // 1.1ms to take sample
    ina->setShuntVoltageConversionTime(INA226_1100_us);   // 1.1ms to take sample
    ina->setModeShuntBusContinuous();                     // shunt and bus measurements, continuous
    mdelay(40);                                           // allow 40ms to take first set of readings and average
  }
  voltage = current = power = shuntv = 0;
}



void ina226c::end() {
  if (present)
    ina->shutDown();                    // shutdown for power savings
}



void ina226c::loop_1s() {		  // execute this once per second in the main loop
  read();                     // read voltage and current from ina226
}



void ina226c::read() {
  if (present) {
    voltage = ina->getBusVoltage();
    shuntv = ina->getShuntVoltage_mV();
    current = ina->getCurrent_mA();
    power = ina->getPower_mW();
  }
}



void ina226c::print() {
  read();                                                                                             // read data
  SERIALQ.printf("V=%.2f V\tI=%.2f mA\tP=%.2f mW\tShunt=%.2f mV\r\n", voltage, current, power, shuntv); // print data
}

//
