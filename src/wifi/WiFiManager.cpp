#include "wifi/WiFiManager.h"

#include <Arduino.h>
#include <time.h>

WiFiManager::WiFiManager(NetworkConfig* config)
    : config(config),
      lastConnectionAttempt(0),
      lastStatusPrint(0),
      lastNtpCheck(0),
      wasConnected(false),
      ntpStarted(false),
      timeSynced(false) {
}

const char* WiFiManager::getWiFiStatus(wl_status_t status) {
    switch (status) {
        case WL_IDLE_STATUS:
            return "IDLE";

        case WL_NO_SSID_AVAIL:
            return "NO SSID AVAILABLE";

        case WL_SCAN_COMPLETED:
            return "SCAN COMPLETED";

        case WL_CONNECTED:
            return "CONNECTED";

        case WL_CONNECT_FAILED:
            return "CONNECT FAILED";

        case WL_CONNECTION_LOST:
            return "CONNECTION LOST";

        case WL_DISCONNECTED:
            return "DISCONNECTED";

        default:
            return "UNKNOWN STATUS";
    }
}

void WiFiManager::connect() {
    WiFi.mode(WIFI_STA);

    // O gerenciamento das tentativas será feito neste módulo.
    WiFi.setAutoReconnect(false);

    Serial.println("Iniciando conexão Wi-Fi...");

    WiFi.begin(config->ssid, config->password);

    lastConnectionAttempt = millis();
    lastStatusPrint = 0;
}

void WiFiManager::syncTime() {
    // Fuso horário de Brasília: UTC-3.
    constexpr long gmtOffsetSec = -3 * 3600;
    constexpr int daylightOffsetSec = 0;

    Serial.println("Configurando sincronização NTP...");

    // Configura vários servidores. A sincronização acontece
    // em segundo plano, sem um loop de espera.
    configTime(
        gmtOffsetSec,
        daylightOffsetSec,
        "pool.ntp.org",
        "time.google.com",
        "time.windows.com"
    );

    ntpStarted = true;
    timeSynced = false;
    lastNtpCheck = 0;
}

void WiFiManager::begin() {
    connect();
    syncTime();
}

void WiFiManager::loop() {
    const unsigned long now = millis();
    const wl_status_t status = WiFi.status();

    if (status == WL_CONNECTED) {
        if (!wasConnected) {
            wasConnected = true;

            Serial.println("Wi-Fi conectado!");
            Serial.print("IP: ");
            Serial.println(WiFi.localIP());
        }

        // O SNTP sincroniza o relógio em segundo plano.
        if (ntpStarted && !timeSynced) {
            if (
                lastNtpCheck == 0 ||
                now - lastNtpCheck >= 5000
            ) {
                lastNtpCheck = now;

                const time_t currentTime = time(nullptr);

                // Evita tratar um relógio ainda não sincronizado
                // como horário válido.
                if (currentTime >= 1700000000) {
                    timeSynced = true;

                    Serial.println("Horário sincronizado!");

                    Serial.print(
                        "Data/hora: "
                    );

                    Serial.print(ctime(&currentTime));
                } else {
                    Serial.println(
                        "Aguardando sincronização NTP..."
                    );
                }
            }
        }

        return;
    }

    if (wasConnected) {
        wasConnected = false;
        Serial.println("Conexão Wi-Fi perdida.");
    }

    // Informa o estado sem imprimir em todas as iterações.
    if (
        lastStatusPrint == 0 ||
        now - lastStatusPrint >= 5000
    ) {
        lastStatusPrint = now;

        Serial.print("Wi-Fi: ");
        Serial.println(getWiFiStatus(status));
    }

    // Nova tentativa a cada 15 segundos, sem bloquear o loop.
    if (now - lastConnectionAttempt >= 15000) {
        lastConnectionAttempt = now;

        Serial.println("Tentando reconectar ao Wi-Fi...");
        WiFi.reconnect();
    }
}

bool WiFiManager::isConnected() const {
    return WiFi.status() == WL_CONNECTED;
}

bool WiFiManager::isTimeSynced() const {
    return timeSynced;
}