#ifndef DESCRIPTORS_H
#define DESCRIPTORS_H
// pi_Serial flags
#define DESCR_req_NAK "NAK"
#define DESCR_req_NOA "NOA"
#define DESCR_req_ERCRC "ERCRC"
// QPIGS
#define DESCR_AC_In_Voltage "AC_In_Voltage"                                   // BBB.B

#define DESCR_AC_In_Frequenz "AC_In_Frequenz"                                 // CC.C
#define DESCR_AC_Out_Voltage "AC_Out_Voltage"                                 // DDD.D
#define DESCR_AC_Out_Frequenz "AC_Out_Frequenz"                               // EE.E
#define DESCR_AC_Out_VA "AC_Out_VA"                                           // FFFF
#define DESCR_AC_Out_Watt "AC_Out_Watt"                                       // GGGG
#define DESCR_AC_Out_Percent "AC_Out_Percent"                                 // HHH
#define DESCR_AC_In_Voltage_L2 "AC_In_Voltage_L2"
#define DESCR_AC_In_Frequenz_L2 "AC_In_Frequenz_L2"
#define DESCR_AC_Out_Voltage_L2 "AC_Out_Voltage_L2"
#define DESCR_AC_Out_Frequenz_L2 "AC_Out_Frequenz_L2"
#define DESCR_AC_Out_VA_L2 "AC_Out_VA_L2"
#define DESCR_AC_Out_Watt_L2 "AC_Out_Watt_L2"
#define DESCR_AC_Out_Percent_L2 "AC_Out_Percent_L2"
#define DESCR_Inverter_Bus_Voltage "Inverter_Bus_Voltage"                     // III
#define DESCR_Battery_Voltage "Battery_Voltage"                               // JJ.JJ
#define DESCR_Battery_Voltage_L2 "Battery_Voltage_L2"
#define DESCR_Battery_Charge_Current "Battery_Charge_Current"                 // KKK
#define DESCR_Battery_Percent "Battery_Percent"                               // OOO
#define DESCR_Inverter_Bus_Temperature "Inverter_Bus_Temperature"             // TTTT
#define DESCR_PV_Input_Current "PV_Input_Current"                             // EE.E
#define DESCR_PV_Input_Voltage "PV_Input_Voltage"                             // UUU.U
#define DESCR_Battery_SCC_Volt "Battery_SCC_Volt"                             // WW.WW
#define DESCR_Battery_Discharge_Current "Battery_Discharge_Current"           // PPPP
#define DESCR_Status_Flag "Status_Flag"                                       // b0-b7
#define DESCR_Battery_Voltage_Offset_Fans_On "Battery_Voltage_Offset_Fans_On" // QQ
#define DESCR_EEPROM_Version "EEPROM_Version"                                 // VV
#define DESCR_PV_Charging_Power "PV_Charging_Power"                           // MMMM
#define DESCR_Device_Status "Device_Status"                                   // b8-b10
#define DESCR_Device_Status_L2 "Device_Status_L2"
#define DESCR_Solar_Feed_To_Grid_Status "Solar_Feed_To_Grid_Status"           // Y
#define DESCR_Country "Country"                                               // ZZ
#define DESCR_Solar_Feed_To_Grid_Power "Solar_Feed_To_Grid_Power"             // AAAA
#define DESCR_Battery_SCC2_Volt "Battery_SCC2_Volt"
#define DESCR_MPPT1_Charger_Temperature "MPPT1_Charger_Temperature"
#define DESCR_MPPT2_Charger_Temperature "MPPT2_Charger_Temperature"

#define DESCR_Negative_battery_voltage "Negative_battery_voltage"
#define DESCR_Output_current "Output_current"
#define DESCR_Output_load_percent "Output_load_percent"
#define DESCR_Output_power "Output_power"

#define DESCR_AC_output_current "AC_output_current"
#define DESCR_AC_output_frequency "AC_output_frequency"
#define DESCR_AC_output_power "AC_output_power"
#define DESCR_AC_output_voltage "AC_output_voltage"

#define DESCR_Positive_battery_voltage "Positive_battery_voltage"

#define DESCR_PV1_Input_Power "PV1_Input_Power"
#define DESCR_PV2_Input_Power "PV2_Input_Power"
#define DESCR_PV1_Input_Voltage "PV1_Input_Voltage"
#define DESCR_PV2_Input_Voltage "PV2_Input_Voltage"
#define DESCR_Configuration_State "Configuration_State"
#define DESCR_MPPT1_Charger_Status "MPPT1_Charger_Status"
#define DESCR_MPPT2_CHarger_Status "MPPT2_CHarger_Status"
#define DESCR_Load_Connection "Load_Connection"
#define DESCR_Battery_Power_Direction "Battery_Power_Direction"
#define DESCR_ACDC_Power_Direction "ACDC_Power_Direction"
#define DESCR_Line_Power_Direction "Line_Power_Direction"
#define DESCR_Local_Parallel_ID "Local_Parallel_ID"
#define DESCR_Battery_Load "Battery_Load"
#define DESCR_PV_Input_Power "PV_Input_Power"
// QPIGS2
#define DESCR_PV2_Input_Current "PV2_Input_Current"
#define DESCR_PV2_Charging_Power "PV2_Charging_Power"

// QALL
#define DESCR_Warning_Code "Warning_Code" // KK
#define DESCR_Fault_Code "Fault_Code"     // SS
// Q1
#define DESCR_Time_Until_Absorb_Charge "Time_Until_Absorb_Charge" // time since absorb start
#define DESCR_Time_Until_Float_Charge "Time_Until_Float_Charge"   // tiem since float char*ge start
#define DESCR_SCC_Flag "SCC_Flag"                                 // SCC Flag
#define DESCR_AllowSccOnFlag "AllowSccOnFlag"                     // AllowSccOnFlag
#define DESCR_Charge_Average_Current "Charge_Average_Current"     // ChargeAverageCurrent
#define DESCR_Tracker_Temperature "Tracker_Temperature"           // Temp sensor 1 SCC PWM temperature
#define DESCR_Inverter_Temperature "Inverter_Temperature"         // temp sensor 2
#define DESCR_Battery_Temperature "Battery_Temperature"           // temp sensor 3
#define DESCR_Transformer_Temperature "Transformer_Temperature"   //
#define DESCR_Fan_Lock_Status "Fan_Lock_Status"                   // error flag for blocked fan
#define DESCR_Fan_Speed "Fan_Speed"                               // Fan PWM speed
#define DESCR_SCC_Charge_Power "SCC_Charge_Power"                 // SCC char*ge power
#define DESCR_Parallel_Warning "Parallel_Warning"                 // Parallel Warning
#define DESCR_Sync_Frequency "Sync_Frequency"                     // Sync frequency
#define DESCR_Inverter_Charge_State "Inverter_Charge_State"       // charge state
#define DESCR_unknown "unknown"                                   // unknown state
// Inverter_Charge_State states
#define DESCR_No_Charging "No charging"
#define DESCR_Bulk_Stage "Bulk stage"
#define DESCR_Absorb "Absorb"
#define DESCR_Float "Float"
// QPI
#define DESCR_Protocol_ID "Protocol_ID"
// QEX
#define DESCR_PV_Generation_Sum "PV_Generation_Sum"
#define DESCR_PV_Generation_Year "PV_Generation_Year"
#define DESCR_PV_Generation_Month "PV_Generation_Month"
#define DESCR_PV_Generation_Day "PV_Generation_Day"
#define DESCR_AC_In_Generation_Sum "AC_In_Generation_Sum"
#define DESCR_AC_In_Generation_Year "AC_In_Generation_Year"
#define DESCR_AC_In_Generation_Month "AC_In_Generation_Month"
#define DESCR_AC_In_Generation_Day "AC_In_Generation_Day"
// QFLAG
#define DESCR_Buzzer_Enabled "Buzzer_Enabled"
#define DESCR_Overload_Bypass_Enabled "Overload_Bypass_Enabled"
#define DESCR_Power_Saving_Enabled "Power_Saving_Enabled"
#define DESCR_LCD_Reset_To_Default_Enabled "LCD_Reset_To_Default_Enabled"
#define DESCR_Overload_Restart_Enabled "Overload_Restart_Enabled"
#define DESCR_Over_Temperature_Restart_Enabled "Over_Temperature_Restart_Enabled"
#define DESCR_LCD_Backlight_Enabled "LCD_Backlight_Enabled"
#define DESCR_Primary_Source_Interrupt_Alarm_Enabled "Primary_Source_Interrupt_Alarm_Enabled"
#define DESCR_Record_Fault_Code_Enabled "Record_Fault_Code_Enabled"
#define DESCR_Data_Log_Pop_Up "Data_Log_Pop_Up"
// QMN
#define DESCR_Device_Model "Device_Model"
// QMOD
#define DESCR_Inverter_Operation_Mode "Inverter_Operation_Mode"
#define DESCR_Power_On "Power On"
#define DESCR_Standby "Standby"
#define DESCR_Bypass "Bypass"
#define DESCR_Battery "Battery"
#define DESCR_Fault "Fault"
#define DESCR_Hybrid "Hybrid"
#define DESCR_No_Data "No Data"

#define DESCR_AC_In_Rating_Voltage "AC_In_Rating_Voltage"                       // BBB.B
#define DESCR_AC_In_Rating_Current "AC_In_Rating_Current"                       // CC.C
#define DESCR_AC_Out_Rating_Voltage "AC_Out_Rating_Voltage"                     // DDD.D
#define DESCR_AC_Out_Rating_Frequency "AC_Out_Rating_Frequency"                 // EE.E
#define DESCR_AC_Out_Rating_Current "AC_Out_Rating_Current"                     // FF.F
#define DESCR_AC_Out_Rating_Apparent_Power "AC_Out_Rating_Apparent_Power"       // HHHH
#define DESCR_AC_Out_Rating_Active_Power "AC_Out_Rating_Active_Power"           // IIII
#define DESCR_Battery_Rating_Voltage "Battery_Rating_Voltage"                   // JJ.J
#define DESCR_Battery_Recharge_Voltage "Battery_Recharge_Voltage"               // KK.K
#define DESCR_Battery_Under_Voltage "Battery_Under_Voltage"                     // JJ.J
#define DESCR_Battery_Bulk_Voltage "Battery_Bulk_Voltage"                       // KK.K
#define DESCR_Battery_Float_Voltage "Battery_Float_Voltage"                     // LL.L
#define DESCR_Battery_Type "Battery_Type"                                       // O
#define DESCR_Current_Max_AC_Charging_Current "Current_Max_AC_Charging_Current" // PP
#define DESCR_Current_Max_Charging_Current "Current_Max_Charging_Current"       // QQ0
#define DESCR_Input_Voltage_Range "Input_Voltage_Range"                         // O
#define DESCR_Output_Source_Priority "Output_Source_Priority"                   // P
#define DESCR_Charger_Source_Priority "Charger_Source_Priority"                 // Q
#define DESCR_Parallel_Max_Num "Parallel_Max_Num"                               // R
#define DESCR_Machine_Type "Machine_Type"                                       // SS
#define DESCR_Topology "Topology"                                               // T
#define DESCR_Output_Mode "Output_mode"                                         // U
#define DESCR_Battery_Redischarge_Voltage "Battery_Redischarge_Voltage"         // VV.V
#define DESCR_PV_OK_Condition_For_Parallel "PV_OK_Condition_For_Parallel"       // W
#define DESCR_PV_Power_Balance "PV_Power_Balance"                               // X
#define DESCR_Max_Charging_Time_At_CV_Stage "Max_Charging_Time_At_CV_Stage"     // YYY
#define DESCR_Operation_Logic "Operation_Logic"                                 // Z
#define DESCR_Max_Discharging_Current "Max_Discharging_Current"                 // CCC
#define DESCR_Solar_Power_Priority "Solar_Power_Priority"                       // Z
#define DESCR_MPPT_String "MPPT_String"                                         // a
#endif
