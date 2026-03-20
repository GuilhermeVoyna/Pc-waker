#include <WiFiUdp.h>
#include <WiFi.h>
#include "pc/PCController.h"
#include "led/LED.h"

// Instância global do UDP
static WiFiUDP udp;


void PCController::handleCommand(std::string host, String message) {
    if (message == "turn_on" || message == "wake") {
        
        Serial.print("Comando recebido para host: ");
        Serial.println(host.c_str());
        const byte* mac = hostConfig.getMacAddress(host);
        if (mac == nullptr) {
            Serial.print("MAC não encontrado para o host: ");
            Serial.println(host.c_str());
        } else {
            if (sendWakeOnLan(mac)) {
                Serial.println("Wake-on-LAN enviado com sucesso!");
            } else {
                Serial.println("Falha ao enviar Wake-on-LAN.");
            }
        }
    }
    else {
        Serial.println("Comando desconhecido");
    }
}

void PCController::handleBroadCast(String message) {
    for (const auto& host : hostConfig.getHosts()) {
        Serial.print("Enviando broadcast para host: ");
        Serial.println(host.name.c_str());
        handleCommand(host.name, message);
    }
}



bool PCController::sendWakeOnLan(const byte* macAddress) {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("WiFi não conectado.");
        return false;
    }

    // Cria Magic Packet
    byte packet[102];
    memset(packet, 0xFF, 6);
    for (int i = 0; i < 16; i++) {
        memcpy(&packet[6 + i * 6], macAddress, 6);
    }

    IPAddress ip = WiFi.localIP();
    IPAddress subnet = WiFi.subnetMask();
    IPAddress broadcast;
    for (int i = 0; i < 4; i++) {
        broadcast[i] = ip[i] | ~subnet[i];
    }

    udp.begin(9);

    bool success = true;
    for (int i = 0; i < 5; i++) {
        udp.beginPacket(broadcast, 9);
        udp.write(packet, sizeof(packet));
        if (!udp.endPacket()) success = false;
        delay(150);
    }

    Serial.print("Broadcast usado: ");
    Serial.println(broadcast);
    return success;
}

