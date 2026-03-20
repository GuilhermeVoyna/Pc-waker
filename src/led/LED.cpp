#include "led/LED.h"

LED::LED(uint8_t pinNumber)
{
    pin = pinNumber;
    pinMode(pin, OUTPUT);
}

void LED::on()
{
    digitalWrite(pin, HIGH);
}

void LED::off()
{
    digitalWrite(pin, LOW);
}

void LED::toggle()
{
    digitalWrite(pin, !digitalRead(pin));
}
void LED::blink(unsigned long duration, int times)
{
    for (int i = 0; i < times; i++) {
        on();
        delay(duration);
        off();
        delay(duration);
    }
}
