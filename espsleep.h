#ifndef ESPSLEEP_H
#define ESPSLEEP_H


/*

  espsleep.begin();  // setup including esp essentials
  espsleep.loop();   //

*/

#include <Arduino.h>     //
//


// ESP sleep



class espsleepc {
  protected:
    uint8_t sleeppause;

  public:

    void begin();                 // initialize (setup)
    void loop();                  // loop function.  run in loop to process esp essentials functions

    void print_wakeup_reason();   // prints wakeup reason to serial port

};



#ifdef ESPSLEEP_C
class espsleepc espsleep;
#else
extern class espsleepc espsleep;
#endif


#endif

//
