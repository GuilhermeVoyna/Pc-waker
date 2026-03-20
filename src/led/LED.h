#ifndef LED_H
#define LED_H

#include <Arduino.h>

class LED
{
private:
    uint8_t pin;

public:
    LED(uint8_t pinNumber);
    void on();
    void off();
    void toggle();
    void blink(unsigned long duration, int times = 1);
};

#endif
extern LED led;
