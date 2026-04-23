# Solar2MQTT-ESP8266 [![GitHub release](https://img.shields.io/github/release/softwarecrash/Solar2MQTT?include_prereleases=&sort=semver&color=blue)](https://github.com/softwarecrash/Solar2MQTT/releases/latest) [![Discord](https://img.shields.io/discord/1007020337482973254?logo=discord&label=Discord)](https://discord.gg/fb2nZWDExz)

# [HERE](https://github.com/softwarecrash/Solar2MQTT) is the ESP32 Next Generation of Solar2MQTT

# Features:
- captive portal for wifi and MQTT config
- config in webinterface
- Full Controll with [Custom commands](https://github.com/softwarecrash/Solar2MQTT-ESP8266/wiki/Set-parameters)
- get essential data over webinterface, get [all data](https://github.com/softwarecrash/Solar2MQTT-ESP8266/wiki/Datapoints-and-units) over MQTT
- classic MQTT datapoints or Json string over MQTT
- get Json over web at /livejson?
- firmware update over webinterface
- debug log on Wemos USB or Webserial
- [blink codes](https://github.com/softwarecrash/Solar2MQTT-ESP8266/wiki/Blink-Codes) for the current state of the ESP
- [Reset functions](https://github.com/softwarecrash/Solar2MQTT-ESP8266/wiki/Reset)
- [Support Home Assistant](https://github.com/softwarecrash/Solar2MQTT-ESP8266/wiki/HomeAssistant-integration)
- with Teapod




**works with**
- Most devices that use the watchpower PC Software
-  Most devices that use the Solarpower PC Software
- PIP devices
- i solar 
- IGrid
- Many devices from EASUN
- and many many others based on the chinese solar inverter with a rj45 jack and usb port, primary identified by the display
- Take a look at the [device list in the wiki](https://github.com/softwarecrash/Solar2MQTT-ESP8266/wiki/Confirmed-Working-Device-List)


**Main screen:**

![image](https://github.com/softwarecrash/Solar2MQTT-ESP8266/assets/17761850/de945ad5-29ad-476e-9562-a0eba1b4f2ce)

**Settings:**

![image](https://github.com/softwarecrash/Solar2MQTT-ESP8266/assets/17761850/075d1e66-3912-4a33-b7d3-a52da99c8553)

**Config:**

![image](https://github.com/softwarecrash/Solar2MQTT-ESP8266/assets/17761850/823093bf-8abe-4b7e-913f-7bac9420d108)





# How to use:
- flash your ESP8266 (recommended Wemos D1 Mini) with our [Flash2MQTT-Tool](https://all-solutions.github.io/Flash2MQTT/?get=Solar2MQTT) or with our desktop [2MQTT-Flasher](https://github.com/all-solutions/2MQTT-Flasher)
- connect the ESP like the [wiring diagram](https://github.com/softwarecrash/Solar2MQTT-ESP8266/wiki/Wiring-Diagram)
- search for the wifi ap "Solar2MQTT-AP" and connect to it
- surf to 192.168.4.1 and set up your wifi and optional MQTT
- that's it :)

### How-To video by Jarnsen

<a href="http://www.youtube.com/watch?feature=player_embedded&v=7u8hPLdXeso" target="_blank">
 <img src="http://img.youtube.com/vi/7u8hPLdXeso/0.jpg" alt="Watch the video" />
</a>



**POWER:** Using a 3.3V DC Buck Converter that can handle up to 20V or a DC/DC or USB power currently.

# Parts required to build

Most of the parts can be bought as modules, it's usually cheaper that way.

- ESP8266 - Wemos D1 Mini or ESP8266-01
- MAX3232 module Like this https://amzn.eu/d/8t3gk5t or https://bit.ly/3BFPqrw or with orginal cable https://www.amazon.de/dp/B09XWPTDYP
- DC-DC buck module - 12-80v down to 5v

# Completely assembled and tested PCB's

You are welcome to get fully stocked and tested PCB's. These are then already loaded with the lastest firmware. The earnings from the PCBs are used for the further development of existing and new projects.

![Solar-MQTT-PCB](https://user-images.githubusercontent.com/17761850/233859179-cc9c9075-b88a-4f38-b804-bc0f409cf8ce.png)

If interested see [here](https://all-solutions.store)

#
Questions?
[Join the Discord Channel (German / English)](https://discord.gg/pAArqVsVS4)

#
[<img src="https://cdn.buymeacoffee.com/buttons/default-orange.png" alt="Buy Me A Coffee" height="41" width="174"/>](https://donate.softwarecrash.de)

[![LICENSE](https://licensebuttons.net/l/by-nc-nd/4.0/88x31.png)](https://creativecommons.org/licenses/by-nc-nd/4.0/)

