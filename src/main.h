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

static const char ICON_current_ac[] PROGMEM = "current-ac";
static const char ICON_current[] PROGMEM = "current";
static const char ICON_voltage[] PROGMEM = "voltage";
static const char ICON_power[] PROGMEM = "power";
static const char ICON_energy[] PROGMEM = "energy";
static const char ICON_frequency[] PROGMEM = "frequency";
static const char ICON_A[] PROGMEM = "A";
static const char ICON_W[] PROGMEM = "W";
static const char ICON_V[] PROGMEM = "V";
static const char ICON_Hz[] PROGMEM = "Hz";
static const char ICON_Wh[] PROGMEM = "Wh";
static const char ICON_flash_triangle_outline[] PROGMEM = "flash-triangle-outline";
static const char ICON_sine_wave[] PROGMEM = "sine-wave";
static const char ICON_car_battery[] PROGMEM = "car-battery";
static const char ICON_thermometer_lines[] PROGMEM = "thermometer-lines";
static const char ICON_import[] PROGMEM = "import";
static const char ICON_export[] PROGMEM = "export";
static const char ICON_battery_charging[] PROGMEM = "battery-charging";
static const char ICON_tune_variant[] PROGMEM = "tune-variant";
static const char ICON_solar_power_variant[] PROGMEM = "solar-power-variant";
static const char ICON_battery_charging_high[] PROGMEM = "battery-charging-high";
static const char ICON_battery_charging_outline[] PROGMEM = "battery-charging-outline";
static const char ICON_battery_remove_outline[] PROGMEM = "battery-remove-outline";
static const char ICON_ev_station[] PROGMEM = "ev-station";
static const char ICON_state_machine[] PROGMEM = "state-machine";
static const char ICON_clock_time_eight_outline[] PROGMEM = "clock-time-eight-outline";
static const char ICON_battery_outline[] PROGMEM = "battery-outline";
static const char ICON_string_lights[] PROGMEM = "string-lights";
static const char ICON_access_point[] PROGMEM = "access-point";
static const char ICON_protocol[] PROGMEM = "protocol";
static const char ICON_solar_panel[] PROGMEM = "solar-panel";
static const char ICON_priority_high[] PROGMEM = "priority-high";
static const char ICON_earth[] PROGMEM = "earth";
static const char ICON_alert_outline[] PROGMEM = "alert-outline";
static const char ICON_battery_high[] PROGMEM = "battery-high";
static const char ICON_fan[] PROGMEM = "fan";
static const char ICON_car_turbocharger[] PROGMEM = "car-turbocharger";
static const char ICON_card_account_details_outline[] PROGMEM = "card-account-details-outline";
static const char ICON_battery_minus_outline[] PROGMEM = "battery-minus-outline";
static const char ICON_chip[] PROGMEM = "chip";
static const char ICON_flag[] PROGMEM = "flag";
static const char UNIT_percent[] PROGMEM = "%";
static const char UNIT_VA[] PROGMEM = "VA";
static const char UNIT_s[] PROGMEM = "s";
static const char UNIT_degC[] PROGMEM = "°C";
static const char CLASS_power_factor[] PROGMEM = "power_factor";
static const char CLASS_apparent_power[] PROGMEM = "apparent_power";
static const char CLASS_battery[] PROGMEM = "battery";
static const char CLASS_temperature[] PROGMEM = "temperature";
static const char CLASS_duration[] PROGMEM = "duration";
static const char KEY_Battery_capacity[] PROGMEM = "Battery_capacity";
static const char KEY_Grid_frequency[] PROGMEM = "Grid_frequency";
static const char KEY_Grid_voltage[] PROGMEM = "Grid_voltage";
// static const char* DESCR_ = "";

static const char *const haStaticDescriptor[][4] PROGMEM{
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
    {DESCR_Battery_Recharge_Voltage, ICON_battery_charging_high, ICON_V, ICON_voltage},
    {DESCR_Battery_Redischarge_Voltage, ICON_battery_charging_outline, ICON_V, ICON_voltage},
    {DESCR_Battery_Type, ICON_car_battery, "", ""},
    {DESCR_Battery_Under_Voltage, ICON_battery_remove_outline, ICON_V, ICON_voltage},
    {DESCR_Charger_Source_Priority, ICON_ev_station, "", ""},
    {DESCR_Current_Max_AC_Charging_Current, ICON_current_ac, ICON_A, ICON_current},
    {DESCR_Current_Max_Charging_Current, ICON_battery_charging, ICON_A, ICON_current},
    {DESCR_Device_Model, ICON_battery_charging, "", ""},
    {DESCR_Input_Voltage_Range, ICON_flash_triangle_outline, "", ""},
    {DESCR_Machine_Type, ICON_state_machine, "", ""},
    {DESCR_Max_Charging_Time_At_CV_Stage, ICON_clock_time_eight_outline, UNIT_s, CLASS_duration},
    {DESCR_Max_Discharging_Current, ICON_battery_outline, ICON_A, ICON_current},
    {DESCR_MPPT_String, ICON_string_lights, "", ""},
    {DESCR_Operation_Logic, ICON_access_point, "", ""},
    {DESCR_Output_Mode, ICON_export, "", ""},
    {DESCR_Output_Source_Priority, ICON_export, "", ""},
    {DESCR_Parallel_Max_Num, "", "", ""},
    {DESCR_Protocol_ID, ICON_protocol, "", ""},
    {DESCR_PV_OK_Condition_For_Parallel, ICON_solar_panel, "", ""},
    {DESCR_PV_Power_Balance, ICON_solar_panel, "", ""},
    {DESCR_Solar_Power_Priority, ICON_priority_high, "", ""},
    {DESCR_Topology, ICON_earth, "", ""},
    {DESCR_Buzzer_Enabled, ICON_tune_variant, "", ""},
    {DESCR_Overload_Bypass_Enabled, ICON_tune_variant, "", ""},
    {DESCR_Power_Saving_Enabled, ICON_tune_variant, "", ""},
    {DESCR_LCD_Reset_To_Default_Enabled, ICON_tune_variant, "", ""},
    {DESCR_Overload_Restart_Enabled, ICON_tune_variant, "", ""},
    {DESCR_Over_Temperature_Restart_Enabled, ICON_tune_variant, "", ""},
    {DESCR_LCD_Backlight_Enabled, ICON_tune_variant, "", ""},
    {DESCR_Primary_Source_Interrupt_Alarm_Enabled, ICON_tune_variant, "", ""},
    {DESCR_Record_Fault_Code_Enabled, ICON_tune_variant, "", ""}};
static const char *const haLiveDescriptor[][4] PROGMEM{
    // state_topic, icon, unit_ofmeasurement, class
    {DESCR_AC_In_Frequenz, ICON_import, ICON_Hz, ICON_frequency},
    {DESCR_AC_In_Generation_Day, ICON_import, ICON_Wh, ICON_energy},
    {DESCR_AC_In_Generation_Month, ICON_import, ICON_Wh, ICON_energy},
    {DESCR_AC_In_Generation_Sum, ICON_import, ICON_Wh, ICON_energy},
    {DESCR_AC_In_Generation_Year, ICON_import, ICON_Wh, ICON_energy},
    {DESCR_AC_In_Voltage, ICON_import, ICON_V, ICON_voltage},
    {DESCR_AC_Out_Frequenz, ICON_export, ICON_Hz, ICON_frequency},
    {DESCR_AC_Out_Percent, ICON_export, UNIT_percent, CLASS_power_factor},
    {DESCR_AC_Out_VA, ICON_export, UNIT_VA, CLASS_apparent_power},
    {DESCR_AC_Out_Voltage, ICON_export, ICON_V, ICON_voltage},
    {DESCR_AC_Out_Watt, ICON_export, ICON_W, ICON_power},

    {DESCR_AC_output_current, ICON_export, ICON_A, ICON_current},
    {DESCR_AC_output_frequency, ICON_export, ICON_Hz, ICON_frequency},
    {DESCR_AC_output_power, ICON_export, ICON_W, ICON_power},
    {DESCR_AC_output_voltage, ICON_export, ICON_V, ICON_voltage},
    //{"ACDC_Power_Direction","sign-direction","",""},
    {KEY_Battery_capacity, ICON_battery_high, UNIT_percent, CLASS_battery},
    {DESCR_Battery_Charge_Current, ICON_battery_charging_high, ICON_A, ICON_current},
    {DESCR_Battery_Discharge_Current, ICON_battery_charging_outline, ICON_A, ICON_current},
    {DESCR_Battery_Load, ICON_battery_charging_high, ICON_A, ICON_current},
    {DESCR_Battery_Percent, ICON_battery_charging_high, UNIT_percent, CLASS_battery},
    {DESCR_Battery_Power_Direction, ICON_battery_charging_high, "", ""},
    {DESCR_Battery_SCC_Volt, ICON_battery_high, ICON_V, ICON_voltage},
    //{"Battery_SCC2_Volt","battery-high",DESCR_V,DESCR_voltage},
    {DESCR_Battery_Temperature, ICON_thermometer_lines, UNIT_degC, CLASS_temperature},
    {DESCR_Battery_Voltage, ICON_battery_high, ICON_V, ICON_voltage},
    //{DESCR_Battery_Voltage_Offset_Fans_On, "fan", "", ""}, //disabled for some malefunction and unnescary data
    //{"Configuration_State","state-machine","",""},
    //{"Country","earth","",""},
    {DESCR_Device_Status, ICON_state_machine, "", ""},
    {DESCR_EEPROM_Version, ICON_chip, "", ""},
    {DESCR_Fan_Speed, ICON_fan, UNIT_percent, ""},
    {DESCR_Fault_Code, ICON_alert_outline, "", ""},
    {KEY_Grid_frequency, ICON_import, ICON_Hz, ICON_frequency},
    {KEY_Grid_voltage, ICON_import, ICON_V, ICON_voltage},
    {DESCR_Inverter_Bus_Temperature, ICON_thermometer_lines, UNIT_degC, CLASS_temperature},
    {DESCR_Inverter_Bus_Voltage, ICON_flash_triangle_outline, ICON_V, ICON_voltage},
    {DESCR_Inverter_Charge_State, ICON_car_turbocharger, "", ""},
    {DESCR_Inverter_Operation_Mode, ICON_car_turbocharger, "", ""},
    {DESCR_Inverter_Temperature, ICON_thermometer_lines, UNIT_degC, CLASS_temperature},
    //{"Line_Power_Direction","transmission-tower","",""},
    //{"Load_Connection","connection","",""},
    {DESCR_Local_Parallel_ID, ICON_card_account_details_outline, "", ""},
    //{"Max_temperature","thermometer-plus","C","temperature"},
    //{"MPPT1_Charger_Status","car-turbocharger","",""},
    {DESCR_MPPT1_Charger_Temperature, ICON_thermometer_lines, UNIT_degC, CLASS_temperature},
    //{"MPPT2_CHarger_Status","car-turbocharger","",""},
    {DESCR_MPPT2_Charger_Temperature, ICON_thermometer_lines, UNIT_degC, CLASS_temperature},
    {DESCR_Negative_battery_voltage, ICON_battery_minus_outline, ICON_V, ICON_voltage},
    {DESCR_Output_current, ICON_export, ICON_A, ICON_current},
    {DESCR_Output_load_percent, ICON_export, UNIT_percent, CLASS_battery},
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
    {"PV3_input_power", ICON_solar_power_variant, ICON_W, ICON_power},
    {"PV3_input_voltage", ICON_solar_power_variant, ICON_V, ICON_voltage},
    //{"SBUS_voltage",DESCR_flash_triangle_outline,DESCR_V,DESCR_voltage},
    {DESCR_Solar_Feed_To_Grid_Power, ICON_solar_power_variant, ICON_W, ICON_power},
    {DESCR_Solar_Feed_To_Grid_Status, ICON_solar_power_variant, "", ""},
    {DESCR_Status_Flag, ICON_flag, "", ""},
    {DESCR_Time_Until_Absorb_Charge, ICON_solar_power_variant, UNIT_s, CLASS_duration},
    {DESCR_Time_Until_Float_Charge, ICON_solar_power_variant, UNIT_s, CLASS_duration},
    {DESCR_Tracker_Temperature, ICON_thermometer_lines, UNIT_degC, CLASS_temperature},
    {DESCR_Transformer_Temperature, ICON_thermometer_lines, UNIT_degC, CLASS_temperature},
    {DESCR_Warning_Code, ICON_alert_outline, "", ""}};

#endif

