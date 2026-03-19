
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
DNSServer dns;
Settings settings;
WebSerial webSerial;

#ifdef TEMPSENS_PIN
OneWire oneWire(TEMPSENS_PIN);
DallasTemperature dallasTemp(&oneWire);
NonBlockingDallas tempSens(&dallasTemp);
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
bool restartNow = false;
bool workerCanRun = true;
bool haDiscTrigger = false;
unsigned int jsonSize = 0;
uint32_t bootcount = 0;
String commandFromUser;
bool loopbackRequested = false;
bool loopbackInProgress = false;
bool loopbackDone = false;
bool loopbackOk = false;
bool loopbackPrevWorker = true;
unsigned long loopbackWaitStart = 0;
String loopbackMessage;
const uint32_t HA_MIN_HEAP = 12000;
const uint32_t HA_MIN_INTERVAL_MS = 60000;
unsigned long haDiscLastAttempt = 0;
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

static constexpr size_t MAIN_JSON_DOC_SIZE = 8192;
StaticJsonDocument<MAIN_JSON_DOC_SIZE> Json;                 // main Json (static to reduce heap churn)
JsonObject deviceJson = Json["EspData"].to<JsonObject>();    // basic device data
JsonObject staticData = Json["DeviceData"].to<JsonObject>(); // battery package data
JsonObject liveData = Json["LiveData"].to<JsonObject>();     // battery package data


//----------------------------------------------------------------------
void saveConfigCallback()
{
  writeLog("Should save config");
  shouldSaveConfig = true;
}

static void appendJsonVariant(Print &out, JsonVariantConst value)
{
  if (value.isUnbound() || value.isNull())
  {
    out.print(F("null"));
    return;
  }
  serializeJson(value, out);
}

static void copyLiveAlias(const char *targetKey, const char *sourceKey)
{
  JsonVariantConst value = liveData[sourceKey];
  if (!value.isUnbound() && !value.isNull())
  {
    liveData[targetKey] = value;
  }
}

static void syncLegacyLiveDataNames()
{
  copyLiveAlias("Battery_capacity", DESCR_Battery_Percent);
  copyLiveAlias("Grid_voltage", DESCR_AC_In_Voltage);
  copyLiveAlias("Grid_frequency", DESCR_AC_In_Frequenz);

  JsonVariantConst chargingPower = liveData[DESCR_PV_Charging_Power];
  if (chargingPower.isUnbound() || chargingPower.isNull())
  {
    copyLiveAlias(DESCR_PV_Charging_Power, DESCR_PV_Input_Power);
  }

  JsonVariantConst inputPower = liveData[DESCR_PV_Input_Power];
  if (inputPower.isUnbound() || inputPower.isNull())
  {
    copyLiveAlias(DESCR_PV_Input_Power, DESCR_PV_Charging_Power);
  }
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
    line = F("Q1: ");
    line += mppClient.get.raw.q1;
    break;
  case 1:
    line = F("QPIGS: ");
    line += mppClient.get.raw.qpigs;
    break;
  case 2:
    line = F("QPIGS2: ");
    line += mppClient.get.raw.qpigs2;
    break;
  case 3:
    line = F("QPIRI: ");
    line += mppClient.get.raw.qpiri;
    break;
  case 4:
    line = F("QT: ");
    line += mppClient.get.raw.qt;
    break;
  case 5:
    line = F("QET: ");
    line += mppClient.get.raw.qet;
    break;
  case 6:
    line = F("QEY: ");
    line += mppClient.get.raw.qey;
    break;
  case 7:
    line = F("QEM: ");
    line += mppClient.get.raw.qem;
    break;
  case 8:
    line = F("QED: ");
    line += mppClient.get.raw.qed;
    break;
  case 9:
    line = F("QLT: ");
    line += mppClient.get.raw.qlt;
    break;
  case 10:
    line = F("QLY: ");
    line += mppClient.get.raw.qly;
    break;
  case 11:
    line = F("QLM: ");
    line += mppClient.get.raw.qlm;
    break;
  case 12:
    line = F("QLD: ");
    line += mppClient.get.raw.qld;
    break;
  case 13:
    line = F("QPI: ");
    line += mppClient.get.raw.qpi;
    break;
  case 14:
    line = F("QMOD: ");
    line += mppClient.get.raw.qmod;
    break;
  case 15:
    line = F("QALL: ");
    line += mppClient.get.raw.qall;
    break;
  case 16:
    line = F("QMN: ");
    line += mppClient.get.raw.qmn;
    break;
  case 17:
    line = F("QPIWS: ");
    line += mppClient.get.raw.qpiws;
    break;
  case 18:
    line = F("QFLAG: ");
    line += mppClient.get.raw.qflag;
    break;
  case 19:
    line = F("CommandAnswer: ");
    line += mppClient.get.raw.commandAnswer;
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
      {
        char uptimeBuf[16];
        snprintf(uptimeBuf, sizeof(uptimeBuf), "%lu", millis() / 1000);
        state.line += uptimeBuf;
      }
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
    if (kv.value().is<const char *>())
    {
      state.line += kv.value().as<const char *>();
    }
    else
    {
      char valBuf[64];
      size_t n = serializeJson(kv.value(), valBuf, sizeof(valBuf) - 1);
      valBuf[n] = '\0';
      state.line += valBuf;
    }
    state.line += "\n";
    return true;
  }

  return false;
}

/* Message callback of WebSerial */
void recvMsg(uint8_t *data, size_t len)
{
  String d = "";
  d.reserve(len);
  for (uint i = 0; i < len; i++)
  {
    d += char(data[i]);
  }
  commandFromUser = (d);
  webSerial.print(F("Sending ["));
  webSerial.print(d);
  webSerial.println(F("] to Device"));
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

struct HtmlChunkState
{
  const char *chunks[3];
  size_t chunkLengths[3];
  uint8_t chunkIndex = 0;
  size_t chunkOffset = 0;
};

static void appendJsonString(Print &out, const char *value)
{
  out.print('"');
  if (value != nullptr)
  {
    for (const char *p = value; *p != '\0'; ++p)
    {
      char c = *p;
      switch (c)
      {
      case '\\':
      case '"':
        out.print('\\');
        out.print(c);
        break;
      case '\n':
        out.print(F("\\n"));
        break;
      case '\r':
        out.print(F("\\r"));
        break;
      case '\t':
        out.print(F("\\t"));
        break;
      default:
        out.print(c);
        break;
      }
    }
  }
  out.print('"');
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
  AsyncResponseStream *response = request->beginResponseStream("application/json", 128);
  response->print('{');
  response->print(F("\"device_name\":"));
  appendJsonString(*response, settings.data.deviceName);
  response->print(F(",\"software_version\":"));
  appendJsonString(*response, SOFTWARE_VERSION);
  response->print(F(",\"sw_version\":"));
  appendJsonString(*response, SWVERSION);
  response->print(F(",\"dark_mode\":"));
  response->print(settings.data.webUIdarkmode ? F("true") : F("false"));
  response->print(F(",\"flash_size\":"));
  response->print(ESP.getFreeSketchSpace());
  response->print('}');
  request->send(response);
}

void sendConfigJson(AsyncWebServerRequest *request)
{
  AsyncResponseStream *response = request->beginResponseStream("application/json", 256);
  response->print('{');
  response->print(F("\"device_name\":"));
  appendJsonString(*response, settings.data.deviceName);
  response->print(F(",\"mqtt_server\":"));
  appendJsonString(*response, settings.data.mqttServer);
  response->print(F(",\"mqtt_port\":"));
  response->print(settings.data.mqttPort);
  response->print(F(",\"mqtt_user\":"));
  appendJsonString(*response, settings.data.mqttUser);
  response->print(F(",\"mqtt_password\":"));
  appendJsonString(*response, settings.data.mqttPassword);
  response->print(F(",\"mqtt_topic\":"));
  appendJsonString(*response, settings.data.mqttTopic);
  response->print(F(",\"mqtt_refresh\":"));
  response->print(settings.data.mqttRefresh);
  response->print(F(",\"mqtt_trigger\":"));
  appendJsonString(*response, settings.data.mqttTriggerPath);
  response->print(F(",\"mqtt_json\":"));
  response->print(settings.data.mqttJson ? F("true") : F("false"));
  response->print(F(",\"ha_discovery\":"));
  response->print(settings.data.haDiscovery ? F("true") : F("false"));
  response->print(F(",\"webui_dark_mode\":"));
  response->print(settings.data.webUIdarkmode ? F("true") : F("false"));
  response->print(F(",\"http_user\":"));
  appendJsonString(*response, settings.data.httpUser);
  response->print(F(",\"http_pass\":"));
  appendJsonString(*response, settings.data.httpPass);
  response->print('}');
  request->send(response);
}

void sendLiveJson(AsyncWebServerRequest *request)
{
  AsyncResponseStream *response = request->beginResponseStream("application/json", 192);
  response->print('[');
  response->print(WiFi.RSSI());
  response->print(',');
  appendJsonVariant(*response, liveData[DESCR_Battery_Percent]);
  response->print(',');
  appendJsonVariant(*response, liveData[DESCR_PV_Input_Voltage]);
  response->print(',');
  appendJsonVariant(*response, liveData[DESCR_PV_Input_Current]);
  response->print(',');
  appendJsonVariant(*response, liveData[DESCR_PV_Charging_Power]);
  response->print(',');
  appendJsonVariant(*response, liveData[DESCR_PV2_Input_Voltage]);
  response->print(',');
  appendJsonVariant(*response, liveData[DESCR_PV2_Input_Current]);
  response->print(',');
  appendJsonVariant(*response, liveData[DESCR_PV2_Charging_Power]);
  response->print(',');
  appendJsonVariant(*response, liveData[DESCR_AC_In_Voltage]);
  response->print(',');
  appendJsonVariant(*response, liveData[DESCR_AC_In_Frequenz]);
  response->print(',');
  appendJsonVariant(*response, liveData[DESCR_AC_Out_Voltage]);
  response->print(',');
  appendJsonVariant(*response, liveData[DESCR_AC_Out_Frequenz]);
  response->print(',');
  appendJsonVariant(*response, liveData[DESCR_AC_Out_Watt]);
  response->print(',');
  appendJsonVariant(*response, liveData[DESCR_AC_Out_Percent]);
  response->print(',');
  appendJsonVariant(*response, liveData[DESCR_Inverter_Bus_Temperature]);
  response->print(',');
  appendJsonVariant(*response, liveData[DESCR_Battery_Voltage]);
  response->print(',');
  appendJsonVariant(*response, liveData[DESCR_Battery_Load]);
  response->print(',');
  appendJsonVariant(*response, liveData[DESCR_Inverter_Operation_Mode]);
  response->print(']');
  request->send(response);
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
  commandFromUser.reserve(32);
  loopbackMessage.reserve(32);
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

  wm.setDebugOutput(false); // disable wifimanager debug output
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
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
                if(strlen(settings.data.httpUser) > 0 && !request->authenticate(settings.data.httpUser, settings.data.httpPass)) return request->requestAuthentication();
                sendChunkedHtmlPage(request, HTML_MAIN);});
      

    server.on("/livejson", HTTP_GET, [](AsyncWebServerRequest *request)
              {
                if(strlen(settings.data.httpUser) > 0 && !request->authenticate(settings.data.httpUser, settings.data.httpPass)) return request->requestAuthentication();
                sendLiveJson(request); });

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
                AsyncResponseStream *response = request->beginResponseStream("application/json", 128);
                response->print(F("{\"running\":"));
                response->print(loopbackInProgress ? F("true") : F("false"));
                response->print(F(",\"done\":"));
                response->print(loopbackDone ? F("true") : F("false"));
                response->print(F(",\"ok\":"));
                response->print(loopbackOk ? F("true") : F("false"));
                response->print(F(",\"message\":"));
                appendJsonString(*response, loopbackMessage.c_str());
                response->print('}');
                request->send(response); });

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
                state->line.reserve(256);

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
          mppClient.sendCommand(commandFromUser); // send a custom command to the device
        }
        commandFromUser = "";
        mqtttimer = 0;
      }
#ifdef TEMPSENS_PIN
      tempSens.update();
#endif

      mppClient.loop();    // Call the PI Serial Library loop
      mqttclient.loop();
      if (haDiscTrigger || settings.data.haDiscovery)
      {
        if (ESP.getFreeHeap() >= HA_MIN_HEAP &&
            (millis() - haDiscLastAttempt) >= HA_MIN_INTERVAL_MS &&
            measureJson(Json) > jsonSize)
        {
          haDiscLastAttempt = millis();
          if (sendHaDiscovery())
          {
            haDiscTrigger = false;
            jsonSize = measureJson(Json);
          }
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
  syncLegacyLiveDataNames();
  deviceJson[F("Device_name")] = settings.data.deviceName;
  deviceJson[F("ESP_VCC")] = ESP.getVcc() / 1000.0;
  deviceJson[F("Wifi_RSSI")] = WiFi.RSSI();
  deviceJson[F("sw_version")] = SOFTWARE_VERSION;
  deviceJson[F("Free_Heap")] = ESP.getFreeHeap();
  deviceJson[F("HEAP_Fragmentation")] = ESP.getHeapFragmentation();
  deviceJson[F("runtime")] = millis() / 1000;
  deviceJson[F("detect_protocol")] = mppClient.protocol;
  deviceJson[F("detect_raw_qpi")] = mppClient.get.raw.qpi;
#ifdef TEMPSENS_PIN
  if (tempSens.indexExist(tempSens.getSensorsCount() - 1))
  {
    for (size_t i = 0; i < tempSens.getSensorsCount(); i++)
    {
      char key[16];
      snprintf(key, sizeof(key), "DS18B20_%u", static_cast<unsigned int>(i + 1));
      deviceJson[key] = tempSens.getTemperatureC(i);
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

static void publishJsonValue(PubSubClient &client, const char *topic, JsonVariantConst value)
{
  const char *str = value.as<const char *>();
  if (str != nullptr)
  {
    client.publish(topic, str);
    return;
  }
  if (value.isNull())
  {
    client.publish(topic, "");
    return;
  }
  char buffer[64];
  size_t n = serializeJson(value, buffer, sizeof(buffer) - 1);
  buffer[n] = '\0';
  client.publish(topic, buffer);
}

bool connectMQTT()
{
  if (strcmp(settings.data.mqttServer, "") == 0)
    return false;
  char buff[MQTT_TOPIC_BUFFER_LEN];
  if (!mqttclient.connected())
  {
    if (mqttclient.connect(mqttClientId, settings.data.mqttUser, settings.data.mqttPassword, (topicBuilder(buff, "Alive")), 0, true, "false", true))
    {
      if (mqttclient.connected())
      {
        mqttclient.publish(topicBuilder(buff, "Alive"), "true", true); // LWT online message must be retained!
        char ipStr[16];
        IPAddress ip = WiFi.localIP();
        snprintf(ipStr, sizeof(ipStr), "%u.%u.%u.%u", ip[0], ip[1], ip[2], ip[3]);
        mqttclient.publish(topicBuilder(buff, "IP"), ipStr, true);
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
        publishJsonValue(mqttclient, msgBuffer1, jsondat.value());
      }
    }
    if (mppClient.get.raw.commandAnswer.length() > 0)
    {
      char cmdTopic[MQTT_TOPIC_BUFFER_LEN];
      snprintf(cmdTopic, sizeof(cmdTopic), "%s/DeviceControl/Set_Command_answer", settings.data.mqttTopic);
      mqttclient.publish(cmdTopic, (mppClient.get.raw.commandAnswer).c_str());
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
          publishJsonValue(mqttclient, msgBuffer1, jsondat.value());
        }
      }
    }
  }
  mqttclient.publish(topicBuilder(buff, "Alive"), "true", true); // LWT online message must be retained!
  {
    char rssiBuf[12];
    snprintf(rssiBuf, sizeof(rssiBuf), "%d", WiFi.RSSI());
    mqttclient.publish(topicBuilder(buff, "EspData/Wifi_RSSI"), rssiBuf);
  }
  writeLog("Data sent to MQTT");
  return true;
}

void mqttcallback(char *top, unsigned char *payload, unsigned int length)
{
  char buff[MQTT_TOPIC_BUFFER_LEN];
  String messageTemp;
  messageTemp.reserve(length);
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

static size_t appendFmt(char *buffer, size_t bufferSize, size_t pos, const char *fmt, ...)
{
  if (pos >= bufferSize)
  {
    return pos;
  }
  va_list args;
  va_start(args, fmt);
  int written = vsnprintf(buffer + pos, bufferSize - pos, fmt, args);
  va_end(args);
  if (written < 0)
  {
    return pos;
  }
  size_t w = static_cast<size_t>(written);
  if (w >= bufferSize - pos)
  {
    return bufferSize - 1;
  }
  return pos + w;
}

static const char *readDescriptorString(const char *const (*table)[4], size_t row, size_t col)
{
  return reinterpret_cast<const char *>(pgm_read_ptr(&table[row][col]));
}

bool sendHaDiscovery()
{
  if (ESP.getFreeHeap() < HA_MIN_HEAP)
  {
    writeLog("HA discovery skipped (low heap)");
    return false;
  }
  if (!connectMQTT())
  {
    return false;
  }
  const char *deviceModel = staticData["Device_Model"] | "";
  char ipStr[16];
  IPAddress ip = WiFi.localIP();
  snprintf(ipStr, sizeof(ipStr), "%u.%u.%u.%u", ip[0], ip[1], ip[2], ip[3]);
  char deviceDesc[256];
  size_t devicePos = 0;
  devicePos = appendFmt(deviceDesc, sizeof(deviceDesc), devicePos, "\"dev\":{");
  devicePos = appendFmt(deviceDesc, sizeof(deviceDesc), devicePos, "\"ids\":[\"%s\"],", mqttClientId);
  devicePos = appendFmt(deviceDesc, sizeof(deviceDesc), devicePos, "\"name\":\"%s\",", settings.data.deviceName);
  devicePos = appendFmt(deviceDesc, sizeof(deviceDesc), devicePos, "\"cu\":\"http://%s\",", ipStr);
  devicePos = appendFmt(deviceDesc, sizeof(deviceDesc), devicePos, "\"mdl\":\"%s\",", deviceModel);
  devicePos = appendFmt(deviceDesc, sizeof(deviceDesc), devicePos, "\"mf\":\"SoftWareCrash\",");
  devicePos = appendFmt(deviceDesc, sizeof(deviceDesc), devicePos, "\"sw\":\"%s\"}", SOFTWARE_VERSION);

  char topBuff[MQTT_TOPIC_BUFFER_LEN];
  char payload[512];
  for (size_t i = 0; i < sizeof haStaticDescriptor / sizeof haStaticDescriptor[0]; i++)
  {
    const char *name = readDescriptorString(haStaticDescriptor, i, 0);
    const char *icon = readDescriptorString(haStaticDescriptor, i, 1);
    const char *unit = readDescriptorString(haStaticDescriptor, i, 2);
    const char *devClass = readDescriptorString(haStaticDescriptor, i, 3);
    if (staticData[name].is<JsonVariant>())
    {
      size_t pos = 0;
      pos = appendFmt(payload, sizeof(payload), pos, "{");
      pos = appendFmt(payload, sizeof(payload), pos, "\"name\":\"%s\",", name);
      pos = appendFmt(payload, sizeof(payload), pos, "\"stat_t\":\"%s/DeviceData/%s\",", settings.data.mqttTopic, name);
      pos = appendFmt(payload, sizeof(payload), pos, "\"avty_t\":\"%s/Alive\",", settings.data.mqttTopic);
      pos = appendFmt(payload, sizeof(payload), pos, "\"pl_avail\":\"true\",");
      pos = appendFmt(payload, sizeof(payload), pos, "\"pl_not_avail\":\"false\",");
      pos = appendFmt(payload, sizeof(payload), pos, "\"uniq_id\":\"%s.%s\",", mqttClientId, name);
      pos = appendFmt(payload, sizeof(payload), pos, "\"ic\":\"mdi:%s\",", icon);
      if (unit != nullptr && unit[0] != '\0')
      {
        pos = appendFmt(payload, sizeof(payload), pos, "\"unit_of_meas\":\"%s\",", unit);
      }
      if (devClass != nullptr && devClass[0] != '\0')
      {
        pos = appendFmt(payload, sizeof(payload), pos, "\"dev_cla\":\"%s\",", devClass);
      }
      pos = appendFmt(payload, sizeof(payload), pos, "%s}", deviceDesc);
      snprintf(topBuff, sizeof(topBuff), "homeassistant/sensor/%s/%s/config", settings.data.mqttTopic, name); // build the topic
      mqttclient.beginPublish(topBuff, pos, true);
      mqttclient.write((const uint8_t *)payload, pos);
      mqttclient.endPublish();
    }
  }

  for (size_t i = 0; i < sizeof haLiveDescriptor / sizeof haLiveDescriptor[0]; i++)
  {
    const char *name = readDescriptorString(haLiveDescriptor, i, 0);
    const char *icon = readDescriptorString(haLiveDescriptor, i, 1);
    const char *unit = readDescriptorString(haLiveDescriptor, i, 2);
    const char *devClass = readDescriptorString(haLiveDescriptor, i, 3);
    if (liveData[name].is<JsonVariant>())
    {
      size_t pos = 0;
      pos = appendFmt(payload, sizeof(payload), pos, "{");
      pos = appendFmt(payload, sizeof(payload), pos, "\"name\":\"%s\",", name);
      pos = appendFmt(payload, sizeof(payload), pos, "\"stat_t\":\"%s/LiveData/%s\",", settings.data.mqttTopic, name);
      pos = appendFmt(payload, sizeof(payload), pos, "\"avty_t\":\"%s/Alive\",", settings.data.mqttTopic);
      pos = appendFmt(payload, sizeof(payload), pos, "\"pl_avail\":\"true\",");
      pos = appendFmt(payload, sizeof(payload), pos, "\"pl_not_avail\":\"false\",");
      pos = appendFmt(payload, sizeof(payload), pos, "\"uniq_id\":\"%s.%s\",", mqttClientId, name);
      pos = appendFmt(payload, sizeof(payload), pos, "\"ic\":\"mdi:%s\",", icon);
      if (unit != nullptr && unit[0] != '\0')
      {
        pos = appendFmt(payload, sizeof(payload), pos, "\"unit_of_meas\":\"%s\",", unit);
      }
      if (devClass != nullptr && devClass[0] != '\0')
      {
        pos = appendFmt(payload, sizeof(payload), pos, "\"dev_cla\":\"%s\",", devClass);
      }
      pos = appendFmt(payload, sizeof(payload), pos, "%s}", deviceDesc);
      snprintf(topBuff, sizeof(topBuff), "homeassistant/sensor/%s/%s/config", settings.data.mqttTopic, name); // build the topic
      mqttclient.beginPublish(topBuff, pos, true);
      mqttclient.write((const uint8_t *)payload, pos);
      mqttclient.endPublish();
    }
  }
#ifdef TEMPSENS_PIN
  // Ext Temp sensors
  if (tempSens.indexExist(tempSens.getSensorsCount() - 1))
  {
    for (size_t i = 0; i < tempSens.getSensorsCount(); i++)
    {
      char tempDeviceDesc[256];
      size_t tempDescPos = 0;
      tempDescPos = appendFmt(tempDeviceDesc, sizeof(tempDeviceDesc), tempDescPos, "\"dev\":{");
      tempDescPos = appendFmt(tempDeviceDesc, sizeof(tempDeviceDesc), tempDescPos, "\"ids\":[\"%s\"],", mqttClientId);
      tempDescPos = appendFmt(tempDeviceDesc, sizeof(tempDeviceDesc), tempDescPos, "\"name\":\"%s\",", settings.data.deviceName);
      tempDescPos = appendFmt(tempDeviceDesc, sizeof(tempDeviceDesc), tempDescPos, "\"cu\":\"http://%s\",", ipStr);
      tempDescPos = appendFmt(tempDeviceDesc, sizeof(tempDeviceDesc), tempDescPos, "\"mdl\":\"EPEver2MQTT\",");
      tempDescPos = appendFmt(tempDeviceDesc, sizeof(tempDeviceDesc), tempDescPos, "\"mf\":\"SoftWareCrash\",");
      tempDescPos = appendFmt(tempDeviceDesc, sizeof(tempDeviceDesc), tempDescPos, "\"sw\":\"%s\"}", SOFTWARE_VERSION);

      size_t pos = 0;
      pos = appendFmt(payload, sizeof(payload), pos, "{");
      pos = appendFmt(payload, sizeof(payload), pos, "\"name\":\"DS18B20_%d\",", (i + 1));
      pos = appendFmt(payload, sizeof(payload), pos, "\"stat_t\":\"%s/DS18B20_%d\",", settings.data.mqttTopic, (i + 1));
      pos = appendFmt(payload, sizeof(payload), pos, "\"avty_t\":\"%s/Alive\",", settings.data.mqttTopic);
      pos = appendFmt(payload, sizeof(payload), pos, "\"pl_avail\":\"true\",");
      pos = appendFmt(payload, sizeof(payload), pos, "\"pl_not_avail\":\"false\",");
      pos = appendFmt(payload, sizeof(payload), pos, "\"uniq_id\":\"%s.DS18B20_%d\",", mqttClientId, (i + 1));
      pos = appendFmt(payload, sizeof(payload), pos, "\"ic\":\"mdi:thermometer-lines\",");
      pos = appendFmt(payload, sizeof(payload), pos, "\"unit_of_meas\":\"°C\",");
      pos = appendFmt(payload, sizeof(payload), pos, "\"dev_cla\":\"temperature\",");
      pos = appendFmt(payload, sizeof(payload), pos, "%s}", tempDeviceDesc);
      snprintf(topBuff, sizeof(topBuff), "homeassistant/sensor/%s/DS18B20_%d/config", settings.data.mqttTopic, (i + 1)); // build the topic

      mqttclient.beginPublish(topBuff, pos, true);
      mqttclient.write((const uint8_t *)payload, pos);
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

