bool PI_Serial::PIXX_QEX()
{
  if (protocol == PI30)
  {
    String commandAnswer = this->requestData("QET");
    get.raw.qet = commandAnswer;
    if (commandAnswer == DESCR_req_NAK || commandAnswer == DESCR_req_NOA)
    {
      return true;
    }
    if (commandAnswer == DESCR_req_ERCRC)
    {
      return false;
    }
    liveData[DESCR_PV_Generation_Sum] = commandAnswer.toInt();
    commandAnswer = this->requestData("QT");
    get.raw.qt = commandAnswer.substring(0, 8);
    if (commandAnswer == DESCR_req_ERCRC)
    {
      return false;
    }
    if (commandAnswer == DESCR_req_NAK || commandAnswer == DESCR_req_NOA)
    {
      return true;
    }
    else
    {
      commandAnswer = this->requestData("QEY" + get.raw.qt.substring(0, 4));
      get.raw.qey = commandAnswer;
      if (commandAnswer == DESCR_req_ERCRC)
        return false;
      if (commandAnswer == DESCR_req_NAK || commandAnswer == DESCR_req_NOA)
        return true;
      liveData[DESCR_PV_Generation_Year] = commandAnswer.toInt();

      commandAnswer = this->requestData("QEM" + get.raw.qt.substring(0, 6));
      get.raw.qem = commandAnswer;
      if (commandAnswer == DESCR_req_ERCRC)
        return false;
      if (commandAnswer == DESCR_req_NAK || commandAnswer == DESCR_req_NOA)
        return true;
      liveData[DESCR_PV_Generation_Month] = commandAnswer.toInt();

      commandAnswer = this->requestData("QED" + get.raw.qt.substring(0, 8));
      get.raw.qed = commandAnswer;
      if (commandAnswer == DESCR_req_ERCRC)
        return false;
      if (commandAnswer == DESCR_req_NAK || commandAnswer == DESCR_req_NOA)
        return true;
      liveData[DESCR_PV_Generation_Day] = commandAnswer.toInt();

      commandAnswer = this->requestData("QLT");
      get.raw.qlt = commandAnswer;
      if (commandAnswer == DESCR_req_ERCRC)
        return false;
      if (commandAnswer == DESCR_req_NAK || commandAnswer == DESCR_req_NOA)
        return true;
      liveData[DESCR_AC_In_Generation_Sum] = commandAnswer.toInt();

      commandAnswer = this->requestData("QLY" + get.raw.qt.substring(0, 4));
      get.raw.qly = commandAnswer;
      if (commandAnswer == DESCR_req_ERCRC)
        return false;
      if (commandAnswer == DESCR_req_NAK || commandAnswer == DESCR_req_NOA)
        return true;
      liveData[DESCR_AC_In_Generation_Year] = commandAnswer.toInt();

      commandAnswer = this->requestData("QLM" + get.raw.qt.substring(0, 6));
      get.raw.qlm = commandAnswer;
      if (commandAnswer == DESCR_req_ERCRC)
        return false;
      if (commandAnswer == DESCR_req_NAK || commandAnswer == DESCR_req_NOA)
        return true;
      liveData[DESCR_AC_In_Generation_Month] = commandAnswer.toInt();

      commandAnswer = this->requestData("QLD" + get.raw.qt.substring(0, 8));
      get.raw.qld = commandAnswer;
      if (commandAnswer == DESCR_req_ERCRC)
        return false;
      if (commandAnswer == DESCR_req_NAK || commandAnswer == DESCR_req_NOA)
        return true;
      liveData[DESCR_AC_In_Generation_Day] = commandAnswer.toInt();

      return true;
    }
  }
  else if (protocol == PI18)
  {
    String commandAnswer = "";
    switch (qexCounter)
    {
    case 0:
      commandAnswer = this->requestData("^P004T");
      if (commandAnswer == DESCR_req_ERCRC || commandAnswer == DESCR_req_NAK || commandAnswer == DESCR_req_NOA || commandAnswer == "" || commandAnswer.toInt() < 1)
      {
        qexCounter = 0;
        get.raw.qt = "";
        return true;
      }
      else
      {
        get.raw.qt = commandAnswer;
        qexCounter++;
      }
      break;
    case 1:
      commandAnswer = this->requestData("^P013ED" + get.raw.qt.substring(0, 8));
      if (commandAnswer == DESCR_req_ERCRC || commandAnswer == DESCR_req_NAK || commandAnswer == DESCR_req_NOA || commandAnswer == "" || get.raw.qt == "")
      {
        qexCounter = 0;
        return true;
      }
      else
      {
        get.raw.qed = commandAnswer;
        liveData[DESCR_PV_Generation_Day] = commandAnswer.toInt();
        qexCounter++;
      }
      break;
    case 2:
      commandAnswer = this->requestData("^P011EM" + get.raw.qt.substring(0, 6));
      if (commandAnswer == DESCR_req_ERCRC || commandAnswer == DESCR_req_NAK || commandAnswer == DESCR_req_NOA || commandAnswer == "" || get.raw.qt == "")
      {
        qexCounter = 0;
        return true;
      }
      else
      {
        get.raw.qem = commandAnswer;
        liveData[DESCR_PV_Generation_Month] = commandAnswer.toInt();
        qexCounter++;
      }
      break;
    case 3:
      commandAnswer = this->requestData("^P009EY" + get.raw.qt.substring(0, 4));
      if (commandAnswer == DESCR_req_ERCRC || commandAnswer == DESCR_req_NAK || commandAnswer == DESCR_req_NOA || commandAnswer == "" || get.raw.qt == "")
      {
        qexCounter = 0;
        return true;
      }
      else
      {
        get.raw.qey = commandAnswer;
        liveData[DESCR_PV_Generation_Year] = commandAnswer.toInt();
        qexCounter++;
      }
      break;
    case 4:
      commandAnswer = this->requestData("^P005ET");
      if (commandAnswer == DESCR_req_ERCRC || commandAnswer == DESCR_req_NAK || commandAnswer == DESCR_req_NOA || commandAnswer == "")
      {
        qexCounter = 0;
        return true;
      }
      else
      {
        get.raw.qet = commandAnswer;
        liveData[DESCR_PV_Generation_Sum] = commandAnswer.toInt();
        qexCounter = 0;
      }
      break;

    default:
      break;
    } // return true;

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
