#ifndef CONFIG_H
#define CONFIG_H

#include <ArduinoJson.h>
#include <FS.h>

const char CONFIG_FILENAME[] PROGMEM = "/config.json";

typedef std::function<void(const String& value)> THandlerConfig;

class configParam {
 public:
  configParam(const __FlashStringHelper* name, const String& value,
              configParam* next, THandlerConfig onUpdate) {
    this->next = next;
    this->name = name;
    this->value = value;
    this->onUpdate = onUpdate;
  }
  configParam* next = NULL;
  const __FlashStringHelper* name;
  String value;
  THandlerConfig onUpdate;
};

class config {
 public:
  config(const __FlashStringHelper* filename);
  ~config();

  // Добавление нового параметра в конфиг, необходимо указать имя добавляемого
  // параметра в конфиг, при необходимости его значение и функцию, вызываемую
  // при изменении значения. Возвращает true в случае успеха и false если
  // параметр не был добавлен.
  bool add(const __FlashStringHelper* name, const __FlashStringHelper* value,
           THandlerConfig onUpdate);
  bool add(const __FlashStringHelper* name, THandlerConfig onUpdate) {
    return this->add(name, FPSTR(""), onUpdate);
  }

   // Возвращает значение параметра.
  const String operator[](const String& name) { return this->find(name)->value; }
  const String operator[](const __FlashStringHelper* name) {
    return this->find(name)->value;
  }

  // Изменяет значение указанного параметра.
  bool set(const String& name, const String& value);
  bool set(const __FlashStringHelper* name, const String& value) {
    return this->set(String(name), value);
  }

  // Записывает текущую конфигурации в SPIFFS.  Если в качестве параметра
  // передать json строку, то перед сохранением будет произведено
  // обновление значений параметров если они будут найдены в конфиге.
  bool save();
  bool save(const String& apiSave);

  // Читает конфигурацию из SPIFFS.
  bool load();

  // Возвращает json строку с текущей конфигурацией
  String json();

  // Возвращает json строку с текущей конфигурацией, Все ключи с окончанием
  // keyEnd будут заменены на символы value или остануться пустыми
  String jsonSecure(const String& keyEnd, const String& value);
  String jsonSecure(const __FlashStringHelper* keyEnd,
                    const __FlashStringHelper* value) {
    return this->jsonSecure(String(keyEnd), String(value));
  }

  // удаляет файл конфигурации
  bool remove();

  // String fileName() { return String(_filename); }

 private:
  configParam* _paramList;
  const __FlashStringHelper* _filename;

 // Поиск параметра по имени. Возвращает указатель на параметр.
  configParam* find(const String& name);
  configParam* find(const __FlashStringHelper* name) {
    return this->find(String(name));
  }

} config(FPSTR(CONFIG_FILENAME));

config::config(const __FlashStringHelper* filename) {
  _paramList = NULL;
  _filename = filename;
}

config::~config() {
  configParam* cp = _paramList;
  configParam* cpn = _paramList->next;
  while (cp) {
    delete cp;
    cp = cpn;
    if (cp) cpn = cp->next;
  }
}

bool config::add(const __FlashStringHelper* name,
                 const __FlashStringHelper* value = FPSTR(""),
                 THandlerConfig onUpdate = (THandlerConfig)NULL) {
  if (!this->find(name)) {
    configParam* parameter =
        new configParam(name, String(value), this->_paramList, onUpdate);
    this->_paramList = parameter;
    return true;
  }
  return false;
}

configParam* config::find(const String& name) {
  if (_paramList) {
    configParam* cp = this->_paramList;
    while (cp) {
      if (String(cp->name) == name)
        return cp;
      else
        cp = cp->next;
    }
  }
  return NULL;
}

bool config::set(const String& name, const String& value) {
  configParam* cp = this->find(name);
  if (cp) {
    cp->value = value;
    if (cp->onUpdate) cp->onUpdate(cp->value);
    return true;
  }
  return false;
}

bool config::save() {
  if (!_paramList) return false;
  File configFile = SPIFFS.open(String(_filename), "w");
  if (!configFile) return false;
  DynamicJsonBuffer jsonBuffer;
  JsonObject& json = jsonBuffer.createObject();
  configParam* cp = _paramList;
  while (cp) {
    json[String(cp->name)] = cp->value;
    cp = cp->next;
  }
  bool res = json.printTo(configFile) != 0;
  configFile.close();
  return res;
}

bool config::save(const String& apiSave) {
  if (!_paramList) return false;
  DynamicJsonBuffer jsonBuffer;
  JsonObject& json = jsonBuffer.parseObject(apiSave);
  if (json.success()) {
    configParam* cp = _paramList;
    while (cp) {
      if (json.containsKey(cp->name)) set(cp->name, json[cp->name]);
      cp = cp->next;
    }
    return save(_filename);
  }
  return false;
}

bool config::load() {
  if (!_paramList) return false;
  File configFile = SPIFFS.open(String(_filename), "r");
  if (!configFile) return false;
  DynamicJsonBuffer jsonBuffer;
  JsonObject& json = jsonBuffer.parseObject(configFile);
  configFile.close();
  if (!json.success()) return false;
  configParam* cp = _paramList;
  while (cp) {
    if (json.containsKey(String(cp->name)))
      cp->value = json[String(cp->name)].as<String>();
    if (cp->onUpdate) cp->onUpdate(cp->value);
    cp = cp->next;
  }
  return true;
}

String config::json() {
  configParam* cp = this->_paramList;
  String answer = String('{');
  while (cp) {
    answer += '\"' + String(cp->name) + "\":\"" + cp->value + '"';
    cp = cp->next;
    if (cp) answer += ',';
  }
  return answer + '}';
}

String config::jsonSecure(const String& keyEnd, const String& value) {
  configParam* cp = this->_paramList;
  String key, answer = String('{');
  while (cp) {
    key = String(cp->name);
    answer += '\"' + key + "\":\"";
    answer +=
        key.endsWith(keyEnd) ? (cp->value.length() ? value : "") : cp->value;
    answer += '"';
    cp = cp->next;
    if (cp) answer += ',';
  }
  return answer + '}';
}

bool config::remove() {
  String filename = String(_filename);
  if (!SPIFFS.exists(filename)) return false;
  return SPIFFS.remove(filename);
}

#endif
