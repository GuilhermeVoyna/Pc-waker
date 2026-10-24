#include "messaging/MessageHandler.h"
#include "config/Topics.h"
#include "pc/PCController.h"
#include "led/LED.h"
#include "config/HostConfig.h"

#include <cstring>
#include <string>

HostConfig hostConfig;
PCController pcController(hostConfig);

// Função genérica para verificar se o tópico está no formato pc/<host>/<suffix>
bool isPcTopicWithSuffix(const char* topic, const char* suffix) {
    const char* prefix = "pc/";
    size_t topicLen  = strlen(topic);
    size_t prefixLen = strlen(prefix);
    size_t suffixLen = strlen(suffix);

    if (topicLen < prefixLen + suffixLen + 1) return false; // +1 garante pelo menos 1 char para host
    if (strncmp(topic, prefix, prefixLen) != 0) return false;
    if (strcmp(topic + topicLen - suffixLen, suffix) != 0) return false;

    return true;
}

// Funções específicas usando a genérica
bool isStatusTopic(const char* topic) { return isPcTopicWithSuffix(topic, "status"); }
bool isCommandTopic(const char* topic){ return isPcTopicWithSuffix(topic, "command"); }


std::string getHostName(const char* topic) {
    if (topic == nullptr) {
        return "";
    }

    constexpr char prefix[] = "pc/";
    constexpr std::size_t prefixLen = sizeof(prefix) - 1;

    // Verifica se o tópico começa com "pc/"
    if (std::strncmp(topic, prefix, prefixLen) != 0) {
        return "";
    }

    const char* start = topic + prefixLen;
    const char* end = std::strrchr(start, '/');

    // Valida o formato do tópico
    if (end == nullptr || end == start || end[1] == '\0') {
        return "";
    }

    // Cria a string diretamente, sem alocação manual
    return std::string(
        start,
        static_cast<std::size_t>(end - start)
    );
}
void MessageHandler::handle(char* topic, byte* payload, unsigned int length) {

    // Converte payload para String
    String message;
    for (int i = 0; i < length; i++) {
        message += (char)payload[i];
    }

    Serial.print("Topic recebido: ");
    Serial.println(topic);

    Serial.print("Payload: ");
    Serial.println(message);

    // Roteamento por tópico
    if (strcmp(topic, PC_BROADCAST_TOPIC) == 0){
        pcController.handleBroadCast(message);
        led.blink(222, 2);
    }
    
    if (isCommandTopic(topic)) {
        std::string hostName = getHostName(topic);
        pcController.handleCommand(hostName, message);
        led.blink(222, 2);
    }

    if (isStatusTopic(topic)) {
        hostConfig.processHosts(message);
    }
    else {
        Serial.println("Tópico não tratado.");
    }
}