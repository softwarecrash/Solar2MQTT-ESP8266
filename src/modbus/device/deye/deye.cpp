#include "deye.h"


const modbus_register_t *Deye::getLiveRegisters() const
{
    return registers_live;
}

const modbus_register_t *Deye::getStaticRegisters() const
{
    return registers_static;
}

const char *Deye::getName() const
{
    return _name;
}

bool Deye::retrieveModel(MODBUS_COM &mCom, char *modelBuffer, size_t bufferSize)
{
    modelBuffer[0] = '\0'; // Clear the buffer
    StaticJsonDocument<256> doc;
    JsonObject jsonObj = doc.to<JsonObject>(); // Create and get JsonObject
    modbus_register_info_t model_info = {
        .variant = &jsonObj,
        .registers = registers_device_serial,
        .array_size = sizeof(registers_device_serial) / sizeof(modbus_register_t),
        .curr_register = 0};

    for (size_t i = 0; i < model_info.array_size; i++)
    {
        mCom.parseModbusToJson(model_info, false);
        if (mCom.isAllRegistersRead(model_info))
        {
            const char *sn1 = doc["SN1"];
            const char *sn2 = doc["SN2"];
            const char *sn3 = doc["SN3"];
            const char *sn4 = doc["SN4"];
            const char *sn5 = doc["SN5"];
            snprintf(modelBuffer, bufferSize, "%s%s%s%s%s", sn1, sn2, sn3, sn4, sn5);
            return true;
        }
        delay(50);
    }
    return false;
}

// Define the size calculation after the arrays are defined
size_t Deye::getLiveRegistersCount() const
{
    return sizeof(registers_live) / sizeof(modbus_register_t);
}

size_t Deye::getStaticRegistersCount() const
{
    return sizeof(registers_static) / sizeof(modbus_register_t);
}
