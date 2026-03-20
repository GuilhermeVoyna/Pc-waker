#include "mqtt/MQTTManager.h"
#include "config/Topics.h"
#include "messaging/MessageHandler.h"
#include "led/LED.h"

int MAX_ATTEMPTS = 5;
int attempt = 0;

MQTTManager::MQTTManager(NetworkConfig* config, WiFiClientSecure* espClient, PubSubClient* client)
: config(config), espClient(espClient), client(*client) {}

void MQTTManager::begin() {
  espClient->setInsecure(); // depois podemos usar certificado
  client.setServer(config->mqtt_server, config->mqtt_port);
  client.setCallback(callbackStatic);


}

void MQTTManager::callbackStatic(char* topic, byte* payload, unsigned int length) {
  Serial.print("Mensagem recebida [");
  Serial.print(topic);
  Serial.print("]: ");

  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
  led.blink(111, 1);
  MessageHandler handler;
  handler.handle(topic, payload, length);
}

void MQTTManager::reconnect() {
  
  while (!client.connected()) {
    Serial.print("Conectando ao HiveMQ... ");

    if (client.connect("ESP32Client",
                       config->mqtt_user,
                       config->mqtt_password)) {

      Serial.println("Conectado!");
      client.subscribe(PC_COMMAND_TOPIC);
      client.subscribe(PC_BROADCAST_TOPIC);
      client.subscribe(PC_STATUS_TOPIC);                

      led.off();
      attempt = 0;
    } else {
      Serial.print("Falhou, rc=");
      Serial.print(client.state());
      Serial.println(" tentando novamente em 5s");
      led.on();
      delay(5000);
      attempt++;
      if (attempt >= MAX_ATTEMPTS) {
        Serial.println("Máximo de tentativas atingido. Reiniciando...");
        led.blink(50, 20);
        ESP.restart();
        
      }

    }
  }
}

void MQTTManager::loop() {

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi desconectado! Reconectando...");
    WiFi.reconnect();
    delay(2000);
    return;
  }

  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}

void MQTTManager::publish(const char* topic, const char* message) {
  client.publish(topic, message);

}
