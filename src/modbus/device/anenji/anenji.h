#ifndef MODBUS_ANENJI_H
#define MODBUS_ANENJI_H

#include <modbus/device/modbus_device.h>

class Anenji : public ModbusDevice
{
public:
    Anenji() : ModbusDevice(_baudRate, _modbusAddr, _protocol) {}

    virtual const modbus_register_t *getLiveRegisters() const override;
    virtual const modbus_register_t *getStaticRegisters() const override;
    const char *getName() const override;
    bool retrieveModel(MODBUS_COM &mCom, char *modelBuffer, size_t bufferSize) override;
    size_t getLiveRegistersCount() const override;
    size_t getStaticRegistersCount() const override;

private:
     static const long _baudRate = 9600;
     static const uint32_t _modbusAddr = 1;
     static const protocol_type_t _protocol = MODBUS_ANENJI;
    inline static const char *const _name = "Anenji";

    inline static const modbus_register_t registers_live[] = {
        {201, MODBUS_TYPE_HOLDING, REGISTER_TYPE_CUSTOM_VAL_NAME, DESCR_Inverter_Operation_Mode, 0, {.bitfield = {
                                                                                                             "Power On",
                                                                                                             "Standby",
                                                                                                             "Mains",
                                                                                                             "Off-Grid",
                                                                                                             "Bypass",
                                                                                                             "Charging",
                                                                                                             "Fault",
                                                                                                         }}},
        //needs adding
        //https://github.com/softwarecrash/Solar2MQTT/blob/2c86b8f609c82d5e52808cfe2810b10b61730c85/src/modbus/device/anenji/anenji.h
        {223, MODBUS_TYPE_HOLDING, REGISTER_TYPE_INT16, DESCR_PV_Input_Power},
        {219, MODBUS_TYPE_HOLDING, REGISTER_TYPE_DIEMATIC_ONE_DECIMAL, DESCR_PV_Input_Voltage},
        {220, MODBUS_TYPE_HOLDING, REGISTER_TYPE_DIEMATIC_ONE_DECIMAL, DESCR_PV2_Input_Current},
 
        {229, MODBUS_TYPE_HOLDING, REGISTER_TYPE_U16, DESCR_Battery_Percent},
        {215, MODBUS_TYPE_HOLDING, REGISTER_TYPE_DIEMATIC_ONE_DECIMAL, DESCR_Battery_Voltage},
        {227, MODBUS_TYPE_HOLDING, REGISTER_TYPE_INT16, DESCR_Inverter_Temperature},
        {226, MODBUS_TYPE_HOLDING, REGISTER_TYPE_INT16, DESCR_Transformer_Temperature},
        {225, MODBUS_TYPE_HOLDING, REGISTER_TYPE_INT16, DESCR_Output_load_percent}, 
    };

    inline static const modbus_register_t registers_static[] = { 
         {322, MODBUS_TYPE_HOLDING, REGISTER_TYPE_CUSTOM_VAL_NAME, DESCR_Battery_Type, 0, {.bitfield = {
                                                                                            "AGM",
                                                                                            "FLD",
                                                                                            "USER",
                                                                                            "SMK1",
                                                                                            "PYL",
                                                                                            "FOX",
                                                                                        }}},
    };

    inline static const modbus_register_t registers_device_serial[] = {
        {186, MODBUS_TYPE_HOLDING, REGISTER_TYPE_ASCII, "SN1"},
        {188, MODBUS_TYPE_HOLDING, REGISTER_TYPE_ASCII, "SN2"},
        {190, MODBUS_TYPE_HOLDING, REGISTER_TYPE_ASCII, "SN3"},
        {192, MODBUS_TYPE_HOLDING, REGISTER_TYPE_ASCII, "SN4"},
        {194, MODBUS_TYPE_HOLDING, REGISTER_TYPE_ASCII, "SN5"},
        {196, MODBUS_TYPE_HOLDING, REGISTER_TYPE_ASCII, "SN6"}};
};

#endif
