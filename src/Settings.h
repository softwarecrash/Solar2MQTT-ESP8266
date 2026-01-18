#include <Arduino.h>
#include <ArduinoJson.h>
#include <EEPROM.h>

static constexpr size_t DEVICE_NAME_LEN = 128;
static constexpr size_t MQTT_SERVER_LEN = 256;
static constexpr size_t MQTT_USER_LEN = 256;
static constexpr size_t MQTT_PASS_LEN = 256;
static constexpr size_t MQTT_TOPIC_LEN = 256;
static constexpr size_t MQTT_TRIGGER_LEN = 256;
static constexpr size_t HTTP_USER_LEN = 256;
static constexpr size_t HTTP_PASS_LEN = 256;
static constexpr size_t EEPROM_SIZE = 4096;
static constexpr size_t SETTINGS_MAGIC_OFFSET = 0;
static constexpr size_t SETTINGS_LEN_OFFSET = 4;
static constexpr size_t SETTINGS_DATA_OFFSET = 6;
static constexpr uint32_t SETTINGS_MAGIC = 0x53324D51; // "S2MQ"
static constexpr unsigned int MQTT_PORT_DEFAULT = 1883;
static constexpr unsigned int MQTT_PORT_MIN = 0;
static constexpr unsigned int MQTT_PORT_MAX = 65535;
static constexpr unsigned int MQTT_REFRESH_DEFAULT = 30;
static constexpr unsigned int MQTT_REFRESH_MIN = 1;
static constexpr unsigned int MQTT_REFRESH_MAX = 65535;
// Settings: Stores persistent settings, loads and saves JSON in EEPROM

class Settings
{
public:
  struct Data
  {                              // do not re-sort this struct
    char deviceName[DEVICE_NAME_LEN];      // device name
    char mqttServer[MQTT_SERVER_LEN];      // mqtt Server adress
    char mqttUser[MQTT_USER_LEN];          // mqtt Username
    char mqttPassword[MQTT_PASS_LEN];      // mqtt Password
    char mqttTopic[MQTT_TOPIC_LEN];        // mqtt publish topic
    char mqttTriggerPath[MQTT_TRIGGER_LEN];// MQTT Data Trigger Path
    unsigned int mqttPort;       // mqtt port
    unsigned int mqttRefresh;    // mqtt refresh time
    bool mqttJson;               // switch between classic mqtt and json
    bool webUIdarkmode;          // Flag for color mode in webUI
    char httpUser[HTTP_USER_LEN];          // http basic auth username
    char httpPass[HTTP_PASS_LEN];          // http basic auth password
    bool haDiscovery;            // HomeAssistant Discovery switch
  } data;

  void load()
  {
    data = {}; // clear before load data
    applyDefaults();
    String json;
    if (!readJsonFromEeprom(json))
    {
      save();
      return;
    }
    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, json);
    if (err)
    {
      save();
      return;
    }

    bool updated = false;
    updated |= !applyString(doc["deviceName"], data.deviceName, sizeof(data.deviceName));
    updated |= !applyString(doc["mqttServer"], data.mqttServer, sizeof(data.mqttServer));
    updated |= !applyString(doc["mqttUser"], data.mqttUser, sizeof(data.mqttUser));
    updated |= !applyString(doc["mqttPassword"], data.mqttPassword, sizeof(data.mqttPassword));
    updated |= !applyString(doc["mqttTopic"], data.mqttTopic, sizeof(data.mqttTopic));
    updated |= !applyString(doc["mqttTriggerPath"], data.mqttTriggerPath, sizeof(data.mqttTriggerPath));
    updated |= !applyString(doc["httpUser"], data.httpUser, sizeof(data.httpUser));
    updated |= !applyString(doc["httpPass"], data.httpPass, sizeof(data.httpPass));

    updated |= !applyUInt(doc["mqttPort"], data.mqttPort, MQTT_PORT_MIN, MQTT_PORT_MAX);
    updated |= !applyUInt(doc["mqttRefresh"], data.mqttRefresh, MQTT_REFRESH_MIN, MQTT_REFRESH_MAX);
    updated |= !applyBool(doc["mqttJson"], data.mqttJson);
    updated |= !applyBool(doc["webUIdarkmode"], data.webUIdarkmode);
    updated |= !applyBool(doc["haDiscovery"], data.haDiscovery);

    if (updated)
    {
      save();
    }
  }

  void save()
  {
    applyBounds();
    JsonDocument doc;
    doc["deviceName"] = data.deviceName;
    doc["mqttServer"] = data.mqttServer;
    doc["mqttUser"] = data.mqttUser;
    doc["mqttPassword"] = data.mqttPassword;
    doc["mqttTopic"] = data.mqttTopic;
    doc["mqttTriggerPath"] = data.mqttTriggerPath;
    doc["mqttPort"] = data.mqttPort;
    doc["mqttRefresh"] = data.mqttRefresh;
    doc["mqttJson"] = data.mqttJson;
    doc["webUIdarkmode"] = data.webUIdarkmode;
    doc["httpUser"] = data.httpUser;
    doc["httpPass"] = data.httpPass;
    doc["haDiscovery"] = data.haDiscovery;

    String json;
    serializeJson(doc, json);
    writeJsonToEeprom(json);
  }

  void reset()
  {
    data = {};
    applyDefaults();
    save();
  }

private:
  bool readJsonFromEeprom(String &jsonOut)
  {
    EEPROM.begin(EEPROM_SIZE);
    uint32_t magic = 0;
    for (size_t i = 0; i < sizeof(magic); i++)
    {
      magic |= ((uint32_t)EEPROM.read(SETTINGS_MAGIC_OFFSET + i)) << (8 * i);
    }
    if (magic != SETTINGS_MAGIC)
    {
      EEPROM.end();
      return false;
    }
    uint16_t len = 0;
    len |= (uint16_t)EEPROM.read(SETTINGS_LEN_OFFSET);
    len |= (uint16_t)EEPROM.read(SETTINGS_LEN_OFFSET + 1) << 8;
    size_t maxLen = EEPROM_SIZE - SETTINGS_DATA_OFFSET;
    if (len == 0 || len > maxLen)
    {
      EEPROM.end();
      return false;
    }
    jsonOut = "";
    jsonOut.reserve(len);
    for (uint16_t i = 0; i < len; i++)
    {
      jsonOut += (char)EEPROM.read(SETTINGS_DATA_OFFSET + i);
    }
    EEPROM.end();
    return true;
  }

  bool writeJsonToEeprom(const String &json)
  {
    size_t maxLen = EEPROM_SIZE - SETTINGS_DATA_OFFSET;
    if (json.length() == 0 || json.length() > maxLen)
    {
      return false;
    }
    EEPROM.begin(EEPROM_SIZE);
    for (size_t i = 0; i < 4; i++)
    {
      EEPROM.write(SETTINGS_MAGIC_OFFSET + i, (uint8_t)((SETTINGS_MAGIC >> (8 * i)) & 0xFF));
    }
    uint16_t len = (uint16_t)json.length();
    EEPROM.write(SETTINGS_LEN_OFFSET, (uint8_t)(len & 0xFF));
    EEPROM.write(SETTINGS_LEN_OFFSET + 1, (uint8_t)((len >> 8) & 0xFF));
    for (uint16_t i = 0; i < len; i++)
    {
      EEPROM.write(SETTINGS_DATA_OFFSET + i, (uint8_t)json[i]);
    }
    EEPROM.commit();
    EEPROM.end();
    return true;
  }

  static void copyString(char *dest, size_t destSize, const char *src)
  {
    if (!dest || destSize == 0)
    {
      return;
    }
    if (!src)
    {
      src = "";
    }
    strncpy(dest, src, destSize - 1);
    dest[destSize - 1] = '\0';
  }

  static bool applyString(JsonVariant v, char *dest, size_t destSize)
  {
    if (v.isNull())
    {
      return false;
    }
    String value = v.as<String>();
    if (value.length() >= destSize)
    {
      copyString(dest, destSize, value.c_str());
      return false;
    }
    copyString(dest, destSize, value.c_str());
    return true;
  }

  static bool applyBool(JsonVariant v, bool &dest)
  {
    if (v.isNull())
    {
      return false;
    }
    dest = v.as<bool>();
    return true;
  }

  static unsigned int clampUInt(unsigned int value, unsigned int minv, unsigned int maxv)
  {
    if (value < minv)
    {
      return minv;
    }
    if (value > maxv)
    {
      return maxv;
    }
    return value;
  }

  static bool applyUInt(JsonVariant v, unsigned int &dest, unsigned int minv, unsigned int maxv)
  {
    if (v.isNull())
    {
      return false;
    }
    unsigned int value = v.as<unsigned int>();
    unsigned int clamped = clampUInt(value, minv, maxv);
    dest = clamped;
    return value == clamped;
  }

  void applyDefaults()
  {
    copyString(data.deviceName, sizeof(data.deviceName), "Solar2MQTT");
    copyString(data.mqttServer, sizeof(data.mqttServer), "");
    copyString(data.mqttUser, sizeof(data.mqttUser), "");
    copyString(data.mqttPassword, sizeof(data.mqttPassword), "");
    copyString(data.mqttTopic, sizeof(data.mqttTopic), "Solar");
    copyString(data.mqttTriggerPath, sizeof(data.mqttTriggerPath), "");
    data.mqttPort = MQTT_PORT_DEFAULT;
    data.mqttRefresh = MQTT_REFRESH_DEFAULT;
    data.mqttJson = false;
    data.webUIdarkmode = false;
    copyString(data.httpUser, sizeof(data.httpUser), "");
    copyString(data.httpPass, sizeof(data.httpPass), "");
    data.haDiscovery = false;
  }

  void applyBounds()
  {
    data.mqttPort = clampUInt(data.mqttPort, MQTT_PORT_MIN, MQTT_PORT_MAX);
    data.mqttRefresh = clampUInt(data.mqttRefresh, MQTT_REFRESH_MIN, MQTT_REFRESH_MAX);
  }
};
