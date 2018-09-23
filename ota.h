#ifndef OTA_H
#define OTA_H

#include <ArduinoOTA.h>

void ota_setup() {
  // Port defaults to 8266
  // ArduinoOTA.setPort(8266);

  // Hostname defaults to esp8266-[ChipID]
  // ArduinoOTA.setHostname(HOSTNAME);

  // No authentication by default
  // ArduinoOTA.setPassword("admin");

  // Password can be set with it's md5 value as well
  // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
  // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

  ArduinoOTA.onStart([]() {
    Serial.print("OTA:Start[");
  });

  ArduinoOTA.onEnd([]() {
    Serial.println("]End\r\n");
  });

  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.print("|");
  });

  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("OTA:Error[%u]", error);
  });

  ArduinoOTA.begin();
}

inline void ota_loop() {
  ArduinoOTA.handle();
}

#endif // OTA_H
