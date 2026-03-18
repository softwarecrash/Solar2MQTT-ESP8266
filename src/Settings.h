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
static constexpr size_t LEGACY_EEPROM_SIZE = 4096;
static constexpr size_t SETTINGS_STORAGE_SIZE = 2048;
static constexpr size_t SETTINGS_MAGIC_OFFSET = 0;
static constexpr size_t SETTINGS_VERSION_OFFSET = 4;
static constexpr size_t SETTINGS_SIZE_OFFSET = 6;
static constexpr size_t SETTINGS_DATA_OFFSET = 8;
static constexpr size_t LEGACY_SETTINGS_LEN_OFFSET = 4;
static constexpr size_t LEGACY_SETTINGS_DATA_OFFSET = 6;
static constexpr uint32_t SETTINGS_MAGIC = 0x53324D42;
static constexpr uint32_t LEGACY_SETTINGS_MAGIC = 0x53324D51; // "S2MQ"
static constexpr uint16_t SETTINGS_VERSION = 1;
static constexpr unsigned int MQTT_PORT_DEFAULT = 1883;
static constexpr unsigned int MQTT_PORT_MIN = 0;
static constexpr unsigned int MQTT_PORT_MAX = 65535;
static constexpr unsigned int MQTT_REFRESH_DEFAULT = 30;
static constexpr unsigned int MQTT_REFRESH_MIN = 1;
static constexpr unsigned int MQTT_REFRESH_MAX = 65535;
// Settings: Stores persistent settings in a compact binary EEPROM format.

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
  static_assert(SETTINGS_DATA_OFFSET + sizeof(Data) <= SETTINGS_STORAGE_SIZE, "Settings storage too small");

  void load()
  {
    data = {}; // clear before load data
    applyDefaults();
    if (readBinaryFromEeprom())
    {
      return;
    }

    String json;
    if (!readLegacyJsonFromEeprom(json))
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

    applyString(doc["deviceName"], data.deviceName, sizeof(data.deviceName));
    applyString(doc["mqttServer"], data.mqttServer, sizeof(data.mqttServer));
    applyString(doc["mqttUser"], data.mqttUser, sizeof(data.mqttUser));
    applyString(doc["mqttPassword"], data.mqttPassword, sizeof(data.mqttPassword));
    applyString(doc["mqttTopic"], data.mqttTopic, sizeof(data.mqttTopic));
    applyString(doc["mqttTriggerPath"], data.mqttTriggerPath, sizeof(data.mqttTriggerPath));
    applyString(doc["httpUser"], data.httpUser, sizeof(data.httpUser));
    applyString(doc["httpPass"], data.httpPass, sizeof(data.httpPass));
    applyUInt(doc["mqttPort"], data.mqttPort, MQTT_PORT_MIN, MQTT_PORT_MAX);
    applyUInt(doc["mqttRefresh"], data.mqttRefresh, MQTT_REFRESH_MIN, MQTT_REFRESH_MAX);
    applyBool(doc["mqttJson"], data.mqttJson);
    applyBool(doc["webUIdarkmode"], data.webUIdarkmode);
    applyBool(doc["haDiscovery"], data.haDiscovery);
    save();
  }

  void save()
  {
    applyBounds();
    ensureStringTermination();
    writeBinaryToEeprom();
  }

  void reset()
  {
    data = {};
    applyDefaults();
    save();
  }

private:
  bool readBinaryFromEeprom()
  {
    EEPROM.begin(SETTINGS_STORAGE_SIZE);

    uint32_t magic = 0;
    uint16_t version = 0;
    uint16_t storedSize = 0;
    EEPROM.get(SETTINGS_MAGIC_OFFSET, magic);
    EEPROM.get(SETTINGS_VERSION_OFFSET, version);
    EEPROM.get(SETTINGS_SIZE_OFFSET, storedSize);
    if (magic != SETTINGS_MAGIC)
    {
      EEPROM.end();
      return false;
    }
    if (version != SETTINGS_VERSION || storedSize != sizeof(Data))
    {
      EEPROM.end();
      return false;
    }
    EEPROM.get(SETTINGS_DATA_OFFSET, data);
    EEPROM.end();
    ensureStringTermination();
    applyBounds();
    return true;
  }

  bool readLegacyJsonFromEeprom(String &jsonOut)
  {
    EEPROM.begin(LEGACY_SETTINGS_DATA_OFFSET);

    uint32_t magic = 0;
    EEPROM.get(SETTINGS_MAGIC_OFFSET, magic);
    if (magic != LEGACY_SETTINGS_MAGIC)
    {
      EEPROM.end();
      return false;
    }

    uint16_t len = 0;
    len |= (uint16_t)EEPROM.read(LEGACY_SETTINGS_LEN_OFFSET);
    len |= (uint16_t)EEPROM.read(LEGACY_SETTINGS_LEN_OFFSET + 1) << 8;
    EEPROM.end();

    size_t maxLen = LEGACY_EEPROM_SIZE - LEGACY_SETTINGS_DATA_OFFSET;
    if (len == 0 || len > maxLen)
    {
      return false;
    }
    EEPROM.begin(LEGACY_SETTINGS_DATA_OFFSET + len);

    jsonOut = "";
    if (!jsonOut.reserve(len))
    {
      EEPROM.end();
      return false;
    }
    for (uint16_t i = 0; i < len; i++)
    {
      jsonOut += (char)EEPROM.read(LEGACY_SETTINGS_DATA_OFFSET + i);
    }
    EEPROM.end();
    return true;
  }

  bool writeBinaryToEeprom()
  {
    EEPROM.begin(SETTINGS_STORAGE_SIZE);

    uint32_t magic = SETTINGS_MAGIC;
    uint16_t version = SETTINGS_VERSION;
    uint16_t storedSize = sizeof(Data);
    EEPROM.put(SETTINGS_MAGIC_OFFSET, magic);
    EEPROM.put(SETTINGS_VERSION_OFFSET, version);
    EEPROM.put(SETTINGS_SIZE_OFFSET, storedSize);
    EEPROM.put(SETTINGS_DATA_OFFSET, data);
    bool ok = EEPROM.commit();
    EEPROM.end();
    return ok;
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

  void ensureStringTermination()
  {
    data.deviceName[sizeof(data.deviceName) - 1] = '\0';
    data.mqttServer[sizeof(data.mqttServer) - 1] = '\0';
    data.mqttUser[sizeof(data.mqttUser) - 1] = '\0';
    data.mqttPassword[sizeof(data.mqttPassword) - 1] = '\0';
    data.mqttTopic[sizeof(data.mqttTopic) - 1] = '\0';
    data.mqttTriggerPath[sizeof(data.mqttTriggerPath) - 1] = '\0';
    data.httpUser[sizeof(data.httpUser) - 1] = '\0';
    data.httpPass[sizeof(data.httpPass) - 1] = '\0';
  }

  void applyBounds()
  {
    ensureStringTermination();
    data.mqttPort = clampUInt(data.mqttPort, MQTT_PORT_MIN, MQTT_PORT_MAX);
    data.mqttRefresh = clampUInt(data.mqttRefresh, MQTT_REFRESH_MIN, MQTT_REFRESH_MAX);
  }
};
