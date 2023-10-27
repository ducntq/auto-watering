#include <Arduino.h>

void pumpControl(bool state, int pin)
{
    digitalWrite(pin, state ? ON : OFF);
}