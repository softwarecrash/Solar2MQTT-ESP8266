#ifndef SRC_MODBUS_REGISTERS_H_
#define SRC_MODBUS_REGISTERS_H_
#include "Arduino.h"
#include <ArduinoJson.h>

typedef enum
{
    MODBUS_TYPE_HOLDING = 0x00, /*!< Modbus Holding register. */
    //    MODBUS_TYPE_INPUT,                  /*!< Modbus Input register. */
    //    MODBUS_TYPE_COIL,                   /*!< Modbus Coils. */
    //    MODBUS_TYPE_DISCRETE,               /*!< Modbus Discrete bits. */
    //    MODBUS_TYPE_COUNT,
    //    MODBUS_TYPE_UNKNOWN = 0xFF
} modbus_entity_t;

typedef enum
{
    //    REGISTER_TYPE_U8 = 0x00,                   /*!< Unsigned 8 */
    REGISTER_TYPE_U16,             /*!< Unsigned 16 */
    REGISTER_TYPE_INT16,           /*!< Signed 16 */
    REGISTER_TYPE_U32,             /*!< Unsigned 32 */
    REGISTER_TYPE_U32_ONE_DECIMAL, /*!< Unsigned 32 multiply 0.1*/ 
    REGISTER_TYPE_ASCII,           /*!< ASCII type */
    REGISTER_TYPE_DIEMATIC_ONE_DECIMAL,
    REGISTER_TYPE_DIEMATIC_TWO_DECIMAL,
    REGISTER_TYPE_BITFIELD,
    REGISTER_TYPE_DEBUG,
    REGISTER_TYPE_CUSTOM_VAL_NAME,
    REGISTER_TYPE_CALLBACK,         /*call custom callback after register read*/
    REGISTER_TYPE_VIRTUAL_CALLBACK, /*allows to call callback without register read*/
} register_type_t;



typedef union
{
    const char *bitfield[16];
} optional_param_t;
 
class MODBUS_COM;

typedef void (*modbus_callback_t)( JsonObject *variant, uint16_t *registerValue, const struct modbus_register_t *reg, MODBUS_COM &mCom); // Define a function pointer type for the callback

typedef struct modbus_register_t 
{
    uint16_t id;
    modbus_entity_t modbus_entity; /*!< Type of modbus parameter */
    register_type_t type;          /*!< Float, U8, U16, U32, ASCII, etc. */
    const char *name;
    int16_t offset = 0;
    optional_param_t optional_param;
    modbus_callback_t callback; 
} modbus_register_t;


typedef struct
{
    JsonObject *variant;
    const modbus_register_t *registers;
    size_t array_size;
    size_t curr_register;
} modbus_register_info_t;

#endif // SRC_MODBUS_REGISTERS_H_