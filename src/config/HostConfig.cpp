#include "HostConfig.h"
#include <ArduinoJson.h>
HostConfig::HostConfig() {

}
// Adiciona um novo host à lista
void HostConfig::addHost(const std::string& name, const std::string& mac) {
    HostInfo h;
    h.name = name;
    h.mac = mac;
    hosts.push_back(h);
}

// Retorna a lista de hosts
const std::vector<HostInfo>& HostConfig::getHosts() const {
    return hosts;
}

// Procura host por nome
HostInfo* HostConfig::getHostByName(const std::string& name) {
    for (auto &h : hosts) {
        if (h.name == name) return &h;
    }
    return nullptr;
}

void HostConfig::processHosts(String message) {
    JsonDocument doc;

    DeserializationError error = deserializeJson(doc, message);
    if (error) {
        Serial.print("Erro ao parsear JSON: ");
        Serial.println(error.c_str());
        return;
    }

    String status   = doc["status"].as<String>();
    String mac      = doc["mac"].as<String>();
    String hostname = doc["hostname"].as<String>();

    addHost(hostname.c_str(), mac.c_str());
    Serial.println(hostname);
}

const byte* HostConfig::getMacAddress(const std::string& host){
    HostInfo* h = HostConfig::getHostByName(host);
    if (h) {
        static byte mac[6];
        sscanf(h->mac.c_str(), "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx", 
               &mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5]);
        return mac;
    }
    return nullptr;
}