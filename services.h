#ifndef STATUS_H
#define STATUS_H
#include <stdint.h>
#include <stdbool.h>
// Define an enumeration type for system states
typedef enum {
    DIAGNOSTIC_SESSION_CONTROL = 0x10,
    ECU_RESET                  = 0x11,
    SECURITY_ACCESS            = 0x27,
    TESTER_PRESENT             = 0x3E,
    READ_DATA_BY_IDENTIFIER    = 0x22,
    READ_MEMORY_BY_ADDRESS     = 0x23,
    WRITE_DATA_BY_IDETIFIER    = 0x2E,
    WRITE_DATA_BY_IDENTIFIER   = 0X3D,
    ROUTINE_CONTROL            = 0x31,
    REQUEST_DOWNLOAD           = 0x34,
    REQUEST_UPLOAD             = 0x35,
    TRANSFER_DATA              = 0x36,
    REQUEST_TRANSFER_EXIT      = 0X37,
    REQUEST_FILE_TRANSFER      = 0x38  
} ServiceState;

typedef enum{
    DEFAULT_SESSION     = 0x01,
    PROGRAMMING_SESSION = 0x02,
    EXTENDED_SESSION  = 0X03
}SubFunctionType;

typedef enum
{
	SINGLE_FRAME = 1,
	MULTI_FRAME  = 2
}FrameType; 

//to handle all the UDS services

// Function pointer type definition
typedef uint32_t (*ServiceHandler)(ServiceState sid, SubFunctionType subfunction, uint8_t data_length);

// Define a structure to map services to sessions and callbacks
typedef struct {
    ServiceState service;
    uint8_t SubFunctionType;
    ServiceHandler handler;
} UdsServiceMap;

// Function prototypes for services
uint32_t diagnosticControl(ServiceState sid, SubFunctionType subfunction, uint8_t data_length);

// UDS Service Table
const UdsServiceMap UdsServiceTable[] = {
    {DIAGNOSTIC_SESSION_CONTROL, DEFAULT_SESSION | PROGRAMMING_SESSION | EXTENDED_SESSION, diagnosticControl},

 /* 
    {ECU_RESET,                  DEFAULT_SESS | PROGRAMMING_SESS | EXTENDED_SESS, handleEcuReset},
    {SECURITY_ACCESS,            DEFAULT_SESS | PROGRAMMING_SESS | EXTENDED_SESS, NULL_PTR},
    {TESTER_PRESENT,             DEFAULT_SESS | PROGRAMMING_SESS | EXTENDED_SESS, NULL_PTR},
    {READ_DATA_BY_IDENTIFIER,    DEFAULT_SESS | PROGRAMMING_SESS | EXTENDED_SESS, NULL_PTR},
    // Add other services here
*/  
};


#endif // STATUS_H
