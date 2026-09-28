
#ifndef GENERAL_H
#define GENERAL_H
#include <Arduino.h>
#include "inttypes.h"





unsigned long elapsems(unsigned long a, unsigned long b);          // give the elapse between a and b, assuming b > a, and accounding for unsigned long roll-over
char *strfind(char *haystack, const char *needle);                 // search for string needle within haystack
char *strfind(char *haystack, char needle = '\n', uint8_t d = 0);  // search for a character within a string
void delayus(unsigned long d = 1);                                 // delay for a given number of microseconds.  Using micros() timer, the first microsecond will be a partial microsecond delay.
void mdelay(unsigned long d = 1);                                  // delay for a given number of milliseconds.  Using yield() to allow background interrupts.
void udelay(unsigned long d = 1);                                  // delay for a given number of microseconds.  Using yield() to allow background interrupts.
//void disable_wifi();                                               // turn off wifi

//void littlefsmy_list();


#endif

//
