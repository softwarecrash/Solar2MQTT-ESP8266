#ifndef MODBUS_H
#define MODBUS_H

#include "SoftwareSerial.h"
#include <ArduinoJson.h>
#include "modbus_com.h"
#include "device/modbus_device.h"
#include "device/must_pv_ph18/must_pv_ph18.h"
#include "device/deye/deye.h"
#include "device/anenji/anenji.h"

extern JsonObject deviceJson;
extern JsonObject staticData;
extern JsonObject liveData;

class MODBUS
{
public:
    const uint8_t MAX_CONNECTION_ATTEMPTS = 10;
    bool requestStaticData = true;
    bool connection = false;
    modbus_register_info_t live_info;
    modbus_register_info_t static_info;
    MODBUS(SoftwareSerial *port);

    /**
     * @brief Initializes this driver
     * @details Configures the serial peripheral and pre-loads the transmit buffer with command-independent bytes
     */
    bool Init();

    /**
     * @brief Updating the Data from the inverter
     */
    void loop();

    /**
     * @brief callback function
     *
     */
    void callback(std::function<void()> func);
    std::function<void()> requestCallback; 
    protocol_type_t autoDetect();
    /**
     * @brief Sends a complete packet with the specified command
     * @details sends the command over the specified serial connection
     */
    String requestData(String command);

private:
    unsigned long previousTime = 0;
    unsigned long cmdDelayTime = 100;

    byte requestCounter = 0;

    long long int connectionCounter = 0;

    byte qexCounter = 0;

    void prepareRegisters(); 

    /**
     * @brief Serial interface used for communication
     * @details This is set in the constructor
     */
    SoftwareSerial *my_serialIntf;
    ModbusDevice *device = nullptr; 
    MODBUS_COM _mCom;
};

#endif
