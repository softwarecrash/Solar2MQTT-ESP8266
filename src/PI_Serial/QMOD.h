bool PI_Serial::PIXX_QMOD()
{
  if (protocol == PI30)
  {
    String commandAnswer = this->requestData("QMOD");
    get.raw.qmod = commandAnswer;
    if (commandAnswer == DESCR_req_NAK || commandAnswer == DESCR_req_NOA)
    {
      return true;
    }
    if (commandAnswer == DESCR_req_ERCRC)
    {
      return false;
    }
    if (commandAnswer.length() == 1)
    {
      liveData[DESCR_Inverter_Operation_Mode] = getModeDesc((char)commandAnswer.charAt(0));
    }
    return true;
  }
  else if (protocol == PI18)
  {
    String commandAnswer = this->requestData("^P006MOD");
    get.raw.qmod = commandAnswer;
    if (commandAnswer == DESCR_req_NAK || commandAnswer == DESCR_req_NOA)
    {
      return true;
    }
    if (commandAnswer == DESCR_req_ERCRC)
    {
      return false;
    }
    if (commandAnswer.length() == 2)
    {
      switch (commandAnswer.toInt())
      {
      case 0:
        liveData[DESCR_Inverter_Operation_Mode] = DESCR_Power_On;
        break;
      case 1:
        liveData[DESCR_Inverter_Operation_Mode] = DESCR_Standby;
        break;
      case 2:
        liveData[DESCR_Inverter_Operation_Mode] = DESCR_Bypass;
        break;
      case 3:
        liveData[DESCR_Inverter_Operation_Mode] = DESCR_Battery;
        break;
      case 4:
        liveData[DESCR_Inverter_Operation_Mode] = DESCR_Fault;
        break;
      case 5:
        liveData[DESCR_Inverter_Operation_Mode] = DESCR_Hybrid;
        break;
      default:
        liveData[DESCR_Inverter_Operation_Mode] = DESCR_No_Data;
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