
#define GENERAL_C
#include "general.h"
#include "string.h"
#include <WiFi.h>
#include <LittleFS.h>

#include "serialq.h"        // queue buffer for serial output
#ifndef SERIALQ             //
#define SERIALQ Serial      //
#endif                      //






// give the elapse between a and b, assuming b > a, and accounding for unsigned long roll-over
unsigned long elapsems(unsigned long a, unsigned long b) {
  if (a > b) return b - a;                              // if confined to unsigned long, this probably works for both cases
  else return (unsigned long)0xFFFFFFFF - (a - b) + 1;  //(unsigned long long) 0x100000000-(a-b);
}



/*        strfind
  task:   search a string
  in:     haystack = string being searched
          needle = string to search for
  out:    pointer to first occurance of needle within haystack
          if not found, the pointer points to the null terminator of haystack
*/
char *strfind(char *haystack, const char *needle) {
  char *p = haystack;
  int n = strlen(needle);
  while ((*p) && (strncmp(p, needle, n) != 0)) p++;
  return p;
}



/*        strfind
  task:   search a string
  in:     haystack = string being searched
          needle = character to search for
          d = number of chars past search results to move result pointer forward
  out:    pointer to first occurance of needle within haystack
          if not found, the pointer points to the null terminator of haystack
*/
char *strfind(char *haystack, char needle, uint8_t d) {
  char *p = haystack;
  while ((*p) && (*p != needle)) p++;
  while ((*p) && ((d--) > 0)) p++;
  return p;
}


void delayus(unsigned long d) {  // microsecond delay, delay between d-1 and d microseconds
  unsigned long us = micros();
  while ((micros() - us) < d)
    ;
}
void mdelay(unsigned long d) {  // delay between d-1 and d milliseconds, with yield to allow background interrupts
  unsigned long ms = millis();
  while ((millis() - ms) < d)
    yield();
}
void udelay(unsigned long d) {  // delay between d-1 and d microseconds, with yield to allow background interrupts
  unsigned long us = micros();
  while ((micros() - us) < d)
    yield();
}



/*
void disable_wifi() {
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  
//  WiFiMode_t(WIFI_STA);
//  WiFi.disconnect();
//  WiFi.mode_t(WIFI_OFF);
}
*/


/*
void littlefsmy_list() {
  File root = LittleFS.open("/");
  File file = root.openNextFile();
  while (file) {
    SERIALQ.print("FILE: ");
    SERIALQ.println(file.name());
    file = root.openNextFile();
  }
}
*/

//
