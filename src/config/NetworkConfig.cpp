#include "config/NetworkConfig.h"
#include <WiFi.h>

NetworkConfig::NetworkConfig(
  const char* ssid, 
  const char* password, 
  const char* mqtt_server, 
  int mqtt_port, 
  const char* mqtt_user, 
  const char* mqtt_password
) {
  this->ssid = ssid;
  this->password = password;
  this->mqtt_server = mqtt_server;
  this->mqtt_port = mqtt_port;
  this->mqtt_user = mqtt_user;
  this->mqtt_password = mqtt_password;
}

