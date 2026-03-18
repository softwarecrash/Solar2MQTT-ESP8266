bool PI_Serial::PIXX_QFLAG()
{
    if (protocol == PI30)
    {
        String commandAnswer = this->requestData("QFLAG");
        get.raw.qflag = commandAnswer;
        byte commandAnswerLength = commandAnswer.length();
        if (commandAnswer == DESCR_req_NAK || commandAnswer == DESCR_req_NOA)
        {
            return true;
        }
        if (commandAnswer == DESCR_req_ERCRC)
        {
            return false;
        }
        if (commandAnswerLength == 11)
        {
            staticData[DESCR_Buzzer_Enabled] = checkQFLAG(commandAnswer, 'a');
            staticData[DESCR_Overload_Bypass_Enabled] = checkQFLAG(commandAnswer, 'b');
            staticData[DESCR_Power_Saving_Enabled] = checkQFLAG(commandAnswer, 'j');
            staticData[DESCR_LCD_Reset_To_Default_Enabled] = checkQFLAG(commandAnswer, 'k');
            staticData[DESCR_Overload_Restart_Enabled] = checkQFLAG(commandAnswer, 'u');
            staticData[DESCR_Over_Temperature_Restart_Enabled] = checkQFLAG(commandAnswer, 'v');
            staticData[DESCR_LCD_Backlight_Enabled] = checkQFLAG(commandAnswer, 'x');
            staticData[DESCR_Primary_Source_Interrupt_Alarm_Enabled] = checkQFLAG(commandAnswer, 'y');
            staticData[DESCR_Record_Fault_Code_Enabled] = checkQFLAG(commandAnswer, 'z');
            
        } else if(commandAnswerLength == 14){
            //https://github.com/softwarecrash/Solar2MQTT/issues/181
            /*
            l (Data_Log_Pop_Up)???
            g
            d
            */
            staticData[DESCR_Buzzer_Enabled] = checkQFLAG(commandAnswer, 'a');
            staticData[DESCR_Overload_Bypass_Enabled] = checkQFLAG(commandAnswer, 'b');
            //staticData[DESCR_unknown] = checkQFLAG(commandAnswer, 'd');
            //staticData[DESCR_unknown] = checkQFLAG(commandAnswer, 'g');
            staticData[DESCR_Power_Saving_Enabled] = checkQFLAG(commandAnswer, 'j');
            staticData[DESCR_LCD_Reset_To_Default_Enabled] = checkQFLAG(commandAnswer, 'k');
            staticData[DESCR_Data_Log_Pop_Up] = checkQFLAG(commandAnswer, 'l');
            staticData[DESCR_Overload_Restart_Enabled] = checkQFLAG(commandAnswer, 'u');
            staticData[DESCR_Over_Temperature_Restart_Enabled] = checkQFLAG(commandAnswer, 'v');
            staticData[DESCR_LCD_Backlight_Enabled] = checkQFLAG(commandAnswer, 'x');
            staticData[DESCR_Primary_Source_Interrupt_Alarm_Enabled] = checkQFLAG(commandAnswer, 'y');
            staticData[DESCR_Record_Fault_Code_Enabled] = checkQFLAG(commandAnswer, 'z');

        } else {
            get.raw.qflag = "Wrong Length(" + (String)get.raw.qflag.length() + "), Contact Dev:" +get.raw.qflag;
        }
        return true;
    }
    else if (protocol == PI18)
    {
        String commandAnswer = this->requestData("^P007FLAG");
        get.raw.qflag = commandAnswer;
        byte commandAnswerLength = commandAnswer.length();
        if (commandAnswer == DESCR_req_NAK || commandAnswer == DESCR_req_NOA)
        {
            return true;
        }
        if (commandAnswer == DESCR_req_ERCRC)
        {
            return false;
        }
        //[C: ^P007FLAG][CR: 190F][CC: 190F][L:  17]
        //QFLAG 1,1,0,0,0,1,1,1,0
        if (commandAnswerLength == 17)
        {
            staticData[DESCR_Buzzer_Enabled] = ((String)commandAnswer.charAt(0) == "1") ? true : false;
            staticData[DESCR_Overload_Bypass_Enabled] = ((String)(commandAnswer.charAt(2)) == "1") ? true : false;
            staticData[DESCR_LCD_Reset_To_Default_Enabled] = ((String)(commandAnswer.charAt(4)) == "1") ? true : false;
            staticData[DESCR_Overload_Restart_Enabled] = ((String)(commandAnswer.charAt(6)) == "1") ? true : false;
            staticData[DESCR_Over_Temperature_Restart_Enabled] = ((String)(commandAnswer.charAt(8)) == "1") ? true : false;
            staticData[DESCR_LCD_Backlight_Enabled] = ((String)(commandAnswer.charAt(10)) == "1") ? true : false;
            staticData[DESCR_Primary_Source_Interrupt_Alarm_Enabled] = ((String)(commandAnswer.charAt(12)) == "1") ? true : false;
            staticData[DESCR_Record_Fault_Code_Enabled] = ((String)(commandAnswer.charAt(14)) == "1") ? true : false;
        
        } else {
            get.raw.qflag = "Wrong Length(" + (String)get.raw.qflag.length() + "), Contact Dev:" +get.raw.qflag;
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
