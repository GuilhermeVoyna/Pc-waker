#include <WiFiUdp.h>
#include <WiFi.h>
#include "pc/PCController.h"
#include "led/LED.h"

// Instância global do UDP
static WiFiUDP udp;


void PCController::handleCommand(
    std::string host,
    String message
) {
    if (message != "turn_on" && message != "wake") {
        Serial.printf(
            "[WOL] Comando desconhecido: %s\n",
            message.c_str()
        );
        return;
    }

    if (host == "all") {
        Serial.println(
            "[WOL] ERRO: 'all' nao pode ser tratado como host."
        );
        return;
    }

    Serial.printf("[WOL] Host destino: %s\n", host.c_str());

    const byte* mac = hostConfig.getMacAddress(host);

    if (mac == nullptr) {
        Serial.printf(
            "[WOL] ERRO: MAC nao encontrado para %s.\n",
            host.c_str()
        );
        return;
    }

    Serial.printf(
        "[WOL] MAC alvo: %02X:%02X:%02X:%02X:%02X:%02X\n",
        mac[0], mac[1], mac[2],
        mac[3], mac[4], mac[5]
    );

    if (sendWakeOnLan(mac)) {
        Serial.println(
            "[WOL] Magic Packet enviado para o MAC acima."
        );
    } else {
        Serial.println("[WOL] Falha no envio do Magic Packet.");
    }
}

void PCController::handleBroadCast(String message) {
    if (message != "turn_on" && message != "wake") {
        Serial.printf(
            "[WOL] Comando broadcast desconhecido: %s\n",
            message.c_str()
        );
        return;
    }

    const auto& hosts = hostConfig.getHosts();

    if (hosts.empty()) {
        Serial.println(
            "[WOL] Nenhum computador cadastrado. "
            "Nao ha MACs para enviar."
        );
        return;
    }

    Serial.printf(
        "[WOL] Broadcast logico para %u computador(es).\n",
        static_cast<unsigned int>(hosts.size())
    );

    for (const auto& host : hosts) {
        if (host.name == "all") {
            continue;
        }

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

