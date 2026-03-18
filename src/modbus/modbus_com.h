#ifndef MODBUS_COM_H
#define MODBUS_COM_H

#include "SoftwareSerial.h"
#include <ArduinoJson.h>
#include <ModbusMaster.h>
#include "modbus_registers.h"

#define MODBUS_RETRIES 2
#define MODBUS_TIMEOUT 150

#define RS485_DIR_PIN 14 // D5
#define RS485_ESP01_DIR_PIN 0

#define MAX485_DONGLE_DE_PIN 5 // D1, DE pin on the TTL to RS485 converter
#define MAX485_DONGLE_RE_NEG_PIN 4

typedef enum
{
    READ_FAIL = 0,
    READ_OK = 1,
} response_type_t;
 

/**
 * @class MODBUS_COM
 * @brief This class is responsible for Modbus communication, managing the RS485 transceiver, and interacting with a Modbus master.
 * 
 * The class is designed to handle communication through Modbus over an RS485 network. It supports reading Modbus registers and converting them to JSON format.
 * The communication direction (transmission/reception) is managed through digital pins, and the class provides helper functions for decoding various register types.
 */ 
class MODBUS_COM
{
public:
    MODBUS_COM();

    bool readModbusRegisterToJson(const modbus_register_t *reg, JsonObject *variant);
    response_type_t parseModbusToJson(modbus_register_info_t &register_info, bool skip_reg_on_error = true);
    bool isAllRegistersRead(modbus_register_info_t &register_info);
    ModbusMaster *getModbusMaster();
    
private:
    String toBinary(uint16_t input);
    bool decodeDiematicDecimal(uint16_t int_input, int8_t decimals, float *value_ptr);
    bool getModbusResultMsg(uint8_t result);
    bool getModbusValue(uint16_t register_id, modbus_entity_t modbus_entity, uint16_t *value_ptr, uint16_t readBytes = 1);

    static void preTransmission();
    static void postTransmission();

    ModbusMaster _mb;
};

#endif
