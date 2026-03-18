#ifndef MODBUS_DEVICE_H
#define MODBUS_DEVICE_H

#include <SoftwareSerial.h> 
#include <main.h>
#include <modbus/modbus_registers.h>
#include <modbus/modbus_com.h>
 

class ModbusDevice
{
public:
    ModbusDevice(long baudRate, uint32_t modbusAddr, protocol_type_t protocol);

    // Pure virtual functions - these need to be implemented in derived classes
    virtual const modbus_register_t *getLiveRegisters() const = 0;
    virtual const modbus_register_t *getStaticRegisters() const = 0;

    virtual long getBaudRate() const;
    virtual long getModbusAddr() const;
    virtual protocol_type_t getProtocol() const;

    // Pure virtual function for getting the name
    virtual const char *getName() const = 0;

    virtual void init(SoftwareSerial &serial, MODBUS_COM &mCom);

    virtual bool retrieveModel(MODBUS_COM &mCom, char *modelBuffer, size_t bufferSize);

    // Pure virtual methods to count registers
    virtual size_t getLiveRegistersCount() const = 0;
    virtual size_t getStaticRegistersCount() const = 0;

    virtual ~ModbusDevice();

protected:
    long _baudRate;
    uint32_t _modbusAddr;
    protocol_type_t _protocol;
};


#endif