#include "led/LED.h"

LED::LED(uint8_t pinNumber)
    : pin(pinNumber),
      ledState(false),
      blinking(false),
      blinkInterval(0),
      lastToggle(0),
      togglesRemaining(0) {
    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW);
}

void LED::on() {
    blinking = false;
    togglesRemaining = 0;

    ledState = true;
    digitalWrite(pin, HIGH);
}

void LED::off() {
    blinking = false;
    togglesRemaining = 0;

    ledState = false;
    digitalWrite(pin, LOW);
}

void LED::toggle() {
    blinking = false;
    togglesRemaining = 0;

    ledState = !ledState;
    digitalWrite(pin, ledState ? HIGH : LOW);
}

void LED::blink(unsigned long duration, int times) {
    if (times <= 0 || duration == 0) {
        return;
    }

    blinkInterval = duration;
    lastToggle = millis();
    togglesRemaining = static_cast<unsigned long>(times) * 2UL;

    blinking = true;

    // Cada sequência começa com o LED aceso.
    ledState = true;
    digitalWrite(pin, HIGH);
}

void LED::update() {
    if (!blinking) {
        return;
    }

    const unsigned long now = millis();

    if (now - lastToggle < blinkInterval) {
        return;
    }

    lastToggle = now;

    ledState = !ledState;
    digitalWrite(pin, ledState ? HIGH : LOW);

    togglesRemaining--;

    // Garante que a sequência termine com o LED apagado.
    if (togglesRemaining == 0) {
        blinking = false;
        ledState = false;
        digitalWrite(pin, LOW);
    }
}