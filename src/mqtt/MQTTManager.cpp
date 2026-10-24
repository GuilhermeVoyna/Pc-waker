#include <Arduino.h>
#include <WiFi.h>
#include <stdint.h>

#include "mqtt/MQTTManager.h"
#include "config/Topics.h"
#include "messaging/MessageHandler.h"
#include "led/LED.h"

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
}

void MQTTManager::begin() {
    // Mantém a configuração atual.
    // A validação do certificado TLS precisa ser corrigida separadamente.
    espClient->setInsecure();

    client.setServer(
        config->mqtt_server,
        config->mqtt_port
    );

    client.setCallback(callbackStatic);

    // Limita o tempo de espera das operações de socket.
    client.setSocketTimeout(5);
    client.setKeepAlive(30);

    Serial.println("MQTTManager inicializado.");
}

void MQTTManager::callbackStatic(
    char* topic,
    byte* payload,
    unsigned int length
) {
    Serial.print("Mensagem recebida [");
    Serial.print(topic);
    Serial.print("]: ");

    for (unsigned int i = 0; i < length; i++) {
        Serial.print(static_cast<char>(payload[i]));
    }

    Serial.println();

    led.blink(111, 1);

    MessageHandler handler;
    handler.handle(topic, payload, length);
}

void MQTTManager::callback(
    char* topic,
    byte* payload,
    unsigned int length
) {
    // Mantido para compatibilidade com a estrutura atual.
    MessageHandler handler;
    handler.handle(topic, payload, length);
}

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

    // Ainda não chegou o momento da próxima tentativa.
    // A subtração com long também permite lidar com o rollover
    // de millis(), desde que o intervalo seja inferior a 2^31 ms.
    if (
        reconnectScheduled &&
        static_cast<long>(now - nextReconnectAttempt) < 0
    ) {
        return;
    }

    reconnectScheduled = false;

    // Gera um identificador estável a partir do MAC da ESP32.
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

        if (
            !commandSubscribed ||
            !broadcastSubscribed ||
            !statusSubscribed
        ) {
            Serial.println(
                "Aviso: falha ao assinar um ou mais tópicos MQTT."
            );
        } else {
            Serial.println("Todos os tópicos foram assinados.");
        }

        // A conexão foi estabelecida.
        reconnectInterval = 1000;
        reconnectScheduled = false;

        led.off();
    } else {
        // Agenda a próxima tentativa sem usar delay().
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

void MQTTManager::publish(
    const char* topic,
    const char* message
) {
    if (!client.connected()) {
        Serial.println(
            "Mensagem não publicada: MQTT desconectado."
        );
        return;
    }

    if (!client.publish(topic, message)) {
        Serial.println("Falha ao publicar mensagem MQTT.");
    }
}