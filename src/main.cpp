
/*
Solar2MQTT Project
https://github.com/softwarecrash/Solar2MQTT
*/
#include "main.h"
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <ESP8266mDNS.h>
#include <ESPAsyncWiFiManager.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "Settings.h"
#include "html.h"
#include "favicon.h"
#include "PI_Serial/PI_Serial.h"

#ifdef TEMPSENS_PIN
#include <OneWire.h>
#include <DallasTemperature.h>
#include <NonBlockingDallas.h>
#endif




PI_Serial mppClient(INVERTER_RX, INVERTER_TX);
WiFiClient client;
PubSubClient mqttclient(client);
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");
AsyncWebSocketClient *wsClient;
DNSServer dns;
Settings settings;
WebSerial webSerial;

#ifdef TEMPSENS_PIN
OneWire oneWire(TEMPSENS_PIN);
DallasTemperature dallasTemp(&oneWire);
NonBlockingDallas tempSens(&dallasTemp);
DeviceAddress tempDeviceAddress;
uint8_t numOfTempSens;
#endif

#include "status-LED.h"

// new importetd
static constexpr size_t MQTT_CLIENT_ID_LEN = DEVICE_NAME_LEN + 8;
static constexpr size_t MQTT_TOPIC_BUFFER_LEN = MQTT_TOPIC_LEN + 128;
char mqttClientId[MQTT_CLIENT_ID_LEN];
ADC_MODE(ADC_VCC);

// flag for saving data
unsigned long mqtttimer = 0;
unsigned long RestartTimer = 0;
unsigned long slowDownTimer = 0;
bool shouldSaveConfig = false;
char mqtt_server[MQTT_SERVER_LEN];
bool restartNow = false;
bool askInverterOnce = false;
bool workerCanRun = true;
bool publishFirst = false;
bool haDiscTrigger = false;
unsigned int jsonSize = 0;
uint32_t bootcount = 0;
String commandFromUser;
String customResponse;
bool loopbackRequested = false;
bool loopbackInProgress = false;
bool loopbackDone = false;
bool loopbackOk = false;
bool loopbackPrevWorker = true;
unsigned long loopbackWaitStart = 0;
String loopbackMessage;
unsigned long wsCleanupTimer = 0;
struct DebugDownloadState
{
  uint8_t phase = 0;
  uint8_t headerStep = 0;
  uint8_t rawIndex = 0;
  uint8_t sectionIndex = 0;
  bool sectionInit = false;
  bool sectionHeaderPrinted = false;
  JsonObject sectionObj;
  JsonObject::iterator sectionIt;
  String line;
  size_t lineOffset = 0;
};

bool firstPublish;
JsonDocument Json;                                           // main Json
JsonObject deviceJson = Json["EspData"].to<JsonObject>();    // basic device data
JsonObject staticData = Json["DeviceData"].to<JsonObject>(); // battery package data
JsonObject liveData = Json["LiveData"].to<JsonObject>();     // battery package data


//----------------------------------------------------------------------
void saveConfigCallback()
{
  writeLog("Should save config");
  shouldSaveConfig = true;
}

void notifyClients()
{
  if (ws.count() == 0)
  {
    return;
  }

  char payload[512];
  size_t pos = 0;
  bool first = true;

  auto appendComma = [&]() {
    if (!first && pos < sizeof(payload)) {
      payload[pos++] = ',';
    }
    first = false;
  };

  auto appendKey = [&](const char *key) {
    appendComma();
    int written = snprintf(payload + pos, sizeof(payload) - pos, "\"%s\":", key);
    if (written <= 0) {
      return;
    }
    size_t w = static_cast<size_t>(written);
    if (w >= sizeof(payload) - pos) {
      pos = sizeof(payload);
    } else {
      pos += w;
    }
  };

  auto appendVariant = [&](const char *key, JsonVariantConst value) {
    if (pos >= sizeof(payload)) return;
    appendKey(key);
    if (pos >= sizeof(payload)) return;
    pos += serializeJson(value, payload + pos, sizeof(payload) - pos);
  };

  payload[pos++] = '{';
  appendVariant("PV_Input_Voltage", liveData[F("PV_Input_Voltage")]);
  appendVariant("PV_Input_Current", liveData[F("PV_Input_Current")]);
  appendVariant("PV_Charging_Power", liveData[F("PV_Charging_Power")]);
  appendVariant("PV2_Input_Voltage", liveData[F("PV2_Input_Voltage")]);
  appendVariant("PV2_Input_Current", liveData[F("PV2_Input_Current")]);
  appendVariant("PV2_Charging_Power", liveData[F("PV2_Charging_Power")]);
  appendVariant("AC_In_Voltage", liveData[F("AC_In_Voltage")]);
  appendVariant("AC_In_Frequenz", liveData[F("AC_In_Frequenz")]);
  appendVariant("AC_Out_Voltage", liveData[F("AC_Out_Voltage")]);
  appendVariant("AC_Out_Frequenz", liveData[F("AC_Out_Frequenz")]);
  appendVariant("AC_Out_Watt", liveData[F("AC_Out_Watt")]);
  appendVariant("AC_Out_Percent", liveData[F("AC_Out_Percent")]);
  appendVariant("Inverter_Bus_Temperature", liveData[F("Inverter_Bus_Temperature")]);
  appendVariant("Battery_Voltage", liveData[F("Battery_Voltage")]);
  appendVariant("Battery_Percent", liveData[F("Battery_Percent")]);
  appendVariant("Battery_Load", liveData[F("Battery_Load")]);
  appendVariant("Inverter_Operation_Mode", liveData[F("Inverter_Operation_Mode")]);

  appendKey("Wifi_RSSI");
  if (pos < sizeof(payload)) {
    int written = snprintf(payload + pos, sizeof(payload) - pos, "%d", WiFi.RSSI());
    if (written > 0) {
      size_t w = static_cast<size_t>(written);
      if (w >= sizeof(payload) - pos) {
        pos = sizeof(payload);
      } else {
        pos += w;
      }
    }
  }

  if (pos < sizeof(payload)) {
    payload[pos++] = '}';
  } else {
    payload[sizeof(payload) - 1] = '}';
    pos = sizeof(payload);
  }

  ws.textAll(payload, pos);
}

void finishLoopback(bool ok, const String &message)
{
  loopbackOk = ok;
  loopbackMessage = message;
  loopbackDone = true;
  loopbackInProgress = false;
  mppClient.setSuspend(false);
  workerCanRun = loopbackPrevWorker;
}

void handleLoopback()
{
  if (loopbackRequested && !loopbackInProgress)
  {
    loopbackRequested = false;
    loopbackInProgress = true;
    loopbackDone = false;
    loopbackOk = false;
    loopbackMessage = "Running...";
    loopbackPrevWorker = workerCanRun;
    workerCanRun = false;
    mppClient.setSuspend(true);
    loopbackWaitStart = millis();
    return;
  }

  if (!loopbackInProgress)
  {
    return;
  }

  if (mppClient.isBusy())
  {
    if (millis() - loopbackWaitStart > 5000)
    {
      finishLoopback(false, "Serial busy (autodetect running)");
    }
    return;
  }

  String details;
  bool ok = mppClient.loopbackTest(details);
  finishLoopback(ok, details);
}

static const char *getDebugSectionName(uint8_t index)
{
  switch (index)
  {
  case 0:
    return "EspData";
  case 1:
    return "DeviceData";
  case 2:
    return "LiveData";
  default:
    return "";
  }
}

static void buildRawLine(uint8_t index, String &line)
{
  line = "";
  switch (index)
  {
  case 0:
    line = "Q1: " + mppClient.get.raw.q1;
    break;
  case 1:
    line = "QPIGS: " + mppClient.get.raw.qpigs;
    break;
  case 2:
    line = "QPIGS2: " + mppClient.get.raw.qpigs2;
    break;
  case 3:
    line = "QPIRI: " + mppClient.get.raw.qpiri;
    break;
  case 4:
    line = "QT: " + mppClient.get.raw.qt;
    break;
  case 5:
    line = "QET: " + mppClient.get.raw.qet;
    break;
  case 6:
    line = "QEY: " + mppClient.get.raw.qey;
    break;
  case 7:
    line = "QEM: " + mppClient.get.raw.qem;
    break;
  case 8:
    line = "QED: " + mppClient.get.raw.qed;
    break;
  case 9:
    line = "QLT: " + mppClient.get.raw.qlt;
    break;
  case 10:
    line = "QLY: " + mppClient.get.raw.qly;
    break;
  case 11:
    line = "QLM: " + mppClient.get.raw.qlm;
    break;
  case 12:
    line = "QLD: " + mppClient.get.raw.qld;
    break;
  case 13:
    line = "QPI: " + mppClient.get.raw.qpi;
    break;
  case 14:
    line = "QMOD: " + mppClient.get.raw.qmod;
    break;
  case 15:
    line = "QALL: " + mppClient.get.raw.qall;
    break;
  case 16:
    line = "QMN: " + mppClient.get.raw.qmn;
    break;
  case 17:
    line = "QPIWS: " + mppClient.get.raw.qpiws;
    break;
  case 18:
    line = "QFLAG: " + mppClient.get.raw.qflag;
    break;
  case 19:
    line = "CommandAnswer: " + mppClient.get.raw.commandAnswer;
    break;
  default:
    break;
  }
  if (line.length() > 0)
  {
    line += "\n";
  }
}

static bool buildNextDebugLine(DebugDownloadState &state)
{
  state.line = "";
  state.lineOffset = 0;

  if (state.phase == 0)
  {
    switch (state.headerStep)
    {
    case 0:
      state.line = "Solar2MQTT Debug Data\n";
      state.headerStep++;
      return true;
    case 1:
      state.line = "Device: ";
      state.line += settings.data.deviceName;
      state.line += "\n";
      state.headerStep++;
      return true;
    case 2:
      state.line = "Uptime: ";
      state.line += String(millis() / 1000);
      state.line += " s\n";
      state.headerStep++;
      return true;
    case 3:
      state.line = "\n";
      state.headerStep++;
      return true;
    case 4:
      state.line = "[RAW]\n";
      state.headerStep++;
      state.phase = 1;
      return true;
    default:
      state.phase = 1;
      break;
    }
  }

  if (state.phase == 1)
  {
    if (state.rawIndex < 20)
    {
      buildRawLine(state.rawIndex, state.line);
      state.rawIndex++;
      return true;
    }
    state.phase = 2;
    state.sectionIndex = 0;
    state.sectionInit = false;
    state.sectionHeaderPrinted = false;
    state.line = "\n[PARSED]\n";
    return true;
  }

  while (state.phase == 2)
  {
    if (state.sectionIndex >= 3)
    {
      state.phase = 3;
      break;
    }

    const char *sectionName = getDebugSectionName(state.sectionIndex);
    if (!state.sectionInit)
    {
      state.sectionObj = Json[sectionName].as<JsonObject>();
      state.sectionIt = state.sectionObj.begin();
      state.sectionInit = true;
      state.sectionHeaderPrinted = false;
    }

    if (!state.sectionHeaderPrinted)
    {
      state.line = "\n[";
      state.line += sectionName;
      state.line += "]\n";
      state.sectionHeaderPrinted = true;
      return true;
    }

    if (state.sectionIt == state.sectionObj.end())
    {
      state.sectionIndex++;
      state.sectionInit = false;
      continue;
    }

    JsonPair kv = *state.sectionIt;
    ++state.sectionIt;

    state.line = sectionName;
    state.line += ".";
    state.line += kv.key().c_str();
    state.line += "=";
    state.line += kv.value().as<String>();
    state.line += "\n";
    return true;
  }

  return false;
}



void handleWebSocketMessage(void *arg, uint8_t *data, size_t len)
{
  AwsFrameInfo *info = (AwsFrameInfo *)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT)
  {
    // No null-termination: buffer size is not guaranteed to have extra space.
  }
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len)
{
  switch (type)
  {
  case WS_EVT_CONNECT:
    wsClient = client;
    notifyClients();
    break;
    case WS_EVT_PING:
    break;
  case WS_EVT_DISCONNECT:
    wsClient = nullptr;
    ws.cleanupClients(); // clean unused client connections
    break;
  case WS_EVT_DATA:
    handleWebSocketMessage(arg, data, len);
    break;
  case WS_EVT_PONG:
    break;
  case WS_EVT_ERROR:
    wsClient = nullptr;
    ws.cleanupClients(); // clean unused client connections
    break;
  }
}

/* Message callback of WebSerial */
void recvMsg(uint8_t *data, size_t len)
{
  String d = "";
  for (uint i = 0; i < len; i++)
  {
    d += char(data[i]);
  }
  commandFromUser = (d);
  webSerial.println("Sending [" + d + "] to Device");
}

bool resetCounter(bool count)
{

  if (count)
  {
    if (ESP.getResetInfoPtr()->reason == 6)
    {
      ESP.rtcUserMemoryRead(16, &bootcount, sizeof(bootcount));

      if (bootcount >= 10 && bootcount < 20)
      {
        settings.reset();
        ESP.eraseConfig();
        ESP.reset();
      }
      else
      {
        bootcount++;
        ESP.rtcUserMemoryWrite(16, &bootcount, sizeof(bootcount));
      }
    }
    else
    {
      bootcount = 0;
      ESP.rtcUserMemoryWrite(16, &bootcount, sizeof(bootcount));
    }
  }
  else
  {
    bootcount = 0;
    ESP.rtcUserMemoryWrite(16, &bootcount, sizeof(bootcount));
  }
  writeLog("Bootcount:%d Reboot reason:%d", bootcount, ESP.getResetInfoPtr()->reason);
  return true;
}

#include <ESPAsyncWebServer.h>


struct HtmlChunkState
{
  const char *chunks[3];
  size_t chunkLengths[3];
  uint8_t chunkIndex = 0;
  size_t chunkOffset = 0;
};

static void appendJsonString(String &out, const char *value)
{
  out += '"';
  if (value != nullptr)
  {
    for (const char *p = value; *p != '\0'; ++p)
    {
      char c = *p;
      switch (c)
      {
      case '\\':
      case '"':
        out += '\\';
        out += c;
        break;
      case '\n':
        out += "\\n";
        break;
      case '\r':
        out += "\\r";
        break;
      case '\t':
        out += "\\t";
        break;
      default:
        out += c;
        break;
      }
    }
  }
  out += '"';
}

void sendChunkedHtmlPage(AsyncWebServerRequest *request, const char *htmlBodyPROGMEM)
{
  HtmlChunkState *state = new HtmlChunkState;
  if (!state)
  {
    request->send(503, "text/plain", "Out of memory");
    return;
  }
  state->chunks[0] = HTML_HEAD;
  state->chunks[1] = htmlBodyPROGMEM;
  state->chunks[2] = HTML_FOOT;
  state->chunkLengths[0] = strlen_P(HTML_HEAD);
  state->chunkLengths[1] = strlen_P(htmlBodyPROGMEM);
  state->chunkLengths[2] = strlen_P(HTML_FOOT);

  request->onDisconnect([state]()
                        { delete state; });

  AsyncWebServerResponse *response = request->beginChunkedResponse(
      "text/html",
      [state](uint8_t *buffer, size_t maxLen, size_t) -> size_t
      {
        while (state->chunkIndex < 3)
        {
          const char *chunk = state->chunks[state->chunkIndex];
          size_t totalLen = state->chunkLengths[state->chunkIndex];

          if (state->chunkOffset >= totalLen)
          {
            state->chunkIndex++;
            state->chunkOffset = 0;
            continue;
          }

          size_t copyLen = totalLen - state->chunkOffset;
          if (copyLen > maxLen)
            copyLen = maxLen;

          for (size_t i = 0; i < copyLen; ++i)
          {
            buffer[i] = static_cast<uint8_t>(pgm_read_byte(chunk + state->chunkOffset + i));
          }
          state->chunkOffset += copyLen;
          return copyLen;
        }

        return 0;
      });

  response->setContentLength(0);
  request->send(response);
}

void sendMetaJson(AsyncWebServerRequest *request)
{
  size_t estimate = 128 + strlen(settings.data.deviceName) + strlen(SOFTWARE_VERSION) + strlen(SWVERSION);
  String json;
  json.reserve(estimate);
  json += '{';
  json += "\"device_name\":";
  appendJsonString(json, settings.data.deviceName);
  json += ",\"software_version\":";
  appendJsonString(json, SOFTWARE_VERSION);
  json += ",\"sw_version\":";
  appendJsonString(json, SWVERSION);
  json += ",\"dark_mode\":";
  json += settings.data.webUIdarkmode ? "true" : "false";
  json += ",\"flash_size\":";
  json += ESP.getFreeSketchSpace();
  json += '}';
  request->send(200, "application/json", json);
}

void sendConfigJson(AsyncWebServerRequest *request)
{
  size_t estimate = 256 +
                    strlen(settings.data.deviceName) +
                    strlen(settings.data.mqttServer) +
                    strlen(settings.data.mqttUser) +
                    strlen(settings.data.mqttPassword) +
                    strlen(settings.data.mqttTopic) +
                    strlen(settings.data.mqttTriggerPath) +
                    strlen(settings.data.httpUser) +
                    strlen(settings.data.httpPass);
  String json;
  json.reserve(estimate);
  json += '{';
  json += "\"device_name\":";
  appendJsonString(json, settings.data.deviceName);
  json += ",\"mqtt_server\":";
  appendJsonString(json, settings.data.mqttServer);
  json += ",\"mqtt_port\":";
  json += settings.data.mqttPort;
  json += ",\"mqtt_user\":";
  appendJsonString(json, settings.data.mqttUser);
  json += ",\"mqtt_password\":";
  appendJsonString(json, settings.data.mqttPassword);
  json += ",\"mqtt_topic\":";
  appendJsonString(json, settings.data.mqttTopic);
  json += ",\"mqtt_refresh\":";
  json += settings.data.mqttRefresh;
  json += ",\"mqtt_trigger\":";
  appendJsonString(json, settings.data.mqttTriggerPath);
  json += ",\"mqtt_json\":";
  json += settings.data.mqttJson ? "true" : "false";
  json += ",\"ha_discovery\":";
  json += settings.data.haDiscovery ? "true" : "false";
  json += ",\"webui_dark_mode\":";
  json += settings.data.webUIdarkmode ? "true" : "false";
  json += ",\"http_user\":";
  appendJsonString(json, settings.data.httpUser);
  json += ",\"http_pass\":";
  appendJsonString(json, settings.data.httpPass);
  json += '}';
  request->send(200, "application/json", json);
}





void setup()
{
  analogWrite(LED_PIN, 0);
#ifdef isUART_HARDWARE
  analogWrite(LED_COM, 0);
  analogWrite(LED_SRV, 0);
  analogWrite(LED_NET, 0);
#endif
  resetCounter(true);
  DBG_BEGIN(DBG_BAUD); // Debugging towards UART1
  settings.load();
  WiFi.persistent(true); // fix wifi save bug
  WiFi.hostname(settings.data.deviceName);
  WiFi.setAutoReconnect(true);
  WiFi.setAutoConnect(true);
  WiFi.setSleepMode(WIFI_NONE_SLEEP);
  AsyncWiFiManager wm(&server, &dns);
  snprintf(mqttClientId, sizeof(mqttClientId), "%s-%06X", settings.data.deviceName, ESP.getChipId());



  wm.setMinimumSignalQuality(20); // filter weak wifi signals
  wm.setSaveConfigCallback(saveConfigCallback);

  // create custom wifimanager fields

  AsyncWiFiManagerParameter custom_mqtt_server("mqtt_server", "MQTT server", NULL, MQTT_SERVER_LEN - 1);
  AsyncWiFiManagerParameter custom_mqtt_user("mqtt_user", "MQTT User", NULL, MQTT_USER_LEN - 1);
  AsyncWiFiManagerParameter custom_mqtt_pass("mqtt_pass", "MQTT Password", NULL, MQTT_PASS_LEN - 1);
  AsyncWiFiManagerParameter custom_mqtt_topic("mqtt_topic", "MQTT Topic", "Solar01", MQTT_TOPIC_LEN - 1);
  AsyncWiFiManagerParameter custom_mqtt_port("mqtt_port", "MQTT Port", "1883", 6);
  AsyncWiFiManagerParameter custom_mqtt_refresh("mqtt_refresh", "MQTT Send Interval", "300", 4);
  AsyncWiFiManagerParameter custom_device_name("device_name", "Device Name", "Solar2MQTT", DEVICE_NAME_LEN - 1);

  wm.addParameter(&custom_mqtt_server);
  wm.addParameter(&custom_mqtt_user);
  wm.addParameter(&custom_mqtt_pass);
  wm.addParameter(&custom_mqtt_topic);
  wm.addParameter(&custom_mqtt_port);
  wm.addParameter(&custom_mqtt_refresh);
  wm.addParameter(&custom_device_name);

  wm.setDebugOutput(false);       // disable wifimanager debug output
  wm.setMinimumSignalQuality(20); // filter weak wifi signals
  wm.setSaveConfigCallback(saveConfigCallback);
  // save settings if wifi setup is fire up
  bool apRunning = wm.autoConnect("Solar2MQTT-AP");
  if (shouldSaveConfig)
  {
    strncpy(settings.data.mqttServer, custom_mqtt_server.getValue(), sizeof(settings.data.mqttServer) - 1);
    settings.data.mqttServer[sizeof(settings.data.mqttServer) - 1] = '\0';
    strncpy(settings.data.mqttUser, custom_mqtt_user.getValue(), sizeof(settings.data.mqttUser) - 1);
    settings.data.mqttUser[sizeof(settings.data.mqttUser) - 1] = '\0';
    strncpy(settings.data.mqttPassword, custom_mqtt_pass.getValue(), sizeof(settings.data.mqttPassword) - 1);
    settings.data.mqttPassword[sizeof(settings.data.mqttPassword) - 1] = '\0';
    settings.data.mqttPort = atoi(custom_mqtt_port.getValue());
    strncpy(settings.data.deviceName, custom_device_name.getValue(), sizeof(settings.data.deviceName) - 1);
    settings.data.deviceName[sizeof(settings.data.deviceName) - 1] = '\0';
    strncpy(settings.data.mqttTopic, custom_mqtt_topic.getValue(), sizeof(settings.data.mqttTopic) - 1);
    settings.data.mqttTopic[sizeof(settings.data.mqttTopic) - 1] = '\0';
    settings.data.mqttRefresh = atoi(custom_mqtt_refresh.getValue());
    settings.save();
    ESP.restart();
  }

  mqttclient.setServer(settings.data.mqttServer, settings.data.mqttPort);

  mqttclient.setCallback(mqttcallback);

  // check is WiFi connected
  if (apRunning)
  {
    server.on("/old", HTTP_GET, [](AsyncWebServerRequest *request)
              {
              if(strlen(settings.data.httpUser) > 0 && !request->authenticate(settings.data.httpUser, settings.data.httpPass)) return request->requestAuthentication();
              sendChunkedHtmlPage(request, HTML_MAIN); });



              server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
                if(strlen(settings.data.httpUser) > 0 && !request->authenticate(settings.data.httpUser, settings.data.httpPass)) return request->requestAuthentication();
                sendChunkedHtmlPage(request, HTML_MAIN);});
      

    server.on("/livejson", HTTP_GET, [](AsyncWebServerRequest *request)
              {
                if(strlen(settings.data.httpUser) > 0 && !request->authenticate(settings.data.httpUser, settings.data.httpPass)) return request->requestAuthentication();
                AsyncResponseStream *response = request->beginResponseStream("application/json");
                serializeJson(Json, *response);
                request->send(response); });

    server.on("/favicon.ico", HTTP_GET, [](AsyncWebServerRequest *request)
              {
                AsyncWebServerResponse *response = request->beginResponse_P(200, "image/x-icon", favicon_ico, sizeof(favicon_ico));
                response->addHeader("Cache-Control", "public, max-age=604800");
                request->send(response); });

    server.on("/meta", HTTP_GET, [](AsyncWebServerRequest *request)
              {
                if(strlen(settings.data.httpUser) > 0 && !request->authenticate(settings.data.httpUser, settings.data.httpPass)) return request->requestAuthentication();
                sendMetaJson(request); });

    server.on("/config", HTTP_GET, [](AsyncWebServerRequest *request)
              {
                if(strlen(settings.data.httpUser) > 0 && !request->authenticate(settings.data.httpUser, settings.data.httpPass)) return request->requestAuthentication();
                sendConfigJson(request); });

    server.on("/reboot", HTTP_GET, [](AsyncWebServerRequest *request)
              {
                if(strlen(settings.data.httpUser) > 0 && !request->authenticate(settings.data.httpUser, settings.data.httpPass)) return request->requestAuthentication();
                sendChunkedHtmlPage(request, HTML_REBOOT);
                restartNow = true;
                RestartTimer = millis(); });

    server.on("/confirmreset", HTTP_GET, [](AsyncWebServerRequest *request)
              {
                if(strlen(settings.data.httpUser) > 0 && !request->authenticate(settings.data.httpUser, settings.data.httpPass)) return request->requestAuthentication();
                sendChunkedHtmlPage(request, HTML_CONFIRM_RESET); });
    server.on("/reset", HTTP_GET, [](AsyncWebServerRequest *request)
              {
                if(strlen(settings.data.httpUser) > 0 && !request->authenticate(settings.data.httpUser, settings.data.httpPass)) return request->requestAuthentication();
                AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", "Device is Erasing...");
                response->addHeader("Refresh", "15; url=/");
                response->addHeader("Connection", "close");
                request->send(response);
                delay(1000);
                settings.reset();
                ESP.eraseConfig();
                ESP.restart(); });

    server.on("/settingsedit", HTTP_GET, [](AsyncWebServerRequest *request) {
      if(strlen(settings.data.httpUser) > 0 && !request->authenticate(settings.data.httpUser, settings.data.httpPass)) return request->requestAuthentication();
      sendChunkedHtmlPage(request, HTML_SETTINGS_EDIT);});

    server.on("/settings", HTTP_GET, [](AsyncWebServerRequest *request) {
      if(strlen(settings.data.httpUser) > 0 && !request->authenticate(settings.data.httpUser, settings.data.httpPass)) return request->requestAuthentication();
      sendChunkedHtmlPage(request, HTML_SETTINGS);});

    server.on("/settingssave", HTTP_POST, [](AsyncWebServerRequest *request)
              {
                if(strlen(settings.data.httpUser) > 0 && !request->authenticate(settings.data.httpUser, settings.data.httpPass)) return request->requestAuthentication();
                strncpy(settings.data.mqttServer, request->arg("post_mqttServer").c_str(), sizeof(settings.data.mqttServer) - 1);
                settings.data.mqttServer[sizeof(settings.data.mqttServer) - 1] = '\0';
                settings.data.mqttPort = request->arg("post_mqttPort").toInt();
                strncpy(settings.data.mqttUser, request->arg("post_mqttUser").c_str(), sizeof(settings.data.mqttUser) - 1);
                settings.data.mqttUser[sizeof(settings.data.mqttUser) - 1] = '\0';
                strncpy(settings.data.mqttPassword, request->arg("post_mqttPassword").c_str(), sizeof(settings.data.mqttPassword) - 1);
                settings.data.mqttPassword[sizeof(settings.data.mqttPassword) - 1] = '\0';
                strncpy(settings.data.mqttTopic, request->arg("post_mqttTopic").c_str(), sizeof(settings.data.mqttTopic) - 1);
                settings.data.mqttTopic[sizeof(settings.data.mqttTopic) - 1] = '\0';
                settings.data.mqttRefresh = request->arg("post_mqttRefresh").toInt() < 1 ? 1 : request->arg("post_mqttRefresh").toInt(); // prevent lower numbers
                strncpy(settings.data.deviceName, request->arg("post_deviceName").c_str(), sizeof(settings.data.deviceName) - 1);
                settings.data.deviceName[sizeof(settings.data.deviceName) - 1] = '\0';
                settings.data.mqttJson = (request->arg("post_mqttjson") == "true") ? true : false;
                strncpy(settings.data.mqttTriggerPath, request->arg("post_mqtttrigger").c_str(), sizeof(settings.data.mqttTriggerPath) - 1);
                settings.data.mqttTriggerPath[sizeof(settings.data.mqttTriggerPath) - 1] = '\0';
                settings.data.webUIdarkmode = (request->arg("post_webuicolormode") == "true") ? true : false;
                strncpy(settings.data.httpUser, request->arg("post_httpUser").c_str(), sizeof(settings.data.httpUser) - 1);
                settings.data.httpUser[sizeof(settings.data.httpUser) - 1] = '\0';
                strncpy(settings.data.httpPass, request->arg("post_httpPass").c_str(), sizeof(settings.data.httpPass) - 1);
                settings.data.httpPass[sizeof(settings.data.httpPass) - 1] = '\0';
                settings.data.haDiscovery = (request->arg("post_hadiscovery") == "true") ? true : false;
                settings.save();
                request->redirect("/reboot"); });

    server.on("/set", HTTP_GET, [](AsyncWebServerRequest *request)
              {
                if(strlen(settings.data.httpUser) > 0 && !request->authenticate(settings.data.httpUser, settings.data.httpPass)) return request->requestAuthentication();
                 
                if (request->hasParam("CC")) {
                    const AsyncWebParameter *p = request->getParam("CC");
                    commandFromUser = p->value();
                }
                if (request->hasParam("ha")) {
                    haDiscTrigger = true;
                }
                request->send(200, "text/plain", "message received"); });

    server.on("/debug/loopback-test", HTTP_GET, [](AsyncWebServerRequest *request)
              {
                if(strlen(settings.data.httpUser) > 0 && !request->authenticate(settings.data.httpUser, settings.data.httpPass)) return request->requestAuthentication();
                if (loopbackInProgress || loopbackRequested) {
                  request->send(409, "application/json", "{\"started\":false,\"message\":\"Loopback already running\"}");
                  return;
                }
                loopbackRequested = true;
                loopbackDone = false;
                loopbackMessage = "Loopback started";
                request->send(202, "application/json", "{\"started\":true,\"message\":\"Loopback started\"}"); });

    server.on("/debug/loopback-status", HTTP_GET, [](AsyncWebServerRequest *request)
              {
                if(strlen(settings.data.httpUser) > 0 && !request->authenticate(settings.data.httpUser, settings.data.httpPass)) return request->requestAuthentication();
                String payload = String("{\"running\":") + (loopbackInProgress ? "true" : "false") +
                                 ",\"done\":" + (loopbackDone ? "true" : "false") +
                                 ",\"ok\":" + (loopbackOk ? "true" : "false") +
                                 ",\"message\":\"" + loopbackMessage + "\"}";
                request->send(200, "application/json", payload); });

    server.on("/debug/download", HTTP_GET, [](AsyncWebServerRequest *request)
              {
                if(strlen(settings.data.httpUser) > 0 && !request->authenticate(settings.data.httpUser, settings.data.httpPass)) return request->requestAuthentication();

                bool prevWorkerCanRun = workerCanRun;
                workerCanRun = false;
                mppClient.setSuspend(true);

                auto cleanup = [prevWorkerCanRun]() {
                  mppClient.setSuspend(false);
                  workerCanRun = prevWorkerCanRun;
                };
                request->onDisconnect(cleanup);

                auto state = std::make_shared<DebugDownloadState>();

                AsyncWebServerResponse *response = request->beginChunkedResponse(
                  "text/plain",
                  [state](uint8_t *buffer, size_t maxLen, size_t) -> size_t {
                    size_t written = 0;
                    while (written < maxLen)
                    {
                      if (state->lineOffset >= state->line.length())
                      {
                        if (!buildNextDebugLine(*state))
                        {
                          break;
                        }
                      }

                      size_t available = state->line.length() - state->lineOffset;
                      size_t toCopy = maxLen - written;
                      if (toCopy > available)
                      {
                        toCopy = available;
                      }

                      memcpy(buffer + written, state->line.c_str() + state->lineOffset, toCopy);
                      state->lineOffset += toCopy;
                      written += toCopy;
                    }

                    return written;
                  });

                response->addHeader("Content-Disposition", "attachment; filename=\"solar2mqtt-debug.txt\"");
                response->addHeader("Cache-Control", "no-store");
                response->addHeader("Pragma", "no-cache");
                response->addHeader("Connection", "close");
                request->send(response); });

    auto &debugHandler = server.on("/debug", HTTP_GET, [](AsyncWebServerRequest *request) {
                  if(strlen(settings.data.httpUser) > 0 && !request->authenticate(settings.data.httpUser, settings.data.httpPass)) return request->requestAuthentication();
                  sendChunkedHtmlPage(request, HTML_DEBUG);});
    debugHandler.setFilter([](AsyncWebServerRequest *request) {
                  return request->url() == "/debug";
                });

    server.on(
        "/update", HTTP_POST, [](AsyncWebServerRequest *request)
        {
          if(strlen(settings.data.httpUser) > 0 && !request->authenticate(settings.data.httpUser, settings.data.httpPass)) return request->requestAuthentication();
       //To upload through terminal you can use: curl -F "image=@firmware.bin" <ESP_IP>/update

    //https://gist.github.com/JMishou/60cb762047b735685e8a09cd2eb42a60
    // the request handler is triggered after the upload has finished... 
    // create the response, add header, and send response
    AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", (Update.hasError())?"FAIL":"OK");
    response->addHeader("Connection", "close");
    response->addHeader("Access-Control-Allow-Origin", "*");
    //restartNow = true; // Tell the main loop to restart the ESP
    //RestartTimer = millis();  // Tell the main loop to restart the ESP
    request->send(response); },
        [](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final)
        {
          // Upload handler chunks in data

          if (!index)
          { // if index == 0 then this is the first frame of data
            Serial.printf("UploadStart: %s\n", filename.c_str());
            Serial.setDebugOutput(true);

            // calculate sketch space required for the update
            uint32_t maxSketchSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
            if (!Update.begin(maxSketchSpace))
            { // start with max available size
              Update.printError(Serial);
            }
            Update.runAsync(true); // tell the updaterClass to run in async mode
          }

          // Write chunked data to the free sketch space
          if (Update.write(data, len) != len)
          {
            Update.printError(Serial);
          }

          if (final)
          { // if the final flag is set then this is the last frame of data
            if (Update.end(true))
            { // true to set the size to the current progress
              Serial.printf("Update Success: %u B\nRebooting...\n", index + len);
            }
            else
            {
              Update.printError(Serial);
            }
            Serial.setDebugOutput(false);
          }
        });

    server.onNotFound([](AsyncWebServerRequest *request)
                      { request->send(418, "text/plain", "418 I'm a teapot"); });

    // set the device name

    MDNS.begin(settings.data.deviceName);
    MDNS.addService("http", "tcp", 80);
    ws.onEvent(onEvent);
    server.addHandler(&ws);
    webSerial.begin(&server);
    webSerial.onMessage(recvMsg);

    server.begin();

    mppClient.callback(prozessData);
    mppClient.Init(); // init the PI_serial Library
    mqtttimer = (settings.data.mqttRefresh * 1000) * (-1);
  }

  analogWrite(LED_PIN, 255);
#ifdef isUART_HARDWARE
  analogWrite(LED_COM, 255);
  analogWrite(LED_SRV, 255);
  analogWrite(LED_NET, 255);
#endif
  resetCounter(false);

#ifdef TEMPSENS_PIN
  tempSens.begin(NonBlockingDallas::resolution_12, TIME_INTERVAL);
  tempSens.onTemperatureChange(handleTemperatureChange);
#endif
}

void loop()
{
  MDNS.update();
  handleLoopback();
  if (millis() - wsCleanupTimer > 5000)
  {
    ws.cleanupClients();
    wsCleanupTimer = millis();
  }
  if (Update.isRunning())
  {
    workerCanRun = false; // lockout, atfer true need reboot
  }
  if (workerCanRun)
  {
    // Make sure wifi is in the right mode
    if (WiFi.status() == WL_CONNECTED)
    { 
      // No use going to next step unless WIFI is up and running.
      if (commandFromUser != "")
      {
        if (commandFromUser == "autodetect")
        {
          writeLog("restart autodetect");
          mppClient.Init();
        }
        else if (commandFromUser.startsWith("setp "))
        {
          // Extract the parameter substring after "setp "
          String parameterString = commandFromUser.substring(5);
          int parameter = parameterString.toInt();
          if (parameterString != "0" && parameter == 0)
          {
            writeLog("Invalid parameter for 'setp' command.");
          }
          else if (parameter >= NoD && parameter < PROTOCOL_TYPE_MAX)
          {
            mppClient.protocol = static_cast<protocol_type_t>(parameter);
            writeLog("Change protocol to: %s", protocolStrings[parameter]);
          }
          else
          {
            writeLog("Unknown protocol");
          }
        }
        else
        {
          String tmp = mppClient.sendCommand(commandFromUser); // send a custom command to the device
        }
        commandFromUser = "";
        mqtttimer = 0;
      }
#ifdef TEMPSENS_PIN
      tempSens.update();
#endif

      mppClient.loop();    // Call the PI Serial Library loop
      mqttclient.loop();
      if ((haDiscTrigger || settings.data.haDiscovery) && measureJson(Json) > jsonSize)
      {
        if (sendHaDiscovery())
        {
          haDiscTrigger = false;
          jsonSize = measureJson(Json);
        }
      }
    }
  }
  if (restartNow && millis() >= (RestartTimer + 500))
  {
    ESP.restart();
  }
  notificationLED(); // notification LED routine
}

bool prozessData()
{
  if (millis() < (slowDownTimer + 1000) && mppClient.protocol == 0)
  {
    return true;
  }
  writeLog("ProzessData P:%s C:%s", (String)mppClient.protocol, (String)mppClient.connection);
  getJsonData();
  if (ws.count() > 0)
  {
    notifyClients();
  }
  if (millis() - mqtttimer > (settings.data.mqttRefresh * 1000) || mqtttimer == 0)
  {
#ifdef TEMPSENS_PIN
    tempSens.requestTemperature();
#endif

    sendtoMQTT(); // Update data to MQTT server if we should
    mqtttimer = millis();
  }

  slowDownTimer = millis();
  return true;
}

void getJsonData()
{
  deviceJson[F("Device_name")] = settings.data.deviceName;
  deviceJson[F("ESP_VCC")] = ESP.getVcc() / 1000.0;
  deviceJson[F("Wifi_RSSI")] = WiFi.RSSI();
  deviceJson[F("sw_version")] = SOFTWARE_VERSION;
  deviceJson[F("Free_Heap")] = ESP.getFreeHeap();
  deviceJson[F("HEAP_Fragmentation")] = ESP.getHeapFragmentation();
  deviceJson[F("runtime")] = millis() / 1000;
  deviceJson[F("ws_clients")] = ws.count();
  deviceJson[F("detect_protocol")] = mppClient.protocol;
  deviceJson[F("detect_raw_qpi")] = mppClient.get.raw.qpi;
#ifdef TEMPSENS_PIN
  if (tempSens.indexExist(tempSens.getSensorsCount() - 1))
  {
    for (size_t i = 0; i < tempSens.getSensorsCount(); i++)
    {
      deviceJson["DS18B20_" + String(i + 1)] = tempSens.getTemperatureC(i);
    }
  }
#endif
}

char *topicBuilder(char *buffer, char const *path, char const *numering = "")
{                                                  // buffer, topic
  const char *mainTopic = settings.data.mqttTopic; // get the main topic path
  snprintf(buffer, MQTT_TOPIC_BUFFER_LEN, "%s/%s%s", mainTopic, path, numering);
  return buffer;
}

bool connectMQTT()
{
  if (strcmp(settings.data.mqttServer, "") == 0)
    return false;
  char buff[MQTT_TOPIC_BUFFER_LEN];
  if (!mqttclient.connected())
  {
    firstPublish = false;

    if (mqttclient.connect(mqttClientId, settings.data.mqttUser, settings.data.mqttPassword, (topicBuilder(buff, "Alive")), 0, true, "false", true))
    {
      if (mqttclient.connected())
      {
        mqttclient.publish(topicBuilder(buff, "Alive"), "true", true); // LWT online message must be retained!
        mqttclient.publish(topicBuilder(buff, "IP"), (const char *)(WiFi.localIP().toString()).c_str(), true);
        mqttclient.subscribe(topicBuilder(buff, "DeviceControl/Set_Command"));
        if (strlen(settings.data.mqttTriggerPath) >= 1)
        {
          mqttclient.subscribe(settings.data.mqttTriggerPath);
        }
      }
    }
    else
    {
      return false; // Exit if we couldnt connect to MQTT brooker
    }
    firstPublish = true;
    writeLog("MQTT Client State: %d", mqttclient.state());
  }
  return true;
}

bool sendtoMQTT()
{
  char buff[MQTT_TOPIC_BUFFER_LEN]; // temp buffer for the topic string
  if (!connectMQTT())
  {
    writeLog("No connection to MQTT Server: %d", mqttclient.state());
    firstPublish = false;
    return false;
  }
  if (!settings.data.mqttJson)
  {
    char msgBuffer1[MQTT_TOPIC_BUFFER_LEN];
    for (JsonPair jsonDev : Json.as<JsonObject>())
    {
      for (JsonPair jsondat : jsonDev.value().as<JsonObject>())
      {
        snprintf(msgBuffer1, sizeof(msgBuffer1), "%s/%s/%s", settings.data.mqttTopic, jsonDev.key().c_str(), jsondat.key().c_str());
        mqttclient.publish(msgBuffer1, jsondat.value().as<String>().c_str());
      }
    }
    if (mppClient.get.raw.commandAnswer.length() > 0)
    {
      mqttclient.publish((String(settings.data.mqttTopic) + String("/DeviceControl/Set_Command_answer")).c_str(), (mppClient.get.raw.commandAnswer).c_str());
      writeLog("raw command answer: %s", mppClient.get.raw.commandAnswer.c_str());
      mppClient.get.raw.commandAnswer = "";
    }
    // RAW
    mqttclient.publish(topicBuilder(buff, "RAW/Q1"), (mppClient.get.raw.q1).c_str());
    mqttclient.publish(topicBuilder(buff, "RAW/QPIGS"), (mppClient.get.raw.qpigs).c_str());
    mqttclient.publish(topicBuilder(buff, "RAW/QPIGS2"), (mppClient.get.raw.qpigs2).c_str());
    mqttclient.publish(topicBuilder(buff, "RAW/QPIRI"), (mppClient.get.raw.qpiri).c_str());
    mqttclient.publish(topicBuilder(buff, "RAW/QT"), (mppClient.get.raw.qt).c_str());
    mqttclient.publish(topicBuilder(buff, "RAW/QET"), (mppClient.get.raw.qet).c_str());
    mqttclient.publish(topicBuilder(buff, "RAW/QEY"), (mppClient.get.raw.qey).c_str());
    mqttclient.publish(topicBuilder(buff, "RAW/QEM"), (mppClient.get.raw.qem).c_str());
    mqttclient.publish(topicBuilder(buff, "RAW/QED"), (mppClient.get.raw.qed).c_str());
    mqttclient.publish(topicBuilder(buff, "RAW/QLT"), (mppClient.get.raw.qlt).c_str());
    mqttclient.publish(topicBuilder(buff, "RAW/QLY"), (mppClient.get.raw.qly).c_str());
    mqttclient.publish(topicBuilder(buff, "RAW/QLM"), (mppClient.get.raw.qlm).c_str());
    mqttclient.publish(topicBuilder(buff, "RAW/QLD"), (mppClient.get.raw.qld).c_str());
    mqttclient.publish(topicBuilder(buff, "RAW/QPI"), (mppClient.get.raw.qpi).c_str());
    mqttclient.publish(topicBuilder(buff, "RAW/QMOD"), (mppClient.get.raw.qmod).c_str());
    mqttclient.publish(topicBuilder(buff, "RAW/QALL"), (mppClient.get.raw.qall).c_str());
    mqttclient.publish(topicBuilder(buff, "RAW/QMN"), (mppClient.get.raw.qmn).c_str());
    mqttclient.publish(topicBuilder(buff, "RAW/QPIWS"), (mppClient.get.raw.qpiws).c_str());
    mqttclient.publish(topicBuilder(buff, "RAW/QFLAG"), (mppClient.get.raw.qflag).c_str());
  }
  else
  {
    mqttclient.beginPublish(topicBuilder(buff, "Data"), measureJson(Json), false);
    serializeJson(Json, mqttclient);
    mqttclient.endPublish();
    if (settings.data.haDiscovery || haDiscTrigger)
    {
      char msgBuffer1[MQTT_TOPIC_BUFFER_LEN];
      for (JsonPair jsonDev : Json.as<JsonObject>())
      {
        for (JsonPair jsondat : jsonDev.value().as<JsonObject>())
        {
          snprintf(msgBuffer1, sizeof(msgBuffer1), "%s/%s/%s", settings.data.mqttTopic, jsonDev.key().c_str(), jsondat.key().c_str());
          mqttclient.publish(msgBuffer1, jsondat.value().as<String>().c_str());
        }
      }
    }
  }
  mqttclient.publish(topicBuilder(buff, "Alive"), "true", true); // LWT online message must be retained!
  mqttclient.publish(topicBuilder(buff, "EspData/Wifi_RSSI"), String(WiFi.RSSI()).c_str());
  writeLog("Data sent to MQTT");
  firstPublish = true;

  return true;
}

void mqttcallback(char *top, unsigned char *payload, unsigned int length)
{
  char buff[MQTT_TOPIC_BUFFER_LEN];
  String messageTemp;
  for (unsigned int i = 0; i < length; i++)
  {
    messageTemp += (char)payload[i];
  }
  if (strlen(settings.data.mqttTriggerPath) > 0 && strcmp(top, settings.data.mqttTriggerPath) == 0)
  {
    writeLog("MQTT Data Trigger Firered Up");
    mqtttimer = (settings.data.mqttRefresh * 1000) * (-1);
  }

  if (messageTemp == "NAK" || messageTemp == "(NAK" || messageTemp == "")
    return;

  // send raw control command
  if (strcmp(top, topicBuilder(buff, "DeviceControl/Set_Command")) == 0 && messageTemp.length() > 0)
  {
    writeLog("Command received: %s", messageTemp.c_str());
    commandFromUser = messageTemp;
  }
}

bool sendHaDiscovery()
{
  if (!connectMQTT())
  {
    return false;
  }
  String haDeviceDescription = String("\"dev\":") +
                               "{\"ids\":[\"" + mqttClientId + "\"]," +
                               "\"name\":\"" + settings.data.deviceName + "\"," +
                               "\"cu\":\"http://" + WiFi.localIP().toString() + "\"," +
                               "\"mdl\":\"" + staticData["Device_Model"].as<String>().c_str() + "\"," +
                               "\"mf\":\"SoftWareCrash\"," +
                               "\"sw\":\"" + SOFTWARE_VERSION + "\"" +
                               "}";

  char topBuff[MQTT_TOPIC_BUFFER_LEN];
  for (size_t i = 0; i < sizeof haStaticDescriptor / sizeof haStaticDescriptor[0]; i++)
  {
    if (staticData[haStaticDescriptor[i][0]].is<JsonVariant>())
    {
      String haPayLoad = String("{") +
                         "\"name\":\"" + haStaticDescriptor[i][0] + "\"," +
                         "\"stat_t\":\"" + settings.data.mqttTopic + "/DeviceData/" + haStaticDescriptor[i][0] + "\"," +
                         "\"avty_t\":\"" + settings.data.mqttTopic + "/Alive\"," +
                         "\"pl_avail\": \"true\"," +
                         "\"pl_not_avail\": \"false\"," +
                         "\"uniq_id\":\"" + mqttClientId + "." + haStaticDescriptor[i][0] + "\"," +
                         "\"ic\":\"mdi:" + haStaticDescriptor[i][1] + "\",";
      if (strlen(haStaticDescriptor[i][2]) != 0)
        haPayLoad += (String) "\"unit_of_meas\":\"" + haStaticDescriptor[i][2] + "\",";
      if (strlen(haStaticDescriptor[i][3]) != 0)
        haPayLoad += (String) "\"dev_cla\":\"" + haStaticDescriptor[i][3] + "\",";
      haPayLoad += haDeviceDescription;
      haPayLoad += "}";
      snprintf(topBuff, sizeof(topBuff), "homeassistant/sensor/%s/%s/config", settings.data.mqttTopic, haStaticDescriptor[i][0]); // build the topic
      mqttclient.beginPublish(topBuff, haPayLoad.length(), true);
      for (size_t i = 0; i < haPayLoad.length(); i++)
      {
        mqttclient.write(haPayLoad[i]);
      }
      mqttclient.endPublish();
    }
  }

  for (size_t i = 0; i < sizeof haLiveDescriptor / sizeof haLiveDescriptor[0]; i++)
  {
    if (liveData[haLiveDescriptor[i][0]].is<JsonVariant>())
    {
      String haPayLoad = String("{") +
                         "\"name\":\"" + haLiveDescriptor[i][0] + "\"," +
                         "\"stat_t\":\"" + settings.data.mqttTopic + "/LiveData/" + haLiveDescriptor[i][0] + "\"," +
                         "\"avty_t\":\"" + settings.data.mqttTopic + "/Alive\"," +
                         "\"pl_avail\": \"true\"," +
                         "\"pl_not_avail\": \"false\"," +
                         "\"uniq_id\":\"" + mqttClientId + "." + haLiveDescriptor[i][0] + "\"," +
                         "\"ic\":\"mdi:" + haLiveDescriptor[i][1] + "\",";
      if (strlen(haLiveDescriptor[i][2]) != 0)
        haPayLoad += (String) "\"unit_of_meas\":\"" + haLiveDescriptor[i][2] + "\",";
      if (strlen(haLiveDescriptor[i][3]) != 0)
        haPayLoad += (String) "\"dev_cla\":\"" + haLiveDescriptor[i][3] + "\",";
      haPayLoad += haDeviceDescription;
      haPayLoad += "}";
      snprintf(topBuff, sizeof(topBuff), "homeassistant/sensor/%s/%s/config", settings.data.mqttTopic, haLiveDescriptor[i][0]); // build the topic
      mqttclient.beginPublish(topBuff, haPayLoad.length(), true);
      for (size_t i = 0; i < haPayLoad.length(); i++)
      {
        mqttclient.write(haPayLoad[i]);
      }
      mqttclient.endPublish();
    }
  }
#ifdef TEMPSENS_PIN
  // Ext Temp sensors
  if (tempSens.indexExist(tempSens.getSensorsCount() - 1))
  {
    for (size_t i = 0; i < tempSens.getSensorsCount(); i++)
    {
      String haDeviceDescription = String("\"dev\":") +
                                   "{\"ids\":[\"" + mqttClientId + "\"]," +
                                   "\"name\":\"" + settings.data.deviceName + "\"," +
                                   "\"cu\":\"http://" + WiFi.localIP().toString() + "\"," +
                                   "\"mdl\":\"EPEver2MQTT\"," +
                                   "\"mf\":\"SoftWareCrash\"," +
                                   "\"sw\":\"" + SOFTWARE_VERSION + "\"" +
                                   "}";

      String haPayLoad = String("{") +
                         "\"name\":\"DS18B20_" + (i + 1) + "\"," +
                         "\"stat_t\":\"" + settings.data.mqttTopic + "/DS18B20_" + (i + 1) + "\"," +
                         "\"avty_t\":\"" + settings.data.mqttTopic + "/Alive\"," +
                         "\"pl_avail\": \"true\"," +
                         "\"pl_not_avail\": \"false\"," +
                         "\"uniq_id\":\"" + mqttClientId + ".DS18B20_" + (i + 1) + "\"," +
                         "\"ic\":\"mdi:thermometer-lines\"," +
                         "\"unit_of_meas\":\"°C\"," +
                         "\"dev_cla\":\"temperature\",";
      haPayLoad += haDeviceDescription;
      haPayLoad += "}";
      snprintf(topBuff, sizeof(topBuff), "homeassistant/sensor/%s/DS18B20_%d/config", settings.data.mqttTopic, (i + 1)); // build the topic

      mqttclient.beginPublish(topBuff, haPayLoad.length(), true);
      for (size_t i = 0; i < haPayLoad.length(); i++)
      {
        mqttclient.write(haPayLoad[i]);
      }
      mqttclient.endPublish();
    }
  }
#endif
  return true;
}

void handleTemperatureChange(int deviceIndex, int32_t temperatureRAW)
{
  #ifdef TEMPSENS_PIN
  writeLog("<DS18x> DS18B20_%d RAW:%d Celsius:%f Fahrenheit:%f", deviceIndex + 1, temperatureRAW, tempSens.rawToCelsius(temperatureRAW), tempSens.rawToFahrenheit(temperatureRAW));
  char msgBuffer[32];
  char buff[MQTT_TOPIC_BUFFER_LEN]; // temp buffer for the topic string
  mqttclient.publish(topicBuilder(buff, "DS18B20_", itoa((deviceIndex) + 1, msgBuffer, 10)), dtostrf(tempSens.rawToCelsius(temperatureRAW), 4, 2, msgBuffer));
#endif
}

void writeLog(const char *format, ...)
{
  char msg[100];
  va_list args;

  va_start(args, format);
  vsnprintf(msg, sizeof(msg), format, args); // do check return value
  va_end(args);

  // write msg to the log
  DBG_PRINTLN(msg);
  if (webSerial.getConnectionCount() > 0)
  {
    DBG_WEB(msg);
  }
}

