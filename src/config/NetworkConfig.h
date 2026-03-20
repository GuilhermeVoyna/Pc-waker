#ifndef NetworkConfig_H
#define NetworkConfig_H

#include <Arduino.h>


class NetworkConfig {
public:
  const char* ssid; 
  const char* password; 
  const char* mqtt_server; 
  int mqtt_port; 
  const char* mqtt_user; 
  const char* mqtt_password;
  
  NetworkConfig(
    const char* ssid, 
    const char* password, 
    const char* mqtt_server, 
    int mqtt_port, 
    const char* mqtt_user, 
    const char* mqtt_password
  );

  void setup_wifi();

  String GetNetWorkConfig();
};

#endif
