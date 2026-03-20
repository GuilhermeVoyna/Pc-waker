#ifndef MESSAGE_H
#define MESSAGE_H

#include <Arduino.h>

class Message {
public:
  String device_id;
  String action;
  String status;
  String state;

  Message(String dev, String act, String stat, String st);

  String toJSON();
};

#endif
