#ifndef MODBUS_DEYE_H
#define MODBUS_DEYE_H

#include <modbus/device/modbus_device.h>

class Deye : public ModbusDevice
{
public:
    Deye() : ModbusDevice(_baudRate, _modbusAddr, _protocol) {}

    virtual const modbus_register_t *getLiveRegisters() const override;
    virtual const modbus_register_t *getStaticRegisters() const override;
    const char *getName() const override;
    bool retrieveModel(MODBUS_COM &mCom, char *modelBuffer, size_t bufferSize) override;
    size_t getLiveRegistersCount() const override;
    size_t getStaticRegistersCount() const override;

private:
     static const long _baudRate = 9600;
     static const uint32_t _modbusAddr = 1;
     static const protocol_type_t _protocol = MODBUS_DEYE;
    inline static const char *const _name = "DEYE";

inline static const modbus_register_t registers_live[] = {
    {59, MODBUS_TYPE_HOLDING, REGISTER_TYPE_CUSTOM_VAL_NAME, DESCR_Inverter_Operation_Mode, 0, {.bitfield = {
                                                                                                        "Standby",
                                                                                                        "Self Test",
                                                                                                        "Normal",
                                                                                                        "Alerts",
                                                                                                        "Fault",
                                                                                                    }}},

    {109, MODBUS_TYPE_HOLDING, REGISTER_TYPE_DIEMATIC_ONE_DECIMAL, DESCR_PV_Input_Voltage},
    {110, MODBUS_TYPE_HOLDING, REGISTER_TYPE_DIEMATIC_ONE_DECIMAL, DESCR_PV_Input_Current},
    {186, MODBUS_TYPE_HOLDING, REGISTER_TYPE_U16, DESCR_PV_Input_Power},

    {175, MODBUS_TYPE_HOLDING, REGISTER_TYPE_INT16, DESCR_Output_power},
    {178, MODBUS_TYPE_HOLDING, REGISTER_TYPE_INT16, DESCR_AC_output_power},

    {90, MODBUS_TYPE_HOLDING, REGISTER_TYPE_DIEMATIC_ONE_DECIMAL, DESCR_Inverter_Temperature, -100},
    {182, MODBUS_TYPE_HOLDING, REGISTER_TYPE_DIEMATIC_ONE_DECIMAL, DESCR_Battery_Temperature, -100},

    {183, MODBUS_TYPE_HOLDING, REGISTER_TYPE_DIEMATIC_TWO_DECIMAL, DESCR_Battery_Voltage},
    {184, MODBUS_TYPE_HOLDING, REGISTER_TYPE_U16, DESCR_Battery_Percent},
};

inline static const modbus_register_t registers_static[] = {
    {16, MODBUS_TYPE_HOLDING, REGISTER_TYPE_U32_ONE_DECIMAL, DESCR_AC_Out_Rating_Active_Power},
};


    inline static const modbus_register_t registers_device_serial[] = {
        {3, MODBUS_TYPE_HOLDING, REGISTER_TYPE_ASCII, "SN1"},
        {4, MODBUS_TYPE_HOLDING, REGISTER_TYPE_ASCII, "SN2"},
        {5, MODBUS_TYPE_HOLDING, REGISTER_TYPE_ASCII, "SN3"},
        {6, MODBUS_TYPE_HOLDING, REGISTER_TYPE_ASCII, "SN4"},
        {7, MODBUS_TYPE_HOLDING, REGISTER_TYPE_ASCII, "SN5"}};
};

#endif
