
#include <ESP8266WiFi.h>

#include "config.h"
#include "ota.h"

const char CONFIG_WIFI_SSID_NAME[] PROGMEM = "wifi_ssid";
const char CONFIG_WIFI_SSID_VALUE[] PROGMEM = "YOURSSID";

const char CONFIG_WIFI_PASS_NAME[] PROGMEM = "wifi_pass";
const char CONFIG_WIFI_PASS_VALUE[] PROGMEM = "YOURPASSWORD";

void setup() {
  Serial.begin(115200);
  Serial.println("");
  Serial.println("Start");

  ota_setup();

  SPIFFS.begin();

  // Добавляем параметры
  // параметр без значения
  config.add(FPSTR("Param 1"));

  // параметр со значением
  config.add(FPSTR("Param 2"), FPSTR("Value 2"));

  // параметр с callback
  config.add(FPSTR("ParamCallback"), [](const String & value) {
    Serial.printf("\r\nUpdate ParamCallback value=%s\r\n", value.c_str());
  });

  // или все сразу
  config.add(FPSTR(CONFIG_WIFI_SSID_NAME), FPSTR(CONFIG_WIFI_SSID_VALUE), [](const String & value) {
    Serial.printf("\r\nupdate wifi ssid:%s\r\n", value.c_str());
  });
  config.add(FPSTR(CONFIG_WIFI_PASS_NAME), FPSTR(CONFIG_WIFI_PASS_VALUE), [](const String & value) {
    Serial.printf("\r\nupdate wifi pass:%s\r\n", value.c_str());
  });

  // выведем содержимое
  Serial.print("config.json():");
  Serial.println(config.json());

  // сохраним
  config.save();

  // читаем параметр
  Serial.print("Param 2=");
  Serial.println(config[FPSTR("Param 2")]);

  // изменим параметр
  //Serial.println("ParamCallback=NewValue");
  config.set(FPSTR("ParamCallback"), "NewValue");

  // выведем содержимое
  Serial.print("config.json():");
  Serial.println(config.json());

  Serial.print("config.jsonSecure():");
  Serial.println(config.jsonSecure(F("_pass"), F("********")));

  // загрузим
  config.load();

  // выведем содержимое
  Serial.print("config.json():");
  Serial.println(config.json());

  Serial.print("config.jsonSecure():");
  Serial.println(config.jsonSecure(F("_pass"), F("********")));

  Serial.println("");
  Serial.print("connecting to ");
  Serial.print(config[FPSTR(CONFIG_WIFI_SSID_NAME)]);

  WiFi.mode(WIFI_STA);
  WiFi.begin(config[FPSTR(CONFIG_WIFI_SSID_NAME)].c_str(), config[FPSTR(CONFIG_WIFI_PASS_NAME)].c_str());
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  ota_loop();
}
