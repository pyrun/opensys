#include <SPI.h>
#include <RF24.h>
#include <printf.h>

#include "pyrun.h"

/*  Dont edit!
 *  Bitte nicht ändern
*/
pyrun* radio;

/* Typearten:
 *  I - Eingang
 *  O - Ausgänge
 *  S - Analogeingang
 *  A - Analogausgang
*/
char g_type = 'A';

/*  Anzahl der eingänge/ausgänge
 *  2
 *  4
 *  6
 *  8 (Max)
*/
char g_amount = 2;

void setup() {
  radio = new pyrun( 9, 10);
  radio->set( g_type, g_amount);
  radio->setPins( A0,A1,A2,A3,A4,A5, 3, 2);
  radio->setServer();
  Serial.begin(9600);
  printf_begin();
}

void loop() {
  radio->loop();
}



