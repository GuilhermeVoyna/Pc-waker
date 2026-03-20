#include "utils/message.h"

Message::Message(String dev, String act, String stat, String st) {
  device_id = dev;
  action = act;
  status = stat;
  state = st;
}

String Message::toJSON() {
  String json = "{";
  json += "\"device_id\":\"" + device_id + "\",";
  json += "\"action\":\"" + action + "\",";
  json += "\"status\":\"" + status + "\",";
  json += "\"state\":\"" + state + "\"";
  json += "}";
  return json;
}
