#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <WiFi.h>
#include "config/NetworkConfig.h"

class WiFiManager {
private:
    NetworkConfig* config;

    unsigned long lastConnectionAttempt;
    unsigned long lastStatusPrint;
    unsigned long lastNtpCheck;

    bool wasConnected;
    bool ntpStarted;
    bool timeSynced;

public:
    explicit WiFiManager(NetworkConfig* config);

    static const char* getWiFiStatus(wl_status_t status);

    void connect();
    void syncTime();
    void begin();

    // Deve ser chamada continuamente no loop principal.
    void loop();

    bool isConnected() const;
    bool isTimeSynced() const;
};

#endif