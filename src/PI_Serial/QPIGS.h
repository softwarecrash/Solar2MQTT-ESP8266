unsigned int qpigs_106_length = 24;
const char *const qpigs_106[] = {
    // 106 long have 24 fields
    // [PI34 / MPPT-3000], [PI30 HS MS MSX], [PI30 Revo], [PI30 PIP], [PI41 / LV5048]
    DESCR_AC_In_Voltage,                  // BBB.B
    DESCR_AC_In_Frequenz,                 // CC.C
    DESCR_AC_Out_Voltage,                 // DDD.D
    DESCR_AC_Out_Frequenz,                // EE.E
    DESCR_AC_Out_VA,                      // FFFF
    DESCR_AC_Out_Watt,                    // GGGG
    DESCR_AC_Out_Percent,                 // HHH
    DESCR_Inverter_Bus_Voltage,           // III
    DESCR_Battery_Voltage,                // JJ.JJ
    DESCR_Battery_Charge_Current,         // KKK
    DESCR_Battery_Percent,                // OOO
    DESCR_Inverter_Bus_Temperature,       // TTTT
    DESCR_PV_Input_Current,               // EE.E
    DESCR_PV_Input_Voltage,               // UUU.U
    DESCR_Battery_SCC_Volt,               // WW.WW
    DESCR_Battery_Discharge_Current,      // PPPP
    DESCR_Status_Flag,                    // b0-b7
    DESCR_Battery_Voltage_Offset_Fans_On, // QQ
    DESCR_EEPROM_Version,                 // VV
    DESCR_PV_Charging_Power,              // MMMM
    DESCR_Device_Status,                  // b8-b10
    DESCR_Solar_Feed_To_Grid_Status,      // Y
    DESCR_Country,                        // ZZ
    DESCR_Solar_Feed_To_Grid_Power,       // AAAA
};
unsigned int qpigs_90_length = 17;
const char *const qpigs_90[] = {
    // 90 long have 17 fields
    // [PI34 / MPPT-3000], [PI30 HS MS MSX], [PI30 Revo], [PI30 PIP], [PI41 / LV5048]
    DESCR_AC_In_Voltage,             // BBB.B
    DESCR_AC_In_Frequenz,            // CC.C
    DESCR_AC_Out_Voltage,            // DDD.D
    DESCR_AC_Out_Frequenz,           // EE.E
    DESCR_AC_Out_VA,                 // FFFF
    DESCR_AC_Out_Watt,               // GGGG
    DESCR_AC_Out_Percent,            // HHH
    DESCR_Inverter_Bus_Voltage,      // III
    DESCR_Battery_Voltage,           // JJ.JJ
    DESCR_Battery_Charge_Current,    // KKK
    DESCR_Battery_Percent,           // OOO
    DESCR_Inverter_Bus_Temperature,  // TTTT
    DESCR_PV_Input_Current,          // EE.E
    DESCR_PV_Input_Voltage,          // UUU.U
    DESCR_Battery_SCC_Volt,          // WW.WW
    DESCR_Battery_Discharge_Current, // PPPP
    DESCR_Status_Flag,               // b0-b7
};
unsigned int qpigs_21_length = 21;
const char *const qpigs_21[] = {
    // 21 fields
    // [PI30 PIP-GK/MK], [PI41 / LV5048], [PI30 REVO reserved tail]
    DESCR_AC_In_Voltage,                  // BBB.B
    DESCR_AC_In_Frequenz,                 // CC.C
    DESCR_AC_Out_Voltage,                 // DDD.D
    DESCR_AC_Out_Frequenz,                // EE.E
    DESCR_AC_Out_VA,                      // FFFF
    DESCR_AC_Out_Watt,                    // GGGG
    DESCR_AC_Out_Percent,                 // HHH
    DESCR_Inverter_Bus_Voltage,           // III
    DESCR_Battery_Voltage,                // JJ.JJ
    DESCR_Battery_Charge_Current,         // KKK
    DESCR_Battery_Percent,                // OOO
    DESCR_Inverter_Bus_Temperature,       // TTTT
    DESCR_PV_Input_Current,               // EE.E
    DESCR_PV_Input_Voltage,               // UUU.U
    DESCR_Battery_SCC_Volt,               // WW.WW
    DESCR_Battery_Discharge_Current,      // PPPP
    DESCR_Status_Flag,                    // b0-b7
    DESCR_Battery_Voltage_Offset_Fans_On, // QQ
    DESCR_EEPROM_Version,                 // VV
    DESCR_PV_Charging_Power,              // MMMMM
    DESCR_Device_Status,                  // b10b9b8
};
unsigned int qallList_length = 18;
const char *const qallList[] = {
    // [PI30 Revo]
    DESCR_AC_In_Voltage,             // BBB.B
    DESCR_AC_In_Frequenz,            // CC.C
    DESCR_AC_Out_Voltage,            // DDD.D
    DESCR_AC_Out_Frequenz,           // EE.E
    DESCR_AC_Out_Watt,               // FFFF
    DESCR_AC_Out_Percent,            // GGG
    DESCR_Battery_Voltage,           // HH.H
    DESCR_Battery_Percent,           // III
    DESCR_Battery_Charge_Current,    // JJJ
    DESCR_Battery_Discharge_Current, // KKK
    DESCR_PV_Input_Voltage,          // LLL
    DESCR_PV_Input_Current,          // MM.M
    DESCR_PV_Charging_Power,         // NNNN
    DESCR_PV_Generation_Day,         // OOOOOO
    DESCR_PV_Generation_Sum,         // PPPPPP
    DESCR_Inverter_Operation_Mode,   // Q
    DESCR_Warning_Code,              // KK
    DESCR_Fault_Code,                // SS

};
const char *const P005GS[][28] = {
    {DESCR_AC_In_Voltage, "10"},            // AAAA
    {DESCR_AC_In_Frequenz, "10"},           // BBB
    {DESCR_AC_Out_Voltage, "10"},           // CCCC
    {DESCR_AC_Out_Frequenz, "10"},          // DDD
    {DESCR_AC_Out_VA, "0"},                 // EEEE
    {DESCR_AC_Out_Watt, "0"},                   // FFFF
    {DESCR_AC_Out_Percent, "0"},            // GGGG
    {DESCR_Battery_Voltage, "10"},          // HHHH
    {DESCR_Battery_SCC_Volt, "10"},         // III
    {DESCR_Battery_SCC2_Volt, "10"},            // JJJ
    {DESCR_Battery_Discharge_Current, "0"}, // KKK
    {DESCR_Battery_Charge_Current, "0"},    // LLL
    {DESCR_Battery_Percent, "0"},           // MMM
    {DESCR_Inverter_Bus_Temperature, "0"},  // NNN
    {DESCR_MPPT1_Charger_Temperature, "0"},     // OOO
    {DESCR_MPPT2_Charger_Temperature, "0"},     // PPP
    {DESCR_PV1_Input_Power, "0"},               // QQQQ
    {DESCR_PV2_Input_Power, "0"},               // RRRR
    {DESCR_PV1_Input_Voltage, "10"},            // SSSS
    {DESCR_PV2_Input_Voltage, "10"},            // TTTT
    {DESCR_Configuration_State, "0"},           // U
    {DESCR_MPPT1_Charger_Status, "0"},          // V
    {DESCR_MPPT2_CHarger_Status, "0"},          // W
    {DESCR_Load_Connection, "0"},               // X
    {DESCR_Battery_Power_Direction, "0"},       // Y
    {DESCR_ACDC_Power_Direction, "0"},          // Z
    {DESCR_Line_Power_Direction, "0"},          // a
    {DESCR_Local_Parallel_ID, "0"},             // b
};

bool PI_Serial::PIXX_QPIGS()
{
  const char *const *qpigsList = nullptr;
  unsigned int qpigsList_length;
  if (protocol == PI30)
  {
    // byte protocolNum = 0; // for future use
    get.raw.qall = "";
    String commandAnswerQALL = this->requestData("QALL");
    get.raw.qall = commandAnswerQALL;
    String commandAnswerQPIGS = this->requestData("QPIGS");
    get.raw.qpigs = commandAnswerQPIGS;
    if (commandAnswerQPIGS == DESCR_req_NAK || commandAnswerQPIGS == DESCR_req_NOA)
      return true;
    if (commandAnswerQPIGS == DESCR_req_ERCRC)
      return false;

    // Split the string into substrings
    String strs[30]; // buffer for string splitting
    int StringCount = 0;
    String commandAnswerQPIGSPayload = commandAnswerQPIGS;
    while (commandAnswerQPIGSPayload.length() > 0 && StringCount < 30)
    {
      int index = commandAnswerQPIGSPayload.indexOf(delimiter);
      if (index == -1) // No separator found
      {
        strs[StringCount++] = commandAnswerQPIGSPayload;
        break;
      }
      else
      {
        strs[StringCount++] = commandAnswerQPIGSPayload.substring(0, index);
        commandAnswerQPIGSPayload = commandAnswerQPIGSPayload.substring(index + 1);
      }
    }

    if (StringCount >= (int)qpigs_106_length)
    {
      qpigsList = qpigs_106;
      qpigsList_length = qpigs_106_length;
      for (unsigned int i = 0; i < qpigsList_length && i < (unsigned int)StringCount; i++)
      {
        if (!strs[i].isEmpty() && strcmp(qpigsList[i], "") != 0)
        {
          liveData[qpigsList[i]] = (int)(strs[i].toFloat() * 100 + 0.5) / 100.0;
        }
      }
      // make some things pretty
      liveData[DESCR_Battery_Load] = (liveData[DESCR_Battery_Charge_Current].as<unsigned short>() - liveData[DESCR_Battery_Discharge_Current].as<unsigned short>());
      liveData[DESCR_PV_Input_Power] = (liveData[DESCR_PV_Input_Voltage].as<unsigned short>() * liveData[DESCR_PV_Input_Current].as<unsigned short>());
    }
    else if (StringCount >= (int)qpigs_21_length)
    {
      qpigsList = qpigs_21;
      qpigsList_length = qpigs_21_length;
      for (unsigned int i = 0; i < qpigsList_length && i < (unsigned int)StringCount; i++)
      {
        if (!strs[i].isEmpty() && strcmp(qpigsList[i], "") != 0)
        {
          liveData[qpigsList[i]] = (int)(strs[i].toFloat() * 100 + 0.5) / 100.0;
        }
      }
      liveData[DESCR_Battery_Load] = (liveData[DESCR_Battery_Charge_Current].as<unsigned short>() - liveData[DESCR_Battery_Discharge_Current].as<unsigned short>());
      liveData[DESCR_PV_Input_Power] = (liveData[DESCR_PV_Input_Voltage].as<unsigned short>() * liveData[DESCR_PV_Input_Current].as<unsigned short>());
    }
    else if (StringCount >= (int)qpigs_90_length)
    {
      qpigsList = qpigs_90;
      qpigsList_length = qpigs_90_length;
      for (unsigned int i = 0; i < qpigsList_length && i < (unsigned int)StringCount; i++)
      {
        if (!strs[i].isEmpty() && strcmp(qpigsList[i], "") != 0)
        {
          liveData[qpigsList[i]] = (int)(strs[i].toFloat() * 100 + 0.5) / 100.0;
        }
      }
      liveData[DESCR_Battery_Load] = (liveData[DESCR_Battery_Charge_Current].as<unsigned short>() - liveData[DESCR_Battery_Discharge_Current].as<unsigned short>());
      liveData[DESCR_PV_Input_Power] = (liveData[DESCR_PV_Input_Voltage].as<unsigned short>() * liveData[DESCR_PV_Input_Current].as<unsigned short>());
    }
    else
    {
      get.raw.qpigs = "Wrong Field Count(" + (String)StringCount + "), Contact Dev:" + get.raw.qpigs;
    }


    if (get.raw.qall.length() > 0 && commandAnswerQALL != DESCR_req_NAK && commandAnswerQALL != DESCR_req_NOA && commandAnswerQALL != DESCR_req_ERCRC)
    {
      String strsQALL[30];
      //  Split the string into substrings
      int StringCountQALL = 0;
      String commandAnswerQALLPayload = commandAnswerQALL;
      while (commandAnswerQALLPayload.length() > 0 && StringCountQALL < 30)
      {
        int index = commandAnswerQALLPayload.indexOf(delimiter);
        if (index == -1) // No separator found
        {
          strsQALL[StringCountQALL++] = commandAnswerQALLPayload;
          break;
        }
        else
        {
          strsQALL[StringCountQALL++] = commandAnswerQALLPayload.substring(0, index);
          commandAnswerQALLPayload = commandAnswerQALLPayload.substring(index + 1);
        }
      }

      if (StringCountQALL >= (int)qallList_length)
      {
        for (unsigned int i = 0; i < qallList_length && i < (unsigned int)StringCountQALL; i++)
        {
           if (!strsQALL[i].isEmpty() && strcmp(qallList[i], "") != 0)
             liveData[qallList[i]] = (int)(strsQALL[i].toFloat() * 100 + 0.5) / 100.0;
        }
        liveData[DESCR_Inverter_Operation_Mode] = getModeDesc((char)liveData[DESCR_Inverter_Operation_Mode].as<String>().charAt(0));
        liveData[DESCR_Battery_Load] = (liveData[DESCR_Battery_Charge_Current].as<unsigned short>() - liveData[DESCR_Battery_Discharge_Current].as<unsigned short>());
      }
      else
      {
        get.raw.qall = "Wrong Field Count(" + (String)StringCountQALL + "), Contact Dev:" + get.raw.qall;
      }
    }

    return true;
  }
  else if (protocol == PI18)
  {
    String commandAnswer = this->requestData("^P005GS");
    get.raw.qpigs = commandAnswer;
    if (commandAnswer == DESCR_req_NAK || commandAnswer == DESCR_req_NOA)
      return true;
    if (commandAnswer == DESCR_req_ERCRC)
      return false;
    // Split the string into substrings
    String strs[30]; // buffer for string splitting
    int StringCount = 0;
    String commandAnswerPayload = commandAnswer;
    while (commandAnswerPayload.length() > 0 && StringCount < 30)
    {
      int index = commandAnswerPayload.indexOf(delimiter);
      if (index == -1) // No separator found
      {
        strs[StringCount++] = commandAnswerPayload;
        break;
      }
      else
      {
        strs[StringCount++] = commandAnswerPayload.substring(0, index);
        commandAnswerPayload = commandAnswerPayload.substring(index + 1);
      }
    }

    if (StringCount >= (int)(sizeof P005GS / sizeof P005GS[0]))
    {
      for (unsigned int i = 0; i < sizeof P005GS[0] / sizeof P005GS[0][0]; i++)
      {
        if (!strs[i].isEmpty() && strcmp(P005GS[i][0], "") != 0)
        {
          if (atoi(P005GS[i][1]) > 0)
          {
            liveData[P005GS[i][0]] = (int)((strs[i].toFloat() / atoi(P005GS[i][1])) * 100 + 0.5) / 100.0;
          }
          else if (atoi(P005GS[i][1]) == 0)
          {
            liveData[P005GS[i][0]] = strs[i].toInt();
          }
          else
          {
            liveData[P005GS[i][0]] = strs[i];
          }
        }
      }
      // make some things pretty

       liveData[DESCR_PV_Input_Voltage] = (liveData[DESCR_PV1_Input_Voltage].as<unsigned short>() + liveData[DESCR_PV2_Input_Voltage].as<unsigned short>());
       liveData[DESCR_PV_Charging_Power] = (liveData[DESCR_PV1_Input_Power].as<unsigned short>() + liveData[DESCR_PV2_Input_Power].as<unsigned short>());
       liveData[DESCR_PV_Input_Current] = (int)((liveData[DESCR_PV_Charging_Power].as<unsigned short>() / (liveData[DESCR_PV_Input_Voltage].as<unsigned short>() + 0.5)) * 100) / 100.0;
       liveData[DESCR_Battery_Load] = (liveData[DESCR_Battery_Charge_Current].as<unsigned short>() - liveData[DESCR_Battery_Discharge_Current].as<unsigned short>());
    }
    else
    {
      get.raw.qpigs = "Wrong Field Count(" + (String)StringCount + "), Contact Dev:" + get.raw.qpigs;
    }
    return true;
  }
  else if (protocol == NoD)
  {
    return false;
  }
  else
  {
    return false;
  }
}
