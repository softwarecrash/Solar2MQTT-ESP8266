#ifndef MAIN_H
#define MAIN_H

//#include <WebSerialLite.h>
#include <MycilaWebSerial.h>
#include "descriptors.h"
#define ARDUINOJSON_USE_DOUBLE 1
#define ARDUINOJSON_USE_LONG_LONG 1
// #define ARDUINOJSON_ENABLE_PROGMEM 1

#ifdef isUART_HARDWARE
#define INVERTER_TX 1
#define INVERTER_RX 3
#define LED_COM 5
#define LED_SRV 0
#define LED_NET 4
#else
#define INVERTER_TX 13
#define INVERTER_RX 12
#define TEMPSENS_PIN 4     // DS18B20 Pin; D2 on Wemos D1 Mini
#define TIME_INTERVAL 1500 // Time interval among sensor readings [milliseconds]
#endif

#define LED_PIN 02 // D4 with the LED on Wemos D1 Mini

#define DBG_BAUD 115200
#define DBG_WEBLN(...) webSerial.println(__VA_ARGS__)
#define DBG_WEB(...) webSerial.print(__VA_ARGS__)
#define SOFTWARE_VERSION SWVERSION
#define DBG Serial
#define DBG_BEGIN(...) DBG.begin(__VA_ARGS__)
#define DBG_PRINTLN(...) DBG.println(__VA_ARGS__)

typedef enum
{
    NoD,
    PI18,
    PI30,
    MODBUS_MUST,
    MODBUS_DEYE,
    MODBUS_ANENJI,
    PROTOCOL_TYPE_MAX // Add a max value to mark the upper enum bound
} protocol_type_t;

// together with new protocol you will also need to define string
constexpr const char *protocolStrings[] = {
    "NoD",
    "PI18",
    "PI30",
    "MODBUS_MUST",
    "MODBUS_DEYE",
    "MODBUS_ANENJI"};

/**
 * @brief callback function for wifimanager save config data
 *
 */
void saveConfigCallback();

/**
 * @brief callback function for data
 *
 */
bool prozessData();

/**
 * @brief fires up the websocket and send data to the clients
 *
 */
void notifyClients();

/**
 * @brief build the topic string and return
 *
 */
char *topicBuilder(char *buffer, char const *path, char const *numering);

/**
 * @brief mqtt connect function, check if connection etablished and reconnect and subscribe to spezific topics if needed
 *
 */
bool connectMQTT();

/**
 * @brief send the data to mqtt
 *
 */
bool sendtoMQTT();

/**
 * @brief send the data async to mqtt
 *
 */
bool sendtoMQTTasync();

/**
 * @brief get the basic device data
 *
 */
// void getJsonDevice();

/**
 * @brief read the data from bms and put it in the json
 */
void getJsonData();

/**
 * @brief callback function, watch the sunscribed topics and process the data
 *
 */
void mqttcallback(char *top, unsigned char *payload, unsigned int length);

bool sendHaDiscovery();
/**
 * @brief function for ext. TempSensors
 */
void handleTemperatureChange(int deviceIndex, int32_t temperatureRAW);

/**
 * @brief this function act like s/n/printf() and give the output to the configured serial and webserial
 *
 */
void writeLog(const char *format, ...);

static const char *ICON_current_ac = "current-ac";
static const char *ICON_current = "current";
static const char *ICON_voltage = "voltage";
static const char *ICON_power = "power";
static const char *ICON_energy = "energy";
static const char *ICON_frequency = "frequency";
static const char *ICON_A = "A";
static const char *ICON_W = "W";
static const char *ICON_V = "V";
static const char *ICON_Hz = "Hz";
static const char *ICON_Wh = "Wh";
static const char *ICON_flash_triangle_outline = "flash-triangle-outline";
static const char *ICON_sine_wave = "sine-wave";
static const char *ICON_car_battery = "car-battery";
static const char *ICON_thermometer_lines = "thermometer-lines";
static const char *ICON_import = "import";
static const char *ICON_export = "export";
static const char *ICON_battery_charging = "battery-charging";
static const char *ICON_tune_variant = "tune-variant";
static const char *ICON_solar_power_variant = "solar-power-variant";
// static const char* DESCR_ = "";

static const char *const haStaticDescriptor[][4]{
    // state_topic, icon, unit_ofmeasurement, class
    {DESCR_AC_In_Rating_Current, ICON_current_ac, ICON_A, ICON_current},
    {DESCR_AC_In_Rating_Voltage, ICON_flash_triangle_outline, ICON_V, ICON_voltage},
    {DESCR_AC_Out_Rating_Active_Power, ICON_sine_wave, ICON_W, ICON_power},
    {DESCR_AC_Out_Rating_Apparent_Power, ICON_sine_wave, ICON_W, ICON_power},
    {DESCR_AC_Out_Rating_Current, ICON_current_ac, ICON_A, ICON_current},
    {DESCR_AC_Out_Rating_Frequency, ICON_sine_wave, ICON_Hz, ICON_frequency},
    {DESCR_AC_Out_Rating_Voltage, ICON_flash_triangle_outline, ICON_V, ICON_voltage},
    {DESCR_Battery_Bulk_Voltage, ICON_car_battery, ICON_V, ICON_voltage},
    {DESCR_Battery_Float_Voltage, ICON_car_battery, ICON_V, ICON_voltage},
    {DESCR_Battery_Rating_Voltage, ICON_car_battery, ICON_V, ICON_voltage},
    {DESCR_Battery_Recharge_Voltage, "battery-charging-high", ICON_V, ICON_voltage},
    {DESCR_Battery_Redischarge_Voltage, "battery-charging-outline", ICON_V, ICON_voltage},
    {DESCR_Battery_Type, ICON_car_battery, "", ""},
    {DESCR_Battery_Under_Voltage, "battery-remove-outline", ICON_V, ICON_voltage},
    {DESCR_Charger_Source_Priority, "ev-station", "", ""},
    {DESCR_Current_Max_AC_Charging_Current, ICON_current_ac, ICON_A, ICON_current},
    {DESCR_Current_Max_Charging_Current, ICON_battery_charging, ICON_A, ICON_current},
    {DESCR_Device_Model, ICON_battery_charging, "", ""},
    {DESCR_Input_Voltage_Range, ICON_flash_triangle_outline, "", ""},
    {DESCR_Machine_Type, "state-machine", "", ""},
    {DESCR_Max_Charging_Time_At_CV_Stage, "clock-time-eight-outli", "s", "duration"},
    {DESCR_Max_Discharging_Current, "battery-outline", ICON_A, ICON_current},
    {DESCR_MPPT_String, "string-lights", "", ""},
    {DESCR_Operation_Logic, "access-point", "", ""},
    {DESCR_Output_Mode, ICON_export, "", ""},
    {DESCR_Output_Source_Priority, ICON_export, "", ""},
    {DESCR_Parallel_Max_Num, "", "", ""},
    {DESCR_Protocol_ID, "protocol", "", ""},
    {DESCR_PV_OK_Condition_For_Parallel, "solar-panel", "", ""},
    {DESCR_PV_Power_Balance, "solar-panel", "", ""},
    {DESCR_Solar_Power_Priority, "priority-high", "", ""},
    {DESCR_Topology, "earth", "", ""},
    {DESCR_Buzzer_Enabled, ICON_tune_variant, "", ""},
    {DESCR_Overload_Bypass_Enabled, ICON_tune_variant, "", ""},
    {DESCR_Power_Saving_Enabled, ICON_tune_variant, "", ""},
    {DESCR_LCD_Reset_To_Default_Enabled, ICON_tune_variant, "", ""},
    {DESCR_Overload_Restart_Enabled, ICON_tune_variant, "", ""},
    {DESCR_Over_Temperature_Restart_Enabled, ICON_tune_variant, "", ""},
    {DESCR_LCD_Backlight_Enabled, ICON_tune_variant, "", ""},
    {DESCR_Primary_Source_Interrupt_Alarm_Enabled, ICON_tune_variant, "", ""},
    {DESCR_Record_Fault_Code_Enabled, ICON_tune_variant, "", ""}};
static const char *const haLiveDescriptor[][4]{
    // state_topic, icon, unit_ofmeasurement, class
    {DESCR_AC_In_Frequenz, ICON_import, ICON_Hz, ICON_frequency},
    {DESCR_AC_In_Generation_Day, ICON_import, ICON_Wh, ICON_energy},
    {DESCR_AC_In_Generation_Month, ICON_import, ICON_Wh, ICON_energy},
    {DESCR_AC_In_Generation_Sum, ICON_import, ICON_Wh, ICON_energy},
    {DESCR_AC_In_Generation_Year, ICON_import, ICON_Wh, ICON_energy},
    {DESCR_AC_In_Voltage, ICON_import, ICON_V, ICON_voltage},
    {DESCR_AC_Out_Frequenz, ICON_export, ICON_Hz, ICON_frequency},
    {DESCR_AC_Out_Percent, ICON_export, "%", "power_factor"},
    {DESCR_AC_Out_VA, ICON_export, "VA", "apparent_power"},
    {DESCR_AC_Out_Voltage, ICON_export, ICON_V, ICON_voltage},
    {DESCR_AC_Out_Watt, ICON_export, ICON_W, ICON_power},

    {DESCR_AC_output_current, ICON_export, ICON_A, ICON_current},
    {DESCR_AC_output_frequency, ICON_export, ICON_Hz, ICON_frequency},
    {DESCR_AC_output_power, ICON_export, ICON_W, ICON_power},
    {DESCR_AC_output_voltage, ICON_export, ICON_V, ICON_voltage},
    //{"ACDC_Power_Direction","sign-direction","",""},
    {"Battery_capacity", "battery-high", "%", "battery"},
    {DESCR_Battery_Charge_Current, "battery-charging-high", ICON_A, ICON_current},
    {DESCR_Battery_Discharge_Current, "battery-charging-outli", ICON_A, ICON_current},
    {DESCR_Battery_Load, "battery-charging-high", ICON_A, ICON_current},
    {DESCR_Battery_Percent, "battery-charging-high", "%", "battery"},
    {DESCR_Battery_Power_Direction, "battery-charging-high", "", ""},
    {DESCR_Battery_SCC_Volt, "battery-high", ICON_V, ICON_voltage},
    //{"Battery_SCC2_Volt","battery-high",DESCR_V,DESCR_voltage},
    {DESCR_Battery_Temperature, ICON_thermometer_lines, "°C", "temperature"},
    {DESCR_Battery_Voltage, "battery-high", ICON_V, ICON_voltage},
    //{DESCR_Battery_Voltage_Offset_Fans_On, "fan", "", ""}, //disabled for some malefunction and unnescary data
    //{"Configuration_State","state-machine","",""},
    //{"Country","earth","",""},
    {DESCR_Device_Status, "state-machine", "", ""},
    {DESCR_EEPROM_Version, "chip", "", ""},
    {DESCR_Fan_Speed, "fan", "%", ""},
    {DESCR_Fault_Code, "alert-outline", "", ""},
    {"Grid_frequency", ICON_import, ICON_Hz, ICON_frequency},
    {"Grid_voltage", ICON_import, ICON_V, ICON_voltage},
    {DESCR_Inverter_Bus_Temperature, ICON_thermometer_lines, "°C", "temperature"},
    {DESCR_Inverter_Bus_Voltage, ICON_flash_triangle_outline, ICON_V, ICON_voltage},
    {DESCR_Inverter_Charge_State, "car-turbocharger", "", ""},
    {DESCR_Inverter_Operation_Mode, "car-turbocharger", "", ""},
    {DESCR_Inverter_Temperature, ICON_thermometer_lines, "°C", "temperature"},
    //{"Line_Power_Direction","transmission-tower","",""},
    //{"Load_Connection","connection","",""},
    {DESCR_Local_Parallel_ID, "card-account-details-outline", "", ""},
    //{"Max_temperature","thermometer-plus","C","temperature"},
    //{"MPPT1_Charger_Status","car-turbocharger","",""},
    {DESCR_MPPT1_Charger_Temperature, ICON_thermometer_lines, "°C", "temperature"},
    //{"MPPT2_CHarger_Status","car-turbocharger","",""},
    {DESCR_MPPT2_Charger_Temperature, ICON_thermometer_lines, "°C", "temperature"},
    {DESCR_Negative_battery_voltage, "battery-minus-outline", ICON_V, ICON_voltage},
    {DESCR_Output_current, ICON_export, ICON_A, ICON_current},
    {DESCR_Output_load_percent, ICON_export, "%", "battery"},
    {DESCR_Output_power, ICON_export, ICON_W, ICON_power},
    //{"PBUS_voltage","",DESCR_V,DESCR_voltage},
    {DESCR_Positive_battery_voltage, ICON_car_battery, ICON_V, ICON_voltage},
    {DESCR_PV_Charging_Power, ICON_solar_power_variant, ICON_W, ICON_power},
    {DESCR_PV_Generation_Day, ICON_solar_power_variant, ICON_Wh, ICON_energy},
    {DESCR_PV_Generation_Month, ICON_solar_power_variant, ICON_Wh, ICON_energy},
    {DESCR_PV_Generation_Sum, ICON_solar_power_variant, ICON_Wh, ICON_energy},
    {DESCR_PV_Generation_Year, ICON_solar_power_variant, ICON_Wh, ICON_energy},
    {DESCR_PV_Input_Current, ICON_solar_power_variant, ICON_A, ICON_current},
    {DESCR_PV_Input_Power, ICON_solar_power_variant, ICON_W, ICON_power},
    {DESCR_PV_Input_Voltage, ICON_solar_power_variant, ICON_V, ICON_voltage},
    {DESCR_PV1_Input_Power, ICON_solar_power_variant, ICON_W, ICON_power},
    {DESCR_PV1_Input_Voltage, ICON_solar_power_variant, ICON_V, ICON_voltage},
    {DESCR_PV2_Charging_Power, ICON_solar_power_variant, ICON_W, ICON_power},
    {DESCR_PV2_Input_Current, ICON_solar_power_variant, ICON_A, ICON_current},
    {DESCR_PV2_Input_Power, ICON_solar_power_variant, ICON_W, ICON_power},
    {DESCR_PV2_Input_Voltage, ICON_solar_power_variant, ICON_V, ICON_voltage},
    {"PV3_Input_Power", ICON_solar_power_variant, ICON_W, ICON_power},
    {"PV3_Input_Voltage", ICON_solar_power_variant, ICON_V, ICON_voltage},
    //{"SBUS_voltage",DESCR_flash_triangle_outline,DESCR_V,DESCR_voltage},
    {DESCR_Solar_Feed_To_Grid_Power, ICON_solar_power_variant, ICON_W, ICON_power},
    {DESCR_Solar_Feed_To_Grid_Status, ICON_solar_power_variant, "", ""},
    {DESCR_Status_Flag, "flag", "", ""},
    {DESCR_Time_Until_Absorb_Charge, ICON_solar_power_variant, "s", "duration"},
    {DESCR_Time_Until_Float_Charge, ICON_solar_power_variant, "s", "duration"},
    {DESCR_Tracker_Temperature, ICON_thermometer_lines, "°C", "temperature"},
    {DESCR_Transformer_Temperature, ICON_thermometer_lines, "°C", "temperature"},
    {DESCR_Warning_Code, "alert-outline", "", ""}};

#endif

