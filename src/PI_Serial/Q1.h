/*
All known devices with PIxx protocol are react to the Q1 comand

examples:
27 fields, length 109: 00001 16971 01 00 00 026 033 022 029 02 00 000 0036 0000 0000 00.00 10 0 060 030 100 030 58.40 000 120 0 0000 - from https://github.com/jblance/mpp-solar/discussions/284
27 fields, length 105:     0 00027 00 00 00 000 033 027 035 02 00 000 0030 0000 0000 49.95 10 0 000 000 000 000 00.00 000 000 0 0000 - from https://github.com/softwarecrash/Solar2MQTT/issues/158
17 fields, length 70:  07792 00001 00 00 00 000 034 030 000 01 00 000 0030 0000 0000 00.00 13 from - -from https://github.com/softwarecrash/Solar2MQTT/discussions/144
13 fields, length 47:  00 00 00 000 026 021 026 00 00 000 0030 0000 13 -from https://github.com/softwarecrash/Solar2MQTT/discussions/144
13 fields, length 47:  00 00 00 000 039 030 031 00 00 000 0030 0000 11 - from manuA
22 fields, length 87   00000 00000 01 01 00 030 027 028 029 00 00 000 0031 0364 0000 00.00 11 07 00 3741 530 3 - from derLoosi
length helper: https://elmar-eigner.de/text-zeichen-laenge.html
*/
unsigned int Q1_47_length = 13;
const char *const Q1_47[] = {
    DESCR_Time_Until_Absorb_Charge,
    DESCR_Time_Until_Float_Charge,
    DESCR_unknown,
    DESCR_Tracker_Temperature,
    DESCR_Inverter_Temperature,
    DESCR_Battery_Temperature,
    DESCR_Transformer_Temperature,
    DESCR_unknown,
    DESCR_unknown,
    DESCR_unknown,
    DESCR_Fan_Speed,
    DESCR_unknown,
    DESCR_Inverter_Charge_State,
};
unsigned int Q1_70_length = 17;
const char *const Q1_70[] = {
    DESCR_Time_Until_Absorb_Charge, // Time until the end of absorb charging
    DESCR_Time_Until_Float_Charge,  // Time until the end of float charging
    DESCR_unknown,                  // SCC Flag
    DESCR_unknown,                  // AllowSccOnFlag
    DESCR_unknown,                  // ChargeAverageCurrent
    DESCR_Tracker_Temperature,      // SCC PWM temperature
    DESCR_Inverter_Temperature,
    DESCR_Battery_Temperature,
    DESCR_Transformer_Temperature,
    DESCR_unknown, // GPIO13
    DESCR_Fan_Lock_Status,
    DESCR_unknown,
    DESCR_Fan_Speed, // Fan PWM speed
    DESCR_unknown,   // SCC charge power
    DESCR_unknown,   // Parallel Warning
    DESCR_unknown,   // Sync frequency
    DESCR_Inverter_Charge_State,
};
unsigned int Q1_87_length = 17;
const char *const Q1_87[] = {
    DESCR_Time_Until_Absorb_Charge, // Time until the end of absorb charging
    DESCR_Time_Until_Float_Charge,  // Time until the end of float charging
    DESCR_unknown,                  // SCC Flag
    DESCR_unknown,                  // AllowSccOnFlag
    DESCR_unknown,                  // ChargeAverageCurrent
    DESCR_Tracker_Temperature,      // SCC PWM temperature
    DESCR_Inverter_Temperature,
    DESCR_Battery_Temperature,
    DESCR_Transformer_Temperature,
    DESCR_unknown, // GPIO13
    DESCR_Fan_Lock_Status,
    DESCR_unknown,
    DESCR_Fan_Speed, // Fan PWM speed
    DESCR_unknown,   // SCC charge power
    DESCR_unknown,   // Parallel Warning
    DESCR_unknown,   // Sync frequency
    DESCR_Inverter_Charge_State,
};
unsigned int Q1_22_length = 22;
const char *const Q1_22[] = {
    DESCR_Time_Until_Absorb_Charge, // Time until the end of absorb charging
    DESCR_Time_Until_Float_Charge,  // Time until the end of float charging
    DESCR_unknown,                  // SCC Flag
    DESCR_unknown,                  // AllowSccOnFlag
    DESCR_unknown,                  // ChargeAverageCurrent
    DESCR_Tracker_Temperature,      // SCC PWM temperature
    DESCR_Inverter_Temperature,
    DESCR_Battery_Temperature,
    DESCR_Transformer_Temperature,
    DESCR_unknown, // GPIO13
    DESCR_Fan_Lock_Status,
    DESCR_unknown,
    DESCR_Fan_Speed, // Fan PWM speed
    DESCR_unknown,   // SCC charge power
    DESCR_unknown,   // Parallel Warning
    DESCR_unknown,   // Sync frequency
    DESCR_Inverter_Charge_State,
    DESCR_unknown,
    DESCR_unknown,
    DESCR_unknown,
    DESCR_unknown,
    DESCR_unknown,
};
unsigned int Q1_108_length = 27;
const char *const Q1_108[] = {
    DESCR_Time_Until_Absorb_Charge, // Time until the end of absorb charging
    DESCR_Time_Until_Float_Charge,  // Time until the end of float charging
    DESCR_SCC_Flag,                 // SCC Flag
    DESCR_AllowSccOnFlag,           // AllowSccOnFlag
    DESCR_Charge_Average_Current,   // ChargeAverageCurrent
    DESCR_Tracker_Temperature,      // SCC PWM temperature
    DESCR_Inverter_Temperature,
    DESCR_Battery_Temperature,
    DESCR_Transformer_Temperature,
    DESCR_unknown, // GPIO13
    DESCR_Fan_Lock_Status,
    DESCR_unknown,
    DESCR_Fan_Speed,        // Fan PWM speed
    DESCR_SCC_Charge_Power, // SCC charge power
    DESCR_Parallel_Warning, // Parallel Warning
    DESCR_Sync_Frequency,   // Sync frequency
    DESCR_Inverter_Charge_State,
    DESCR_unknown,
    DESCR_unknown,
    DESCR_unknown,
    DESCR_unknown,
    DESCR_unknown,
    DESCR_unknown,
    DESCR_unknown,
    DESCR_unknown,
    DESCR_unknown,
    DESCR_unknown,
};

bool PI_Serial::PIXX_Q1()
{
    if (protocol == PI30 || protocol == PI18) // pi30 and pi18 devices react both on Q1
    {
        const char *const *q1List = nullptr;
        unsigned int q1LList_length = 0;
        String strs[30];
        int StringCount = 0;
        String commandAnswer = this->requestData("Q1");
        get.raw.q1 = commandAnswer;

        if (commandAnswer == DESCR_req_NAK || commandAnswer == DESCR_req_NOA)
            return true;
        if (commandAnswer == DESCR_req_ERCRC)
            return false;
        while (commandAnswer.length() > 0 && StringCount < 30)
        {
            int index = commandAnswer.indexOf(delimiter);
            if (index == -1) // No separator found
            {
                strs[StringCount++] = commandAnswer;
                break;
            }
            else
            {
                strs[StringCount++] = commandAnswer.substring(0, index);
                commandAnswer = commandAnswer.substring(index + 1);
            }
        }
        if (StringCount >= (int)Q1_108_length)
        {
            q1List = Q1_108;
            q1LList_length = Q1_108_length;
        }
        else if (StringCount >= (int)Q1_22_length)
        {
            q1List = Q1_22;
            q1LList_length = Q1_22_length;
        }
        else if (StringCount >= (int)Q1_70_length)
        {
            q1List = Q1_70;
            q1LList_length = Q1_70_length;
        }
        else if (StringCount >= (int)Q1_47_length)
        {
            q1List = Q1_47;
            q1LList_length = Q1_47_length;
        }
        else
        {
            get.raw.q1 = "Wrong Field Count(" + (String)StringCount + "), Contact Dev:" + get.raw.q1;
            return true;
        }
        for (unsigned int i = 0; i < q1LList_length && i < (unsigned int)StringCount; i++)
        {
            if (!strs[i].isEmpty() && strcmp(q1List[i], "") != 0)
                liveData[q1List[i]] = (int)(strs[i].toFloat() * 100 + 0.5) / 100.0;
        }

        if (liveData[DESCR_Inverter_Charge_State].is<JsonVariant>())
        {
            switch ((int)liveData[DESCR_Inverter_Charge_State].as<unsigned int>())
            {
            default:
                // liveData["Inverter_Charge_State"] = "no data";
                break;
            case 10:
                liveData[DESCR_Inverter_Charge_State] = DESCR_No_Charging;
                break;
            case 11:
                liveData[DESCR_Inverter_Charge_State] = DESCR_Bulk_Stage;
                break;
            case 12:
                liveData[DESCR_Inverter_Charge_State] = DESCR_Absorb;
                break;
            case 13:
                liveData[DESCR_Inverter_Charge_State] = DESCR_Float;
                break;
            }
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
