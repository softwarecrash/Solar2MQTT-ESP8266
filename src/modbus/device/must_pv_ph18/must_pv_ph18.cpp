#include "must_pv_ph18.h"

const modbus_register_t *MustPV_PH18::getLiveRegisters() const
{
    return registers_live;
}

const modbus_register_t *MustPV_PH18::getStaticRegisters() const
{
    return registers_static;
}

const char *MustPV_PH18::getName() const
{
    return _name;
}
bool MustPV_PH18::retrieveModel(MODBUS_COM &mCom, char *modelBuffer, size_t bufferSize)
{
    modelBuffer[0] = '\0'; // Clear the buffer
    StaticJsonDocument<256> doc;
    JsonObject jsonObj = doc.to<JsonObject>(); // Create and get JsonObject
    modbus_register_info_t model_info = {
        .variant = &jsonObj,
        .registers = registers_device_model,
        .array_size = sizeof(registers_device_model) / sizeof(modbus_register_t),
        .curr_register = 0};

    for (size_t i = 0; i < model_info.array_size; i++)
    {
        mCom.parseModbusToJson(model_info, false);
        if (mCom.isAllRegistersRead(model_info))
        {
            const char *modelHigh = doc[DEVICE_MODEL_HIGH];
            int modelLow = doc[DEVICE_MODEL_LOW];
            snprintf(modelBuffer, bufferSize, "%s%d", modelHigh, modelLow);
            return true;
        }
        delay(50);
    }

    return false;
}

// Define the size calculation after the arrays are defined
size_t MustPV_PH18::getLiveRegistersCount() const
{
    return sizeof(registers_live) / sizeof(modbus_register_t);
}

size_t MustPV_PH18::getStaticRegistersCount() const
{
    return sizeof(registers_static) / sizeof(modbus_register_t);
}

void MustPV_PH18::generationSum(JsonObject *variant, uint16_t *registerValue, const modbus_register_t *reg, MODBUS_COM &mCom)
{  
    if ((*variant)[MUST_DEVICE_CHARGER_HIGH].is<uint16_t>() && (*variant)[MUST_DEVICE_CHARGER_LOW].is<uint16_t>())
    {
        (*variant)[reg->name] = (*variant)[MUST_DEVICE_CHARGER_HIGH].as<uint16_t>() * 1000 + (*variant)[MUST_DEVICE_CHARGER_LOW].as<uint16_t>();
    }
}
