#ifndef SERVICES_H
#define SERVICES_H

#include <stdint.h>
#include <stdbool.h>

typedef enum
{
   UnlockedL1 =0x01,
   UnlockedL2 =0x03,
   UnlockedL3 =0x05,
   UnlockedL4 =0x07
}UDS_SuppLevel;
/* Define an enumeration type for system states */
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

/* Reqired data for diagnostic control */
#define 	NULL_PTR 				((void *)0)
#define     MIN_DIA_SESSION_LENGTH  0x02

/* Required data for ECU_Reset */
#define  MIN_ECURE_SESSION_LENGTH  0x01
#define MAX_SE_SIZE 1U


/* Define enumeration type for SUbfunctionType */
typedef enum {
    DEFAULT_SESSION     = 0x01,
    PROGRAMMING_SESSION = 0x02,
    EXTENDED_SESSION    = 0X03
} SubFunctionType;

/* Define enumeration type for FrameType */
typedef enum {
    SINGLE_FRAME = 1,
    MULTI_FRAME  = 2
} FrameType;

typedef struct {
    uint8_t data_length;
    ServiceState sid;
    SubFunctionType subfunction;
    uint8_t sequence_number;
    uint8_t data[4096]; // Buffer for multi-frame data (adjust size as needed)
} ProcessedFrame;

/* Function pointer type definition for UDS service handlers */
typedef uint32_t (*ServiceHandler)(ProcessedFrame frame);
void prepare_response(uint8_t *response_data, uint32_t response_code, ServiceState sid, SubFunctionType subfunction);

/* Define a structure to map services to sessions and functions */
typedef struct {
    ServiceState service_id;              // The service ID (e.g., DIAGNOSTIC_SESSION_CONTROL)
    uint8_t min_data_length;              // Minimum required data length for the service
    uint32_t (*service_handler)(void*);   // Pointer to the service handler function
    uint8_t allowed_sessions;             // Sessions allowed for this service
    uint8_t auth;                         // Authentication
    uint8_t secSupp;                       // Security suppression
    UDS_SuppLevel security_level[MAX_SE_SIZE];          // Security level
    
} UdsServiceMap;

/* Declare the UDS Service Table */
typedef struct {
    UdsServiceMap row[1];
    uint8_t size;
}UDS_Table;
/* Declare the size of the UDS Service Table */
size_t getUDSTable(UDS_Table *tableReference);
uint32_t diagnosticControl(void* frame);

#endif // SERVICES_H
