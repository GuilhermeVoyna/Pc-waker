#ifndef LED_H
#define LED_H

#include <Arduino.h>

class LED {
private:
    uint8_t pin;

    bool ledState;
    bool blinking;

    unsigned long blinkInterval;
    unsigned long lastToggle;
    unsigned long togglesRemaining;

public:
    explicit LED(uint8_t pinNumber);

    void on();
    void off();
    void toggle();

    // Inicia uma sequência de piscadas sem bloquear.
    void blink(unsigned long duration, int times = 1);

    // Deve ser chamado continuamente no loop principal.
    void update();
};

#endif

extern LED led;