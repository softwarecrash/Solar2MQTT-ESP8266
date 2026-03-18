bool PI_Serial::PIXX_QPIWS()
{
    if (protocol == PI30)
    {
        String commandAnswer = this->requestData("QPIWS");
        //String commandAnswer = "10000000001010000000000000000000";
        get.raw.qpiws = commandAnswer;
        if (commandAnswer == DESCR_req_NAK || commandAnswer == DESCR_req_NOA)
        {
            return true;
        }
        if (commandAnswer == DESCR_req_ERCRC)
        {
            return false;
        }
        size_t length = commandAnswer.length();
        if (length >= 31)
        {
            char qpiwsStr[256];
            size_t qpiwsLen = 0;
            qpiwsStr[0] = '\0';
            auto isSet = [&](size_t index) { return index < length && commandAnswer.charAt(index) == '1'; };
            auto appendMsg = [&](const char *msg) {
                if (qpiwsLen + 2 < sizeof(qpiwsStr) && qpiwsLen > 0) {
                    qpiwsStr[qpiwsLen++] = ';';
                    qpiwsStr[qpiwsLen++] = ' ';
                }
                size_t msgLen = strlen(msg);
                size_t avail = sizeof(qpiwsStr) - 1 - qpiwsLen;
                if (msgLen > avail) {
                    msgLen = avail;
                }
                if (msgLen > 0) {
                    memcpy(qpiwsStr + qpiwsLen, msg, msgLen);
                    qpiwsLen += msgLen;
                    qpiwsStr[qpiwsLen] = '\0';
                }
            };
            if (isSet(1)) appendMsg("Inverter fault"); // 2
            if (isSet(2)) appendMsg("Bus over fault"); // 3
            if (isSet(3)) appendMsg("Bus under fault"); // 4
            if (isSet(4)) appendMsg("Bus soft fail fault"); // 5
            if (isSet(5)) appendMsg("Line fail warning"); // 6
            if (isSet(6)) appendMsg("OPV short warning"); // 7
            if (isSet(7)) appendMsg("Inverter voltage too low fault"); // 8
            if (isSet(8)) appendMsg("Inverter voltage too high fault"); // 9
            if (isSet(9)) appendMsg("Over temperature fault"); // 10
            if (isSet(10)) appendMsg("Fan locked fault"); // 11
            if (isSet(11)) appendMsg("Battery voltage too high fault"); // 12
            if (isSet(12)) appendMsg("Battery low alarm warning"); // 13
            if (isSet(14)) appendMsg("Battery under shutdown warning"); // 15
            if (isSet(16)) appendMsg("Overload fault"); // 17
            if (isSet(17)) appendMsg("EEPROM fault"); // 18
            if (isSet(18)) appendMsg("Inverter over current fault"); // 19
            if (isSet(19)) appendMsg("Inverter soft fail fault"); // 20
            if (isSet(20)) appendMsg("Self test fail fault"); // 21
            if (isSet(21)) appendMsg("OP DC voltage over fault"); // 22
            if (isSet(22)) appendMsg("Battery open fault"); // 23
            if (isSet(23)) appendMsg("Current sensor fail fault"); // 24
            if (isSet(24)) appendMsg("Battery short fault"); // 25
            if (isSet(25)) appendMsg("Power limit warning"); // 26
            if (isSet(26)) appendMsg("PV voltage high warning"); // 27
            if (isSet(27)) appendMsg("MPPT overload fault"); // 28
            if (isSet(28)) appendMsg("MPPT overload warning"); // 29
            if (isSet(29)) appendMsg("Battery too low to charge warning"); // 30
            liveData[DESCR_Fault_Code] = qpiwsLen ? qpiwsStr : "Ok";
        }else {
            get.raw.qpiws = "Wrong Length(" + (String)get.raw.qpiws.length() + "), Contact Dev:" +get.raw.qpiws;
        }
        return true;
    }
    else if(protocol == PI18){
        String commandAnswer = this->requestData("^P005FWS");
        get.raw.qpiws = commandAnswer;
        if (commandAnswer == DESCR_req_NAK || commandAnswer == DESCR_req_NOA)
        {
            return true;
        }
        if (commandAnswer == DESCR_req_ERCRC)
        {
            return false;
        }
        //[C: ^P005FWS][CR: B69E][CC: B69E][L:  36] from valqk
        //QPIWS 00,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
        if (commandAnswer.length() == 36)
        {
            char qpiwsStr[256];
            size_t qpiwsLen = 0;
            qpiwsStr[0] = '\0';
            auto appendMsg = [&](const char *msg) {
                if (qpiwsLen + 2 < sizeof(qpiwsStr) && qpiwsLen > 0) {
                    qpiwsStr[qpiwsLen++] = ';';
                    qpiwsStr[qpiwsLen++] = ' ';
                }
                size_t msgLen = strlen(msg);
                size_t avail = sizeof(qpiwsStr) - 1 - qpiwsLen;
                if (msgLen > avail) {
                    msgLen = avail;
                }
                if (msgLen > 0) {
                    memcpy(qpiwsStr + qpiwsLen, msg, msgLen);
                    qpiwsLen += msgLen;
                    qpiwsStr[qpiwsLen] = '\0';
                }
            };
            const char *code = commandAnswer.c_str();
            auto codeIs = [&](char a, char b) {
                return code[0] == a && code[1] == b;
            };
            if (codeIs('0', '1')) appendMsg("Fan is locked"); // 2
            if (codeIs('0', '2')) appendMsg("Over temperature"); // 2
            if (codeIs('0', '3')) appendMsg("Battery voltage is too high"); // 2
            if (codeIs('0', '4')) appendMsg("Battery voltage is too low"); // 2
            if (codeIs('0', '5')) appendMsg("Output short circuited or Over temperature"); // 2
            if (codeIs('0', '6')) appendMsg("Output voltage is too high"); // 2
            if (codeIs('0', '7')) appendMsg("Over load time out"); // 2
            if (codeIs('0', '8')) appendMsg("Bus voltage is too high"); // 2
            if (codeIs('0', '9')) appendMsg("Bus soft start failed"); // 2
            if (codeIs('1', '1')) appendMsg("Main relay failed"); // 2
            if (codeIs('5', '1')) appendMsg("Over current inverter"); // 2
            if (codeIs('5', '2')) appendMsg("Bus soft start failed"); // 2
            if (codeIs('5', '3')) appendMsg("Inverter soft start failed"); // 2
            if (codeIs('5', '4')) appendMsg("Self-test failed"); // 2
            if (codeIs('5', '5')) appendMsg("Over DC voltage on output of inverter"); // 2
            if (codeIs('5', '6')) appendMsg("Battery connection is open"); // 2
            if (codeIs('5', '7')) appendMsg("Current sensor failed"); // 2
            if (codeIs('5', '8')) appendMsg("Output voltage is too low"); // 2
            if (codeIs('6', '0')) appendMsg("Inverter negative power"); // 2
            if (codeIs('7', '1')) appendMsg("Parallel version different"); // 2
            if (codeIs('7', '2')) appendMsg("Output circuit failed"); // 2
            if (codeIs('8', '0')) appendMsg("CAN communication failed"); // 2
            if (codeIs('8', '1')) appendMsg("Parallel host line lost"); // 2
            if (codeIs('8', '2')) appendMsg("Parallel synchronized signal lost"); // 2
            if (codeIs('8', '3')) appendMsg("Parallel battery voltage detect different"); // 2
            if (codeIs('8', '4')) appendMsg("Parallel Line voltage or frequency detect different"); // 2
            if (codeIs('8', '5')) appendMsg("Parallel Line input current unbalanced"); // 2
            if (codeIs('8', '6')) appendMsg("Parallel output setting different"); // 2

            if ((char)commandAnswer.charAt(3) == '1') appendMsg("Line fail"); // 2
            if ((char)commandAnswer.charAt(5) == '1') appendMsg("Over temperature"); // 20
            if ((char)commandAnswer.charAt(7) == '1') appendMsg("Output circuit short"); // 3
            if ((char)commandAnswer.charAt(9) == '1') appendMsg("Inverter over temperature"); // 4
            if ((char)commandAnswer.charAt(11) == '1') appendMsg("Fan lock"); // 5
            if ((char)commandAnswer.charAt(13) == '1') appendMsg("Battery voltage high"); // 6
            if ((char)commandAnswer.charAt(15) == '1') appendMsg("Battery low"); // 7
            if ((char)commandAnswer.charAt(17) == '1') appendMsg("Battery under"); // 8
            if ((char)commandAnswer.charAt(19) == '1') appendMsg("Over load"); // 9
            if ((char)commandAnswer.charAt(21) == '1') appendMsg("Eeprom fail"); // 10
            if ((char)commandAnswer.charAt(23) == '1') appendMsg("Power limit"); // 11
            if ((char)commandAnswer.charAt(25) == '1') appendMsg("PV1 voltage high"); // 12
            if ((char)commandAnswer.charAt(27) == '1') appendMsg("PV2 voltage high"); // 13
            if ((char)commandAnswer.charAt(29) == '1') appendMsg("MPPT1 overload warning"); // 15
            if ((char)commandAnswer.charAt(31) == '1') appendMsg("MPPT2 overload warning"); // 17
            if ((char)commandAnswer.charAt(33) == '1') appendMsg("Battery too low to charge for SCC1"); // 18
            if ((char)commandAnswer.charAt(35) == '1') appendMsg("Battery too low to charge for SCC2"); // 19

            liveData[DESCR_Fault_Code] = qpiwsLen ? qpiwsStr : "Ok";
        }else {
            get.raw.qpiws = "Wrong Length(" + (String)get.raw.qpiws.length() + "), Contact Dev:" +get.raw.qpiws;
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
