/*
  stream queue


*/

#define SERIALQ_C
#include "serialq.h"
#include <stdlib.h>


SerialQc::SerialQc() {
  s = 0;
}
SerialQc::~SerialQc() {
}
void SerialQc::begin(Stream *s1, uint16_t bin, uint16_t bout) {
  s = s1;
  qin.begin(bin);
  qout.begin(bout);
}
void SerialQc::loop() {
  for (uint8_t i=2; i>0; i--) {                       // trying two character output per loop cycle to give faster serial
    if (s->available() && (!qin.full())) {
      qin.push(s->read());
    }
    //if (s->availableForWrite() && (!qout.empty())) {        // only for HW serial, will block all output on software serial
    if (!qout.empty()) {                                      // since this does not check output availability, it will block until byte written to output
      s->print((char)qout.pull());
    }
  }
}



void SerialQc::end() {
  flushout();
  //flushin(128);
}
void SerialQc::flushout(uint16_t n) {
  if (!s) {        // if no valid stream
    qout.clear();  // then clear/dump queue
    return;        //
  }                // end if
  if (n) n++;      // n decrements down to 1 then we stop, or n=0 for infinite
  while (((n == 0) || (n > 1)) && (qout.available() > 0)) {
    //while (s->availableForWrite()==0) yield;	// wait for stream to accept output
    s->print((char)qout.pull());
    if (n) n--;
  }
}
void SerialQc::flushin(uint16_t n) {
  if (n) n++;                                               // n decrements down to 1 then we stop, or n=0 for infinite
  while (((n == 0) || (n > 1)) && (qin.available() > 0)) {  // flush input queue
    qin.pull();
    if (n) n--;
  }
  if (!s) return;
  while (((n == 0) || (n > 1)) && (s->available())) {  // flush input stream
    s->read();
    if (n) n--;
  }
}



uint16_t SerialQc::available() {
  if (qin.empty()) {
    return s->available();
  } else {
    return qin.available();
  }
}
char SerialQc::read() {
  if (qin.empty()) {          // if empty queue then
    return s->read();         //   read direct from stream
  } else {                    // else
    return (char)qin.pull();  //   read from queue
  }
}



void SerialQc::write(const char c) {
  if (qout.empty() && qout.full()) {  // if no queue buffer, then write direct to stream
    s->print(c);
    return;
  }
  while (qout.full())  // make sure we have room in queue
    flushout(1);
  qout.push(c);
}
void SerialQc::write(const char *s) {
  for (char *t = (char *)s; *t; t++) {
    write(*t);
  }
}
int SerialQc::write(const uint8_t *b, int n) {
  int i = n;
  for (char *t = (char *)b; i > 0; i--) {
    write(*t);
    t++;
  }
  return n;
}
void SerialQc::print(const char c) {
  write(c);
}
void SerialQc::print(const char *s) {
  write(s);
}
void SerialQc::print(const int32_t i) {
  char s[20];
  lltoa(i, s, 10);
  //sprintf(s, "%Ld", i);
  write(s);
}
void SerialQc::print(const uint32_t i) {
  char s[20];
  lltoa(i, s, 10);
  //sprintf(s, "%Lu", i);
  write(s);
}
void SerialQc::print(const uint8_t i) {
  print((uint32_t) i);
}
void SerialQc::print(const uint16_t i) {
  print((uint32_t) i);
}
void SerialQc::print(const int8_t i) {
  print((int32_t) i);
}
void SerialQc::print(const int16_t i) {
  print((int32_t) i);
}
void SerialQc::print(const long double f) {
  char s[20];
  dtostrf(f, 0, 6, s);
  //sprintf(s, "%Lf", f);
  write(s);
}
void SerialQc::print(const float f) {
  print((long double) f);
}
void SerialQc::print(const double f) {
  print((long double) f);
}

void SerialQc::println(const char *s) {
  write(s);
  write("\r\n");
}
void SerialQc::println() {
  write("\r\n");
}

/*		printf...
  in:		fmt = string format
	  	... = string data
*/
int SerialQc::printfv(const char *fmt, va_list args) {
  const static uint16_t N = 1023;
  char s[N];
  int r = vsnprintf(s, N, fmt, args);
  write(s);
  return r;
}
int SerialQc::printf(const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  int r = printfv(fmt, args);
  va_end(args);
  return r;
}

//
