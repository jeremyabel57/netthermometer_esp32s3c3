


#ifndef SERIF_H
#define SERIF_H


/*

serif.begin();	// setup
serif.loop();	  // interface with host thru usb/primary uart


*/

#include <inttypes.h>    //
#include <Arduino.h>     //
                         //
#include "serialq.h"     //
#ifndef SERIALQ          //
#define SERIALQ Serial   //
#endif                   // \
                         //
#define SERINSZ 256      // enough to handle one line of input


class serifc {
protected:
  char serinbuf[SERINSZ];  // buffer for input from host
  uint8_t serinmode = 0;   // serial input mode (0=newline/cr ends input, 1=\x03 ETX ends input)
  uint16_t serinbuflen = 0;
  bool serinateol = false;

  void serinproc();  // process complete line/command entered from host
  void serin(char);  // process serial input from host

public:

  void begin();               // initialize (setup)
  void loop();                // run in loop to respond to serial input

};


#ifdef SERIF_C
class serifc serif;
#else
extern class serifc serif;  // serial interface object
#endif


#endif

//
