#pragma once
#include <Arduino.h>
#include <WiFiUdp.h>
#include "config/HostConfig.h"

class PCController {
private:
    HostConfig& hostConfig;
public:
    PCController(HostConfig& hc) : hostConfig(hc) {}
    void handleCommand(std::string host, String message);
    void handleBroadCast(String message);
    bool sendWakeOnLan(const byte* macAddress);
};