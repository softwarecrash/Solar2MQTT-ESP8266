bool PI_Serial::PIXX_QFLAG()
{
    if (protocol == PI30)
    {
        String commandAnswer = this->requestData("QFLAG");
        get.raw.qflag = commandAnswer;
        if (commandAnswer == DESCR_req_NAK || commandAnswer == DESCR_req_NOA)
        {
            return true;
        }
        if (commandAnswer == DESCR_req_ERCRC)
        {
            return false;
        }
        const bool hasStateMarkers = commandAnswer.indexOf('E') >= 0 || commandAnswer.indexOf('D') >= 0;
        if (hasStateMarkers)
        {
            staticData[DESCR_Buzzer_Enabled] = checkQFLAG(commandAnswer, 'a');
            staticData[DESCR_Overload_Bypass_Enabled] = checkQFLAG(commandAnswer, 'b');
            staticData[DESCR_Power_Saving_Enabled] = checkQFLAG(commandAnswer, 'j');
            staticData[DESCR_LCD_Reset_To_Default_Enabled] = checkQFLAG(commandAnswer, 'k');
            staticData[DESCR_Data_Log_Pop_Up] = checkQFLAG(commandAnswer, 'l');
            staticData[DESCR_Overload_Restart_Enabled] = checkQFLAG(commandAnswer, 'u');
            staticData[DESCR_Over_Temperature_Restart_Enabled] = checkQFLAG(commandAnswer, 'v');
            staticData[DESCR_LCD_Backlight_Enabled] = checkQFLAG(commandAnswer, 'x');
            staticData[DESCR_Primary_Source_Interrupt_Alarm_Enabled] = checkQFLAG(commandAnswer, 'y');
            staticData[DESCR_Record_Fault_Code_Enabled] = checkQFLAG(commandAnswer, 'z');
        }
        else
        {
            get.raw.qflag = "Wrong Format(" + (String)get.raw.qflag.length() + "), Contact Dev:" + get.raw.qflag;
        }
        return true;
    }
    else if (protocol == PI18)
    {
        String commandAnswer = this->requestData("^P007FLAG");
        get.raw.qflag = commandAnswer;
        if (commandAnswer == DESCR_req_NAK || commandAnswer == DESCR_req_NOA)
        {
            return true;
        }
        if (commandAnswer == DESCR_req_ERCRC)
        {
            return false;
        }

        auto parseBoolField = [](const char *value, bool &out) -> bool
        {
            if (value == nullptr)
            {
                return false;
            }
            while (*value == ' ' || *value == '\t')
            {
                value++;
            }
            if (*value == '1')
            {
                out = true;
                return true;
            }
            if (*value == '0')
            {
                out = false;
                return true;
            }
            if (*value == 'T' || *value == 't')
            {
                out = true;
                return true;
            }
            if (*value == 'F' || *value == 'f')
            {
                out = false;
                return true;
            }
            return false;
        };

        char bufQFLAG[64];
        commandAnswer.toCharArray(bufQFLAG, sizeof(bufQFLAG));
        char *fieldsQFLAG[16];
        int fieldCount = pi_split_fields(bufQFLAG, ',', fieldsQFLAG, 16);

        if (fieldCount >= 8)
        {
            bool values[8];
            bool valid = true;
            for (int i = 0; i < 8; i++)
            {
                valid = parseBoolField(fieldsQFLAG[i], values[i]) && valid;
            }

            if (valid)
            {
                staticData[DESCR_Buzzer_Enabled] = values[0];
                staticData[DESCR_Overload_Bypass_Enabled] = values[1];
                staticData[DESCR_LCD_Reset_To_Default_Enabled] = values[2];
                staticData[DESCR_Overload_Restart_Enabled] = values[3];
                staticData[DESCR_Over_Temperature_Restart_Enabled] = values[4];
                staticData[DESCR_LCD_Backlight_Enabled] = values[5];
                staticData[DESCR_Primary_Source_Interrupt_Alarm_Enabled] = values[6];
                staticData[DESCR_Record_Fault_Code_Enabled] = values[7];
                return true;
            }

            get.raw.qflag = "Wrong Format(" + (String)get.raw.qflag.length() + "), Contact Dev:" + get.raw.qflag;
            return true;
        }

        get.raw.qflag = "Wrong Field Count(" + (String)fieldCount + "), Contact Dev:" + get.raw.qflag;
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
