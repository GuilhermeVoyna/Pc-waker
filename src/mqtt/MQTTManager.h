#ifndef MQTT_MANAGER_H
#define MQTT_MANAGER_H

#include <Arduino.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>

#include "config/NetworkConfig.h"

class MQTTManager {
private:
    NetworkConfig* config;
    WiFiClientSecure* espClient;
    PubSubClient client;

    // Controle da reconexão MQTT
    unsigned long nextReconnectAttempt;
    unsigned long reconnectInterval;
    bool reconnectScheduled;

    static MQTTManager* activeInstance;

    static void callbackStatic(
        char* topic,
        byte* payload,
        unsigned int length
    );

    void callback(
        char* topic,
        byte* payload,
        unsigned int length
    );

public:
    MQTTManager(
        NetworkConfig* config,
        WiFiClientSecure* espClient,
        PubSubClient* client
    );

    void begin();
    void reconnect();
    void loop();

    void publish(
        const char* topic,
        const char* message
    );

    bool connected() {
        return client.connected();
    }

    bool cleanupRetainedMessage(const char* topic);
};

#endif