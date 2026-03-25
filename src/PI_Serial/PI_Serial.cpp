// #define isDEBUG
#include "ArduinoJson.h"
#include "PI_Serial.h"

#include "CRC16.h"
#include "CRC.h"
CRC16 crc;
// static
#include "QPI.h"
#include "QPIRI.h"
#include "QMN.h"
#include "QFLAG.h"
// variable
#include "Q1.h"
#include "QPIGS.h"
#include "QPIGS2.h"
#include "QMOD.h"
#include "QEX.h"
#include "QPIWS.h"
extern void writeLog(const char *format, ...);
//----------------------------------------------------------------------
//  Public Functions
//----------------------------------------------------------------------

PI_Serial::PI_Serial(int rx, int tx)
{
    this->my_serialIntf = new SoftwareSerial(rx, tx, false); // init pins
    serialIntfBaud = 2400;
    get.raw.qpi.reserve(8);
    get.raw.qall.reserve(80);
    get.raw.qpiri.reserve(96);
    get.raw.qmn.reserve(48);
    get.raw.qflag.reserve(24);
    get.raw.q1.reserve(64);
    get.raw.qpigs.reserve(96);
    get.raw.qpigs2.reserve(24);
    get.raw.qmod.reserve(8);
    get.raw.qt.reserve(12);
    get.raw.qet.reserve(12);
    get.raw.qey.reserve(12);
    get.raw.qem.reserve(12);
    get.raw.qed.reserve(12);
    get.raw.qlt.reserve(12);
    get.raw.qly.reserve(12);
    get.raw.qlm.reserve(12);
    get.raw.qld.reserve(12);
    get.raw.commandAnswer.reserve(96);
    get.raw.qpiws.reserve(48);
    customCommandBuffer.reserve(32);
}

bool PI_Serial::Init()
{
    // Null check the serial interface
    if (this->my_serialIntf == NULL)
    {
        writeLog("No serial specificed!");
        return false;
    }
    autoDetect();
    if (isModbus())
    {
        if (requestCallback)
        {
            modbus->callback(requestCallback);
        }
        return true;
    }
    this->my_serialIntf->setTimeout(500);
    this->my_serialIntf->enableRxGPIOPullUp(true);
    this->my_serialIntf->begin(serialIntfBaud, SWSERIAL_8N1);
    return true;
}

bool PI_Serial::loop()
{
    if (suspendSerial)
    {
        return false;
    }
    if (millis() - previousTime > delayTime)
    {
        if (protocol != NoD)
        {
            if (sendCustomCommand())
            {
                requestStaticData = true;
                requestCounter = 0;
                previousTime = millis();
                return true;
            }
            if (isModbus())
            {
                modbus->loop();
            }
            else
            {
                switch (requestStaticData)
                {
                case true:
                    switch (requestCounter)
                    {
                    case 0:
                        requestCounter = PIXX_QPIRI() ? (requestCounter + 1) : 0;
                        break;
                    case 1:
                        requestCounter = PIXX_QMN() ? (requestCounter + 1) : 0;
                        break;
                    case 2:
                        requestCounter = PIXX_QPI() ? (requestCounter + 1) : 0;
                        break;
                    case 3:
                        requestCounter = PIXX_QFLAG() ? (requestCounter + 1) : 0;
                        requestCounter = 0;
                        requestStaticData = false;
                        break;
                    }
                    break;

                case false:
                    switch (requestCounter)
                    {
                    case 0:
                        requestCounter = PIXX_QPIGS() ? (requestCounter + 1) : 0;
                        break;
                    case 1:
                        requestCounter = PIXX_QPIGS2() ? (requestCounter + 1) : 0;
                        break;
                    case 2:
                        requestCounter = PIXX_QMOD() ? (requestCounter + 1) : 0;
                        break;
                    case 3:
                        requestCounter = PIXX_Q1() ? (requestCounter + 1) : 0;
                        break;
                    case 4:
                        requestCounter = PIXX_QEX() ? (requestCounter + 1) : 0;
                        break;
                    case 5:
                        requestCounter = PIXX_QPIWS() ? (requestCounter + 1) : 0;
                        break;
                    case 6:
                        requestCounter = PIXX_QPIRI() ? (requestCounter + 1) : 0;
                        break;
                    case 7:
                        requestCallback();
                        requestCounter = 0;
                        break;
                    }
                    break;
                }
            }
            if (isModbus())
            {
                connection = modbus->connection;
            }
            else
            {
                connection = (connectionCounter < 10) ? true : false;
            }
            previousTime = millis();
        }
        else
        {
            autoDetect();
            previousTime = millis();
            requestCallback();
            connection = false;
        }
    }
    return true;
}

void PI_Serial::callback(std::function<void()> func)
{
    requestCallback = func;
}

String PI_Serial::sendCommand(String command)
{
    if (command == "")
    {
        return command;
    }
    customCommandBuffer = command;
    return command;
}

bool PI_Serial::loopbackTest(String &details)
{
    details = "";
    if (this->my_serialIntf == nullptr)
    {
        details = "No serial interface";
        return false;
    }

    unsigned int baud = serialIntfBaud == 0 ? 2400 : serialIntfBaud;
    this->my_serialIntf->begin(baud, SWSERIAL_8N1);
    this->my_serialIntf->setTimeout(200);
    this->my_serialIntf->enableTx(true);

    while (this->my_serialIntf->available() > 0)
    {
        this->my_serialIntf->read();
    }

    const char *pattern = "S2MQT";
    const size_t patternLen = strlen(pattern);
    this->my_serialIntf->write(reinterpret_cast<const uint8_t *>(pattern), patternLen);
    this->my_serialIntf->flush();
    this->my_serialIntf->enableTx(false);
    delay(20);

    char buffer[8] = {};
    size_t readLen = this->my_serialIntf->readBytes(buffer, patternLen);
    bool ok = (readLen == patternLen && memcmp(buffer, pattern, patternLen) == 0);
    details = ok ? "Loopback OK" : "Loopback failed";
    return ok;
}

void PI_Serial::setSuspend(bool enabled)
{
    suspendSerial = enabled;
    if (enabled)
    {
        abortAutoDetect = true;
    }
    else
    {
        abortAutoDetect = false;
    }
}

bool PI_Serial::isSuspended() const
{
    return suspendSerial;
}

bool PI_Serial::isBusy() const
{
    return busyCount > 0;
}
//----------------------------------------------------------------------
// Private Functions
//----------------------------------------------------------------------
void PI_Serial::autoDetect() // function for autodetect the inverter type
{
    busyCount++;
    if (abortAutoDetect || suspendSerial)
    {
        writeLog("Autodetect aborted");
        goto autodetect_done;
    }
    protocol = NoD;
    if (modbus != nullptr)
    {
        delete modbus;
        modbus = nullptr;
    }
    writeLog("----------------- Start Autodetect -----------------");
    for (size_t i = 0; i < 3; i++) // try 3 times to detect the inverter
    {
        if (abortAutoDetect || suspendSerial)
        {
            writeLog("Autodetect aborted");
            goto autodetect_done;
        }
        writeLog("Try Autodetect Protocol");

        startChar = "(";
        serialIntfBaud = 2400;
        this->my_serialIntf->begin(serialIntfBaud, SWSERIAL_8N1);
        get.raw.qpi = this->requestData("QPI");
        writeLog("QPI:\t\t%s (Length: %d)", get.raw.qpi, get.raw.qpi.length());
        if (abortAutoDetect || suspendSerial)
        {
            writeLog("Autodetect aborted");
            goto autodetect_done;
        }
        if (get.raw.qpi != "" && get.raw.qpi.substring(0, 2) == "PI")
        {
            writeLog("<Autodetect> Match protocol: PI3X");
            delimiter = " ";
            protocol = PI30;
            break;
        }
        startChar = "^Dxxx";
        this->my_serialIntf->begin(serialIntfBaud, SWSERIAL_8N1);
        get.raw.qpi = this->requestData("^P005PI");
        writeLog("^P005PI:\t\t%s (Length: %d)", get.raw.qpi, get.raw.qpi.length());
        if (abortAutoDetect || suspendSerial)
        {
            writeLog("Autodetect aborted");
            goto autodetect_done;
        }
        if (get.raw.qpi != "" && get.raw.qpi == "18")
        {
            writeLog("<Autodetect> Match protocol: PI18");
            delimiter = ",";
            protocol = PI18;
            break;
        }
        this->my_serialIntf->end();
    }
    this->my_serialIntf->end();
    if (protocol == NoD && !abortAutoDetect && !suspendSerial)
    {
        modbus = new MODBUS(this->my_serialIntf);
        if (modbus != nullptr)
        {
            modbus->Init();
            protocol = modbus->autoDetect();
            if (protocol == NoD)
            {
                delete modbus;
                modbus = nullptr;
            }
        }
    } 
    writeLog("----------------- End Autodetect -----------------");
autodetect_done:
    if (busyCount > 0)
    {
        busyCount--;
    }
}

bool PI_Serial::sendCustomCommand()
{
    if (customCommandBuffer == "")
        return false;

    if (isModbus())
    {
        get.raw.commandAnswer = modbus->requestData(customCommandBuffer);
    }
    else
    {
        get.raw.commandAnswer = requestData(customCommandBuffer);
    }
    customCommandBuffer = "";
    return true;
}

String PI_Serial::requestData(String command)
{

    String commandBuffer = "";
    commandBuffer.reserve(128);
    uint16_t crcCalc = 0;
    uint16_t crcRecive = 0;

    busyCount++;
    this->my_serialIntf->write(command.c_str());
    this->my_serialIntf->write(highByte(getCRC(command)));
    this->my_serialIntf->write(lowByte(getCRC(command)));
    this->my_serialIntf->write(0x0D);
    this->my_serialIntf->flush();

    // for testing
    this->my_serialIntf->enableTx(true);
    delay(20);
    commandBuffer = this->my_serialIntf->readStringUntil('\r');
    this->my_serialIntf->enableTx(false);

    const size_t cbLen = commandBuffer.length();
    const char *cbBuf = commandBuffer.c_str();
    if (cbLen >= 3 &&
        getCRC(cbBuf, cbLen - 2) == 256U * (uint8_t)commandBuffer[cbLen - 2] + (uint8_t)commandBuffer[cbLen - 1] &&
        getCRC(cbBuf, cbLen - 2) != 0 && 256U * (uint8_t)commandBuffer[cbLen - 2] + (uint8_t)commandBuffer[cbLen - 1] != 0)
    {
        crcCalc = 256U * (uint8_t)commandBuffer[cbLen - 2] + (uint8_t)commandBuffer[cbLen - 1];
        crcRecive = getCRC(cbBuf, cbLen - 2);
        commandBuffer.remove(cbLen - 2); // remove the crc
        commandBuffer.remove(0, strlen(startChar));       // remove the start char ( for Pi30 and ^Dxxx for Pi18

        // requestOK++;
        connectionCounter = 0;
    }
    else if (cbLen >= 2 &&
             getCHK(cbBuf, cbLen - 1) + 1 == commandBuffer[cbLen - 1] &&
             getCHK(cbBuf, cbLen - 1) + 1 != 0 && commandBuffer[cbLen - 1] != 0 &&
             command == "QALL" // crude fix for the qall chk thing
             )                 // CHK for QALL
    {
        crcCalc = getCHK(cbBuf, cbLen - 1) + 1;
        crcRecive = commandBuffer[cbLen - 1];
        commandBuffer.remove(cbLen - 1); // remove the crc
        commandBuffer.remove(0, strlen(startChar));       // remove the start char ( for Pi30 and ^Dxxx for Pi18

        // requestOK++;
        connectionCounter = 0;
    }
    else if (commandBuffer == "NAK" ||
             (cbLen >= (strlen(startChar) + 3) &&
              memcmp(cbBuf + strlen(startChar), "NAK", 3) == 0)) // catch NAK without crc
    {
        commandBuffer = "NAK";
    }
    else if (commandBuffer == "") // catch empty answer, its similar to NAK
    {
        commandBuffer = "NOA";
    }
    else
    {
        writeLog("ERROR Send: >%s< Recive: >%s<", command, commandBuffer);
        connectionCounter++;
        commandBuffer = "ERCRC";
    }
    writeLog("[C: %6s][CR: %4X][CC: %4X][L: %3u]", command.c_str(), crcRecive, crcCalc, commandBuffer.length());
    if (busyCount > 0)
    {
        busyCount--;
    }
    if (requestSettlingTime > 0)
    {
        delay(requestSettlingTime);
    }
    return commandBuffer;
}

uint16_t cal_crc_half(uint8_t *pin, uint8_t len)
// https://forums.aeva.asn.au/viewtopic.php?t=4332&start=25
{
    uint16_t crc;
    uint8_t da;
    uint8_t *ptr;
    uint8_t bCRCHign;
    uint8_t bCRCLow;
    uint16_t crc_ta[16] =
        {
            0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50a5, 0x60c6, 0x70e7,
            0x8108, 0x9129, 0xa14a, 0xb16b, 0xc18c, 0xd1ad, 0xe1ce, 0xf1ef};
    ptr = pin;
    crc = 0;
    while (len-- != 0)
    {
        da = static_cast<uint8_t>((crc >> 8)) >> 4;
        crc <<= 4;
        crc ^= crc_ta[da ^ (*ptr >> 4)];
        da = static_cast<uint8_t>((crc >> 8)) >> 4;
        crc <<= 4;
        crc ^= crc_ta[da ^ (*ptr & 0x0f)];
        ptr++;
    }
    bCRCLow = crc;
    bCRCHign = static_cast<uint8_t>(crc >> 8);
    if (bCRCLow == 0x28 || bCRCLow == 0x0d || bCRCLow == 0x0a)
    {
        bCRCLow++;
    }
    if (bCRCHign == 0x28 || bCRCHign == 0x0d || bCRCHign == 0x0a)
    {
        bCRCHign++;
    }
    crc = static_cast<uint16_t>(bCRCHign) << 8;
    crc += bCRCLow;
    return crc;
}

uint16_t PI_Serial::getCRC(String data) // get a calculated crc from a string
{
    return getCRC(data.c_str(), data.length());
}

byte PI_Serial::getCHK(String data) // get a calculatedt CHK
{
    return getCHK(data.c_str(), data.length());
}

uint16_t PI_Serial::getCRC(const char *data, size_t len) // get a calculated crc from a buffer
{
    crc.reset();
    crc.setPolynome(0x1021);
    if (data && len)
    {
        crc.add((uint8_t *)data, len);
    }
    crc.calc();
    uint8_t CRCLow = lowByte(crc.calc());
    uint8_t CRCHigh = highByte(crc.calc());
    uint16_t CRCre;
    if (CRCLow == 0x28 || CRCLow == 0x0d || CRCLow == 0x0a)
    {
        CRCLow++;
    }
    if (CRCHigh == 0x28 || CRCHigh == 0x0d || CRCHigh == 0x0a)
    {
        CRCHigh++;
    }
    CRCre = ((uint16_t)CRCHigh) << 8;
    CRCre += CRCLow;
    return (CRCre);
}

byte PI_Serial::getCHK(const char *data, size_t len) // get a calculatedt CHK from a buffer
{
    byte chk = 0;
    if (!data || len == 0)
    {
        return chk;
    }
    for (size_t i = 0; i < len; i++)
    {
        chk += data[i];
    }
    return chk;
}

char *PI_Serial::getModeDesc(char mode) // get the char from QMOD and make readable things
{
    char *modeString;
    switch (mode)
    {
    default:
        modeString = (char *)("");
        break;
    case 'P':
        modeString = (char *)"Power On";
        break;
    case 'S':
        modeString = (char *)"Standby";
        break;
    case 'Y':
        modeString = (char *)"Bypass";
        break;
    case 'L':
        modeString = (char *)"Line";
        break;
    case 'B':
        modeString = (char *)"Battery";
        break;
    case 'T':
        modeString = (char *)"Battery Test";
        break;
    case 'F':
        modeString = (char *)"Fault";
        break;
    case 'D':
        modeString = (char *)"Shutdown";
        break;
    case 'G':
        modeString = (char *)"Grid";
        break;
    case 'C':
        modeString = (char *)"Charge";
        break;
    }
    return modeString;
}

bool PI_Serial::isModbus()
{
    return protocol == MODBUS_MUST || protocol == MODBUS_DEYE || protocol == MODBUS_ANENJI;
}

bool PI_Serial::checkQFLAG(const String& flags, char symbol) {
    bool enabled = false;
    for (unsigned int i = 0; i < flags.length(); i++) {
        char c = flags.charAt(i);
        if (c == 'E') enabled = true;
        else if (c == 'D') enabled = false;
        else if (c == symbol) return enabled;
    }
    return false;
}
