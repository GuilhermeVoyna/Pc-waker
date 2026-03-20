#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <WiFi.h>
#include <time.h>
#include "config/NetworkConfig.h"

class WiFiManager {
  private:
    NetworkConfig* config;
    const char* getWiFiStatus(wl_status_t status);
    void connect();
    void syncTime();

  public:
    WiFiManager(NetworkConfig* config);
    void begin();

};

#endif
