static const char *const qpigs2List[] = {
    // [PI34 / MPPT-3000], [PI30 HS MS MSX], [PI30 Revo], [PI30 PIP], [PI41 / LV5048]
    DESCR_PV2_Input_Current,  // BBB.B
    DESCR_PV2_Input_Voltage,  // CCC.C
    DESCR_PV2_Charging_Power, // DDDD
};
static const char *const qpigs2ListL2[] = {
    // [PI41 / LV5048]
    DESCR_AC_In_Voltage_L2,  // AAA.A
    DESCR_AC_In_Frequenz_L2, // BB.B
    DESCR_AC_Out_Voltage_L2, // CCC.C
    DESCR_AC_Out_Frequenz_L2, // DD.D
    DESCR_AC_Out_VA_L2,       // EEEE
    DESCR_AC_Out_Watt_L2,     // FFFF
    DESCR_AC_Out_Percent_L2,  // GGG
    DESCR_PV2_Input_Current,  // HHHH
    DESCR_PV2_Input_Voltage,  // III.I
    DESCR_Battery_Voltage_L2, // JJ.JJ
    DESCR_Device_Status_L2,   // b7b6b5b4b3b2b1b0
};

bool PI_Serial::PIXX_QPIGS2()
{
  if (protocol == PI30)
  {
    String commandAnswer = this->requestData("QPIGS2");
    get.raw.qpigs2 = commandAnswer;
    if (commandAnswer == DESCR_req_NAK || commandAnswer == DESCR_req_NOA)
    {
      return true;
    }
    if (commandAnswer == DESCR_req_ERCRC)
    {
      return false;
    }

    char bufQPIGS2[128];
    commandAnswer.toCharArray(bufQPIGS2, sizeof(bufQPIGS2));
    char *fieldsQPIGS2[30];
    int StringCount = pi_split_fields(bufQPIGS2, delimiter[0], fieldsQPIGS2, 30);

    if (StringCount >= (int)(sizeof qpigs2ListL2 / sizeof qpigs2ListL2[0]))
    {
      for (unsigned int i = 0; i < sizeof qpigs2ListL2 / sizeof qpigs2ListL2[0]; i++)
      {
        if (fieldsQPIGS2[i][0] != '\0' && strcmp(qpigs2ListL2[i], "") != 0)
        {
          liveData[qpigs2ListL2[i]] = pi_parse_float2(fieldsQPIGS2[i]);
        }
      }
      liveData[DESCR_PV2_Input_Power] = (liveData[DESCR_PV2_Input_Voltage].as<unsigned short>() * liveData[DESCR_PV2_Input_Current].as<unsigned short>());
    }
    else if (StringCount >= (int)(sizeof qpigs2List / sizeof qpigs2List[0]))
    {
      for (unsigned int i = 0; i < sizeof qpigs2List / sizeof qpigs2List[0]; i++)
      {
        if (fieldsQPIGS2[i][0] != '\0' && strcmp(qpigs2List[i], "") != 0)
          liveData[qpigs2List[i]] = pi_parse_float2(fieldsQPIGS2[i]);
      }
      // make some things pretty
      liveData[DESCR_PV2_Input_Power] = (liveData[DESCR_PV2_Input_Voltage].as<unsigned short>() * liveData[DESCR_PV2_Input_Current].as<unsigned short>());
    }
    else
    {
      get.raw.qpigs2 = "Wrong Field Count(" + (String)StringCount + "), Contact Dev:" + get.raw.qpigs2;
    }
    return true;
  }
  else if (protocol == PI18)
  {
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
