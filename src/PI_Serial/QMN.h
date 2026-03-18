bool PI_Serial::PIXX_QMN()
{
  if (protocol == PI30)
  {
    String commandAnswer = this->requestData("QMN");
    get.raw.qmn = commandAnswer;
    // calculate the length with https://elmar-eigner.de/text-zeichen-laenge.html
    if (commandAnswer == DESCR_req_NAK || commandAnswer == DESCR_req_NOA)
    {
      return true;
    }
    if (commandAnswer == DESCR_req_ERCRC)
    {
      return false;
    }
    if (commandAnswer.length() > 3 && commandAnswer.length() < 50)
    {
      staticData[DESCR_Device_Model] = commandAnswer;
      return true;
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