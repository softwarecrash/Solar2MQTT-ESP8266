#ifndef MODBUS_MUST_PV_PH18_H
#define MODBUS_MUST_PV_PH18_H

#include <modbus/device/modbus_device.h>

#define DEVICE_MODEL_HIGH "must_device_model_h"
#define DEVICE_MODEL_LOW "must_device_model_l"

#define MUST_DEVICE_CHARGER_HIGH "ch_h"
#define MUST_DEVICE_CHARGER_LOW "ch_l"

class MustPV_PH18 : public ModbusDevice
{
public:
    MustPV_PH18() : ModbusDevice(_baudRate, _modbusAddr, _protocol) {}

    virtual const modbus_register_t *getLiveRegisters() const override;
    virtual const modbus_register_t *getStaticRegisters() const override;
    const char *getName() const override;
    bool retrieveModel(MODBUS_COM &mCom, char *modelBuffer, size_t bufferSize) override;
    size_t getLiveRegistersCount() const override;
    size_t getStaticRegistersCount() const override;
    static void generationSum(JsonObject *variant, uint16_t *registerValue, const modbus_register_t *reg, MODBUS_COM &mCom);

private:
    static const long _baudRate = 19200;
    static const uint32_t _modbusAddr = 4;
    static const protocol_type_t _protocol = MODBUS_MUST;
    inline static const char *const _name = "MUST PV/PH 18";


inline static const modbus_register_t registers_live[] = {
    {0, MODBUS_TYPE_HOLDING, REGISTER_TYPE_VIRTUAL_CALLBACK, DESCR_PV_Generation_Sum, 0, {}, MustPV_PH18::generationSum},
    {15201, MODBUS_TYPE_HOLDING, REGISTER_TYPE_CUSTOM_VAL_NAME, DESCR_Inverter_Operation_Mode, 0, {.bitfield = {
                                                                                                        "Power On",
                                                                                                        "Self Test",
                                                                                                        "OffGrid",
                                                                                                        "GridTie",
                                                                                                        "ByPass",
                                                                                                        "Stop",
                                                                                                        "GridCharging",
                                                                                                    }}},
    {15205, MODBUS_TYPE_HOLDING, REGISTER_TYPE_DIEMATIC_ONE_DECIMAL, DESCR_PV_Input_Voltage},
    {15207, MODBUS_TYPE_HOLDING, REGISTER_TYPE_DIEMATIC_ONE_DECIMAL, DESCR_PV_Input_Current},
    {15208, MODBUS_TYPE_HOLDING, REGISTER_TYPE_U16, DESCR_PV_Charging_Power},
    {15209, MODBUS_TYPE_HOLDING, REGISTER_TYPE_U16, DESCR_MPPT1_Charger_Temperature},
    {15212, MODBUS_TYPE_HOLDING, REGISTER_TYPE_U16, "PV_Relay"},
    {15217, MODBUS_TYPE_HOLDING, REGISTER_TYPE_U16, MUST_DEVICE_CHARGER_HIGH},
    {15218, MODBUS_TYPE_HOLDING, REGISTER_TYPE_U16, MUST_DEVICE_CHARGER_LOW},
    {15219, MODBUS_TYPE_HOLDING, REGISTER_TYPE_U16, DESCR_PV_Generation_Day},

    {25201, MODBUS_TYPE_HOLDING, REGISTER_TYPE_CUSTOM_VAL_NAME, "PV_Charger_Workstate", 0, {.bitfield = {"Initialization", "Self Test", "Work", "Stop"}}},
    {15202, MODBUS_TYPE_HOLDING, REGISTER_TYPE_CUSTOM_VAL_NAME, "PV_Charger_MPPT_State", 0, {.bitfield = {
                                                                                                         "Stop",
                                                                                                         "MPPT",
                                                                                                         "Current Limit",
                                                                                                     }}},
    {15203, MODBUS_TYPE_HOLDING, REGISTER_TYPE_CUSTOM_VAL_NAME, "PV_Charger_Charge_State", 0, {.bitfield = {
                                                                                                           "Stop",
                                                                                                           "Absorb",
                                                                                                           "Float",
                                                                                                           "EQ",
                                                                                                       }}},
    {25205, MODBUS_TYPE_HOLDING, REGISTER_TYPE_DIEMATIC_ONE_DECIMAL, DESCR_Battery_Voltage},
    {25206, MODBUS_TYPE_HOLDING, REGISTER_TYPE_DIEMATIC_ONE_DECIMAL, DESCR_AC_Out_Voltage},
    {25207, MODBUS_TYPE_HOLDING, REGISTER_TYPE_DIEMATIC_ONE_DECIMAL, DESCR_AC_In_Voltage},
    {25208, MODBUS_TYPE_HOLDING, REGISTER_TYPE_DIEMATIC_ONE_DECIMAL, DESCR_Inverter_Bus_Voltage},
    {25210, MODBUS_TYPE_HOLDING, REGISTER_TYPE_DIEMATIC_ONE_DECIMAL, DESCR_Output_current},
    {25211, MODBUS_TYPE_HOLDING, REGISTER_TYPE_DIEMATIC_ONE_DECIMAL, DESCR_AC_output_current},
    {25212, MODBUS_TYPE_HOLDING, REGISTER_TYPE_DIEMATIC_ONE_DECIMAL, "Inverter_Load_Current"},
    {25213, MODBUS_TYPE_HOLDING, REGISTER_TYPE_U16, DESCR_Output_power},
    {25214, MODBUS_TYPE_HOLDING, REGISTER_TYPE_INT16, "AC_In_Power"},
    {25215, MODBUS_TYPE_HOLDING, REGISTER_TYPE_U16, DESCR_AC_Out_Watt},
    {25216, MODBUS_TYPE_HOLDING, REGISTER_TYPE_U16, DESCR_AC_Out_Percent},
    {25225, MODBUS_TYPE_HOLDING, REGISTER_TYPE_DIEMATIC_TWO_DECIMAL, DESCR_AC_Out_Frequenz},
    {25226, MODBUS_TYPE_HOLDING, REGISTER_TYPE_DIEMATIC_TWO_DECIMAL, DESCR_AC_In_Frequenz},
    {25233, MODBUS_TYPE_HOLDING, REGISTER_TYPE_U16, DESCR_Inverter_Bus_Temperature},
    {25234, MODBUS_TYPE_HOLDING, REGISTER_TYPE_U16, DESCR_Transformer_Temperature},
    {25274, MODBUS_TYPE_HOLDING, REGISTER_TYPE_INT16, DESCR_Battery_Load},
};

 
    inline static const modbus_register_t registers_static[] = {
        {10110, MODBUS_TYPE_HOLDING, REGISTER_TYPE_CUSTOM_VAL_NAME, DESCR_Battery_Type, 0, {.bitfield = {
                                                                                            "No choose",
                                                                                            "User defined",
                                                                                            "Lithium",
                                                                                            "Sealed Lead",
                                                                                            "AGM",
                                                                                            "GEL",
                                                                                            "Flooded",
                                                                                        }}},
        {10103, MODBUS_TYPE_HOLDING, REGISTER_TYPE_DIEMATIC_ONE_DECIMAL, DESCR_Battery_Float_Voltage},
    };

    inline static const modbus_register_t registers_device_model[] = {
        {20000, MODBUS_TYPE_HOLDING, REGISTER_TYPE_ASCII, DEVICE_MODEL_HIGH},
        {20001, MODBUS_TYPE_HOLDING, REGISTER_TYPE_U16, DEVICE_MODEL_LOW, 0}};
};

#endif
