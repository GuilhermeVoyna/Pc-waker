#pragma once

#include <vector>
#include <string>
#include <Arduino.h>

struct HostInfo {
    std::string name;
    std::string mac;
};

class HostConfig {
private:
    std::vector<HostInfo> hosts;

public:
    HostConfig();

    void processHosts(String message);

    // Insere ou atualiza um computador existente.
    void upsertHost(
        const std::string& name,
        const std::string& mac
    );

    // Mantido para compatibilidade com chamadas existentes.
    void addHost(
        const std::string& name,
        const std::string& mac
    );

    const std::vector<HostInfo>& getHosts() const;

    HostInfo* getHostByName(const std::string& name);

    const byte* getMacAddress(const std::string& host);
};