#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <time.h>

#include "utils/message.h"
#include "config/NetworkConfig.h"
#include "wifi/WiFiManager.h"
#include "mqtt/MQTTManager.h"
#include "led/LED.h"
#include "config/secrets.h"


 
// Certificado raiz (ISRG Root X1 - usado pelo HiveMQ Cloud)
static const char *root_ca PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
MIIFazCCA1OgAwIBAgISA...
(cole aqui o certificado ISRG Root X1 completo)
-----END CERTIFICATE-----
)EOF";

WiFiClientSecure espClient;
PubSubClient client(espClient);

NetworkConfig config(
  WIFI_SSID,
  WIFI_PASSWORD,
  MQTT_BROKER,
  MQTT_PORT,
  MQTT_USERNAME,
  MQTT_PASSWORD
);

WiFiManager wifi(&config);
MQTTManager mqtt(&config,&espClient,&client);
LED led(BUILTIN_LED);
unsigned long lastMsg = 0;
int value = 0;

// ===============================


void setup() {
    Serial.begin(9600);

    Serial.println("Iniciando...");

    wifi.begin();
    mqtt.begin();

    // Inicia as piscadas sem bloquear.
    led.blink(500, 2);
}

// ===============================

void loop() {
    wifi.loop();
    led.update();
    mqtt.loop();
}