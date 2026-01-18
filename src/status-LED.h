#include <Arduino.h>
// LED blink codes every 5 seconds:
// 1x all ok, 2x no inverter, 3x no MQTT, 4x no WiFi.
/*
  Blinking LED = Relais Off
  Waveing LED = Relais On
  every 5 seconds:
  1x all ok - Working
  2x no Inverter Connection
  3x no MQTT Connection
  4x no WiFi Connection
*/
unsigned int ledPin = 0;
unsigned int ledTimer = 0;
unsigned int repeatTime = 5000;
unsigned int cycleTime = 250;
unsigned int cycleMillis = 0;
byte ledState = 0;

void notificationLED()
{

  if (millis() >= (ledTimer + repeatTime) && ledState == 0)
  {
    if (WiFi.status() != WL_CONNECTED)
      ledState = 4;
    else if (!mqttclient.connected() && strcmp(settings.data.mqttServer, "") != 0)
      ledState = 3;
    else if (!mppClient.connection)
      ledState = 2;
    else if (WiFi.status() == WL_CONNECTED && mqttclient.connected() && mppClient.connection)
      ledState = 1;

#ifdef isUART_HARDWARE
    digitalWrite(LED_COM, !mppClient.connection);   // make it blink blink when communication
    digitalWrite(LED_SRV, !mqttclient.connected()); // make it blinky when sending data
    digitalWrite(LED_NET, !(WiFi.status() == WL_CONNECTED) ? true : false);
#endif
  }

  if (ledState > 0)
  {
    if (millis() >= (cycleMillis + cycleTime))
    {
      if (ledPin == 0)
      {
        ledPin = 255;
      }
      else
      {
        ledPin = 0;
        ledState--;
      }
      cycleMillis = millis();
      if (ledState == 0)
      {
        ledTimer = millis();
      }
    }
  }
  analogWrite(LED_PIN, 255 - ledPin);
}
