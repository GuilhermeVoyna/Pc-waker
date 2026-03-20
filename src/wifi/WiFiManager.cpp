#include "wifi/WiFiManager.h"
#include "config/NetworkConfig.h"

WiFiManager::WiFiManager(NetworkConfig* config) {
    this->config = config;
}
const char* WiFiManager::getWiFiStatus(wl_status_t status) {
    switch (status) {
        case WL_IDLE_STATUS:       return "IDLE";               // 0
        case WL_NO_SSID_AVAIL:     return "NO SSID AVAILABLE";  // 1
        case WL_SCAN_COMPLETED:    return "SCAN COMPLETED";    // 2
        case WL_CONNECTED:         return "CONNECTED";          // 3
        case WL_CONNECT_FAILED:    return "CONNECT FAILED";    // 4
        case WL_CONNECTION_LOST:   return "CONNECTION LOST";   // 5
        case WL_DISCONNECTED:      return "DISCONNECTED";      // 6
        default:                   return "UNKNOWN STATUS";
    }
}

void WiFiManager::connect() {
    WiFi.disconnect(true, true); // Limpa conexões antigas
    WiFi.mode(WIFI_STA);         // Garante modo estação
    WiFi.begin(config->ssid, config->password);

    Serial.print("Connecting to WiFi");

    int attempts = 0;
    const int maxAttempts = 20;

    while (WiFi.status() != WL_CONNECTED && attempts < maxAttempts) {
        delay(1000);
        attempts++;

        Serial.print(" | Attempt: ");
        Serial.print(attempts);
        Serial.print(" | Status: ");
        Serial.println(getWiFiStatus(WiFi.status()));

        if  (WiFi.status() == WL_CONNECT_FAILED) {
            WiFi.disconnect(true, true); 
            delay(500);
            WiFi.begin(config->ssid, config->password);
        }
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\nWiFi connected!");
        Serial.print("IP: ");
        Serial.println(WiFi.localIP());
    } else {
        Serial.println("\nFailed to connect to WiFi, restarting...");
        ESP.restart();
    }
}

void WiFiManager::syncTime() {
    // Ajuste de fuso horário -3 horas (Brasília)
    const long gmtOffset_sec = -3 * 3600;
    const int daylightOffset_sec = 0;

    // Lista de servidores NTP
    const char* ntpServers[] = {"pool.ntp.org", "time.google.com", "time.windows.com"};
    const int ntpCount = sizeof(ntpServers) / sizeof(ntpServers[0]);

    Serial.println("Synchronizing NTP time...");

    bool synced = false;
    for (int i = 0; i < ntpCount && !synced; i++) {
        configTime(gmtOffset_sec, daylightOffset_sec, ntpServers[i]);
        
        time_t now;
        int attempts = 0;
        const int maxAttempts = 20;

        do {
            delay(1000);
            now = time(nullptr);
            attempts++;

            Serial.print(".");
            Serial.print(" Attempt #");
            Serial.print(attempts);
            Serial.print(" | Server: ");
            Serial.println(ntpServers[i]);
        } while (now < 100000 && attempts < maxAttempts);

        if (now >= 100000) {
            synced = true;
            Serial.println("\nTime synchronized!");
            Serial.println(ctime(&now));
        } else {
            Serial.print("Failed to sync with ");
            Serial.println(ntpServers[i]);
        }
    }

    if (!synced) {
        Serial.println("Failed to synchronize time with all servers. Using system uptime as fallback.");
    }
}

void WiFiManager::begin() {
    connect();
    syncTime();
}
