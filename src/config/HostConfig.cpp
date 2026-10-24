#include "HostConfig.h"

#include <Arduino.h>
#include <ArduinoJson.h>
#include <cctype>
#include <cstdio>

// Normaliza o MAC para letras maiúsculas.
static std::string normalizeMac(const std::string& mac) {
    std::string normalized = mac;

    for (char& c : normalized) {
        c = static_cast<char>(
            std::toupper(static_cast<unsigned char>(c))
        );
    }

    return normalized;
}

// Valida o formato AA:BB:CC:DD:EE:FF.
static bool isValidMac(const std::string& mac) {
    if (mac.length() != 17) {
        return false;
    }

    for (size_t i = 0; i < mac.length(); i++) {
        if (i % 3 == 2) {
            if (mac[i] != ':') {
                return false;
            }
        } else if (
            !std::isxdigit(
                static_cast<unsigned char>(mac[i])
            )
        ) {
            return false;
        }
    }

    return true;
}

HostConfig::HostConfig() {
}

// Adiciona um host ou atualiza seu cadastro existente.
void HostConfig::addHost(
    const std::string& name,
    const std::string& mac
) {
    if (name.empty() || !isValidMac(mac)) {
        Serial.println(
            "Hostname vazio ou MAC invalido. Cadastro ignorado."
        );
        return;
    }

    const std::string normalizedMac = normalizeMac(mac);

    bool exists = false;

    // Remove registros com o mesmo hostname ou MAC.
    // Isso também consolida duplicatas já existentes.
    for (auto it = hosts.begin(); it != hosts.end();) {
        if (
            it->name == name ||
            normalizeMac(it->mac) == normalizedMac
        ) {
            exists = true;
            it = hosts.erase(it);
        } else {
            ++it;
        }
    }

    // Insere somente um registro atualizado.
    hosts.push_back({name, normalizedMac});

    if (exists) {
        Serial.print("Host atualizado: ");
    } else {
        Serial.print("Host cadastrado: ");
    }

    Serial.println(name.c_str());
}

// Retorna a lista de hosts.
const std::vector<HostInfo>& HostConfig::getHosts() const {
    return hosts;
}

// Procura host por nome.
HostInfo* HostConfig::getHostByName(
    const std::string& name
) {
    for (auto& h : hosts) {
        if (h.name == name) {
            return &h;
        }
    }

    return nullptr;
}

// Processa a mensagem recebida via MQTT.
void HostConfig::processHosts(String message) {
    JsonDocument doc;

    DeserializationError error = deserializeJson(doc, message);

    if (error) {
        Serial.print("Erro ao parsear JSON: ");
        Serial.println(error.c_str());
        return;
    }

    String status = doc["status"].as<String>();
    String mac = doc["mac"].as<String>();
    String hostname = doc["hostname"].as<String>();

    if (hostname.isEmpty() || mac.isEmpty()) {
        Serial.println(
            "Hostname ou MAC ausente. Cadastro ignorado."
        );
        return;
    }

    addHost(
        std::string(hostname.c_str()),
        std::string(mac.c_str())
    );
}

// Retorna o MAC em seis bytes.
const byte* HostConfig::getMacAddress(
    const std::string& host
) {
    HostInfo* h = getHostByName(host);

    if (h == nullptr) {
        return nullptr;
    }

    static byte mac[6];

    unsigned int values[6];

    int converted = std::sscanf(
        h->mac.c_str(),
        "%2x:%2x:%2x:%2x:%2x:%2x",
        &values[0],
        &values[1],
        &values[2],
        &values[3],
        &values[4],
        &values[5]
    );

    if (converted != 6) {
        return nullptr;
    }

    for (size_t i = 0; i < 6; i++) {
        mac[i] = static_cast<byte>(values[i]);
    }

    return mac;
}