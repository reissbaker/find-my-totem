#include "./led.h"
#include "Arduino.h"

void LED::BuiltIn::init() {
  pinMode(LED_BUILTIN, OUTPUT);
}

void LED::BuiltIn::on() {
  digitalWrite(LED_BUILTIN, HIGH);
}

void LED::BuiltIn::off() {
  digitalWrite(LED_BUILTIN, LOW);
}

void LED::BuiltIn::blink(int millis) {
  LED::BuiltIn::on();
  delay(millis);
  LED::BuiltIn::off();
  delay(millis);
}