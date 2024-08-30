#include <Arduino.h>

void pumpControl(bool state, int pin)
{
    digitalWrite(pin, state ? LOW : HIGH);
}