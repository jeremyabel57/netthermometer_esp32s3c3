/*
  stream queue


  
*/

#ifndef SERIALQ_H
#define SERIALQ_H

#define SERIALQ SerialQ   // for use as SERIALQ.function() instead of SerialQ.function().  then if not including SerialQ then define SERIALQ as Serial


#include <Arduino.h>      // Arduino
#include <inttypes.h>     // inttypes
#include "queuearray.h"   // queue implemented in an array
#include "stdarg.h"       // variable argument functions, like printf



class SerialQc {
public:
  SerialQc();   // constructor - set max length of queue (0-254), allocate buffer memory
  ~SerialQc();  // destructor - free buffer memory

  void begin(Stream *s1 = &Serial, uint16_t bin = 1024, uint16_t bout = 1024);  // set the size of the buffers
  void end();                                                                   // flush output queue to get ready for shutdown
  void loop();                                                                  // run inside main loop

  void flushout(uint16_t n = 0);  // flush the output queue, writing the queue data to the stream
  void flushin(uint16_t n = 0);   // flush the input queue, discarding the input data

  uint16_t available();  // number of bytes available in the queue or the stream
  char read();           // read one byte from the queue/stream

  // write to queue/stream
  void write(char);                            // single character
  void write(const char *);                    // string
  int write(const uint8_t *, int);             // buffer and length
  void print(const char);                      // char
  void print(const char *);                    // string
  void print(const uint8_t);                   // uint8_t
  void print(const uint16_t);                  // uint16_t
  void print(const uint32_t);                  // uint32_t
  void print(const int8_t);                    // int8_t
  void print(const int16_t);                   // int16_t
  void print(const int32_t);                   // int32_t
  void print(const long double);               // long long float
  void print(const float);                     // float
  void print(const double);                    // double
  void println(const char *);                  // string + newline
  void println();                              // just the newline
  int printfv(const char *fmt, va_list args);  // formatted string
  int printf(const char *fmt, ...);            // formatted string


protected:
  QueueArray<char, uint16_t> qin,  // input queue
    qout;                          // output queue
  Stream *s;                       // i/o stream
};



#ifdef SERIALQ_C
SerialQc SerialQ;
#else
extern SerialQc SerialQ;
#endif


#endif

//
