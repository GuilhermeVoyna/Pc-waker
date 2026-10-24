#include <Arduino.h>
#include <WiFi.h>
#include <stdint.h>

#include "mqtt/MQTTManager.h"
#include "config/Topics.h"
#include "messaging/MessageHandler.h"
#include "led/LED.h"
#include "config/root_ca.h"

// Instância utilizada pelo callback estático do PubSubClient.
MQTTManager* MQTTManager::activeInstance = nullptr;


MQTTManager::MQTTManager(
    NetworkConfig* config,
    WiFiClientSecure* espClient,
    PubSubClient* client
)
    : config(config),
      espClient(espClient),
      client(*client),
      nextReconnectAttempt(0),
      reconnectInterval(1000),
      reconnectScheduled(false) {
    activeInstance = this;
}


void MQTTManager::begin() {
    // Valida o certificado do servidor durante a conexão TLS.
    espClient->setCACert(HIVEMQ_ROOT_CA);

    client.setServer(
        config->mqtt_server,
        config->mqtt_port
    );

    client.setCallback(callbackStatic);

    client.setSocketTimeout(5);
    client.setKeepAlive(30);

    Serial.println(
        "MQTTManager inicializado com validacao TLS."
    );
}


// Callback estático chamado pelo PubSubClient.
void MQTTManager::callbackStatic(
    char* topic,
    byte* payload,
    unsigned int length
) {
    if (topic == nullptr || payload == nullptr) {
        Serial.println("[MQTT] Mensagem invalida.");
        return;
    }

    if (activeInstance == nullptr) {
        Serial.println("[MQTT] MQTTManager indisponivel.");
        return;
    }

    // Mensagens MQTT podem conter bytes nulos.
    String message;
    message.reserve(length);

    for (unsigned int i = 0; i < length; i++) {
        message += static_cast<char>(payload[i]);
    }

    message.trim();

    Serial.printf("[MQTT] Topico: %s\n", topic);
    Serial.printf("[MQTT] Payload: %s\n", message.c_str());

    // --------------------------------------------------
    // COMANDO CLEAN
    // --------------------------------------------------

    // "clean": limpa a mensagem retida do tópico recebido.
    if (message.equalsIgnoreCase("clean")) {
        activeInstance->cleanupRetainedMessage(topic);
        return;
    }

    // "clean pc/atibaia/status":
    // limpa a mensagem retida do tópico informado.
    if (
        message.length() > 6 &&
        message.substring(0, 6).equalsIgnoreCase("clean ")
    ) {
        String target = message.substring(6);
        target.trim();

        bool validTarget = false;

        // Aceita exclusivamente pc/<hostname>/status.
        if (target.startsWith("pc/")) {
            const int firstSlash = target.indexOf('/', 3);

            if (
                firstSlash > 3 &&
                target.substring(firstSlash + 1) == "status" &&
                target.indexOf('/', firstSlash + 1) == -1
            ) {
                String hostname = target.substring(3, firstSlash);

                validTarget = !hostname.isEmpty();

                // Impede curingas MQTT e caracteres inesperados.
                for (unsigned int i = 0; i < hostname.length(); i++) {
                    const char c = hostname[i];

                    const bool allowed =
                        (c >= 'a' && c <= 'z') ||
                        (c >= 'A' && c <= 'Z') ||
                        (c >= '0' && c <= '9') ||
                        c == '-' ||
                        c == '_' ||
                        c == '.';

                    if (!allowed) {
                        validTarget = false;
                        break;
                    }
                }

                // "all" não representa um computador individual.
                if (hostname.equalsIgnoreCase("all")) {
                    validTarget = false;
                }
            }
        }

        if (!validTarget) {
            Serial.println(
                "[CLEANUP] Topico invalido. "
                "Use: clean pc/<hostname>/status"
            );
            return;
        }

        activeInstance->cleanupRetainedMessage(
            target.c_str()
        );

        return;
    }

    // Não encaminha mensagens vazias para o handler.
    if (length == 0) {
        Serial.println("[MQTT] Payload vazio ignorado.");
        return;
    }

    // Mensagem normal.
    led.blink(111, 1);

    MessageHandler handler;
    handler.handle(topic, payload, length);
}


// Mantido para compatibilidade com a estrutura existente.
void MQTTManager::callback(
    char* topic,
    byte* payload,
    unsigned int length
) {
    MessageHandler handler;
    handler.handle(topic, payload, length);
}


// --------------------------------------------------
// RECONEXAO MQTT
// --------------------------------------------------

void MQTTManager::reconnect() {
    if (client.connected()) {
        reconnectScheduled = false;
        reconnectInterval = 1000;
        return;
    }

    if (WiFi.status() != WL_CONNECTED) {
        return;
    }

    const unsigned long now = millis();

    // Aguarda o próximo intervalo de reconexão.
    if (
        reconnectScheduled &&
        static_cast<long>(now - nextReconnectAttempt) < 0
    ) {
        return;
    }

    reconnectScheduled = false;

    // Identificador estável derivado do MAC da ESP32.
    String mac = WiFi.macAddress();
    mac.replace(":", "");

    String clientId = "pc-waker-";
    clientId += mac;

    Serial.print("Conectando ao HiveMQ como ");
    Serial.println(clientId);

    const bool connected = client.connect(
        clientId.c_str(),
        config->mqtt_user,
        config->mqtt_password
    );

    if (connected) {
        Serial.println("MQTT conectado!");

        const bool commandSubscribed =
            client.subscribe(PC_COMMAND_TOPIC);

        const bool broadcastSubscribed =
            client.subscribe(PC_BROADCAST_TOPIC);

        const bool statusSubscribed =
            client.subscribe(PC_STATUS_TOPIC);

        Serial.println("ASSINATURAS MQTT:");

        Serial.printf(
            "Comandos: [%s] %s\n",
            commandSubscribed ? "OK" : "ERRO",
            PC_COMMAND_TOPIC
        );

        Serial.printf(
            "Broadcast: [%s] %s\n",
            broadcastSubscribed ? "OK" : "ERRO",
            PC_BROADCAST_TOPIC
        );

        Serial.printf(
            "Status: [%s] %s\n",
            statusSubscribed ? "OK" : "ERRO",
            PC_STATUS_TOPIC
        );

        reconnectInterval = 1000;
        reconnectScheduled = false;

        led.off();
    } else {
        const unsigned long retryDelay = reconnectInterval;

        Serial.printf(
            "Falha MQTT, rc=%d. Nova tentativa em %lu ms.\n",
            client.state(),
            retryDelay
        );

        led.on();

        nextReconnectAttempt = now + retryDelay;
        reconnectScheduled = true;

        // Backoff progressivo: 1, 2, 4, 8, 16 e 30 segundos.
        if (reconnectInterval >= 15000) {
            reconnectInterval = 30000;
        } else {
            reconnectInterval *= 2;
        }
    }
}


// --------------------------------------------------
// LOOP MQTT
// --------------------------------------------------

void MQTTManager::loop() {
    if (WiFi.status() != WL_CONNECTED) {
        reconnectScheduled = false;
        reconnectInterval = 1000;
        return;
    }

    if (!client.connected()) {
        reconnect();
    }

    client.loop();
}


// --------------------------------------------------
// PUBLICACAO MQTT
// --------------------------------------------------

void MQTTManager::publish(
    const char* topic,
    const char* message
) {
    if (!client.connected()) {
        Serial.println(
            "Mensagem nao publicada: MQTT desconectado."
        );
        return;
    }

    if (!client.publish(topic, message)) {
        Serial.println("Falha ao publicar mensagem MQTT.");
    }
}


// --------------------------------------------------
// LIMPEZA DE MENSAGENS RETIDAS
// --------------------------------------------------

bool MQTTManager::cleanupRetainedMessage(
    const char* topic
) {
    if (topic == nullptr || topic[0] == '\0') {
        Serial.println("[CLEANUP] Topico vazio ou invalido.");
        return false;
    }

    if (!client.connected()) {
        Serial.println("[CLEANUP] MQTT desconectado.");
        return false;
    }

    // Payload vazio com retain=true solicita a exclusao
    // da mensagem retida nesse tópico.
    const uint8_t emptyPayload = 0;

    const bool success = client.publish(
        topic,
        &emptyPayload,
        0,
        true
    );

    Serial.printf(
        "[CLEANUP] %s: %s\n",
        topic,
        success
            ? "solicitacao enviada"
            : "falha ao publicar"
    );

    return success;
}