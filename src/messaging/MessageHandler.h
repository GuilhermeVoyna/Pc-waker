#pragma once
#include <Arduino.h>

class MessageHandler {
public:
    static void handle(char* topic, byte* payload, unsigned int length);
};
