#ifndef SERVER_H
#define SERVER_H

#include <stdint.h>
#include <linux/can.h>
//#include "services.h"
#include "services/udsDigCon/digSessControl.h"
#include "services/udsEcuReset/ecuReset.h"

// Define CAN_INTERFACE as a preprocessor macro if needed across files
#define CAN_INTERFACE "vcan0"  // Replace with your CAN interface name

#define ECU_RESET_MIN_LENGTH 2
#define MIN_DIA_SESSION_LENGTH 2
#define RESPONSE_LENGTH 8
#define P2_SERVER_MAX 65535 // Example value in milliseconds
#define P2_STAR_SERVER_MAX 655350 // Example value in milliseconds
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
    WRITE_DATA_BY_IDENTIFIER   = 0x2E,
    WRITE_MEMORY_BY_ADDRESS    = 0X3D,
    ROUTINE_CONTROL            = 0x31,
    REQUEST_DOWNLOAD           = 0x34,
    REQUEST_UPLOAD             = 0x35,
    TRANSFER_DATA              = 0x36,
    REQUEST_TRANSFER_EXIT      = 0X37,
    REQUEST_FILE_TRANSFER      = 0x38  
} ServiceState;

/* Reqired data for diagnostic control */
#define 	NULL_PTR 				((void *)0)
//#define     MIN_DIA_SESSION_LENGTH  0x02

/* Required data for ECU_Reset */
#define MAX_SE_SIZE 1U


/* Define enumeration type for SUbfunctionType */
typedef enum {
    DEFAULT_SESSION     = 0x01,
    PROGRAMMING_SESSION = 0x02,
    EXTENDED_SESSION    = 0X03
} SubFunctionType;

typedef struct {
    SubFunctionType current_session;
    uint32_t p2_server_max;
    uint32_t p2_star_server_max;
} SessionInfo;
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
    UdsServiceMap row[2];
    uint8_t size;
}UDS_Table;



/* Function pointer type definition for UDS service handlers */
typedef uint32_t (*ServiceHandler)(ProcessedFrame frame);
void prepare_response(uint8_t *response_data, uint32_t response_code, ServiceState sid, SubFunctionType subfunction);

/* Declare the size of the UDS Service Table */
//size_t getUDSTable(UDS_Table *tableReference);
// Change the UdsTable declaration to an external declaration
extern UDS_Table udsTable;
extern SessionInfo g_session_info;
void initializeSession(void);
void updateSession(SubFunctionType new_session);
bool isSessionAllowed(ServiceState service, SubFunctionType session);
// Change the getUDSTable function to a declaration
size_t getUDSTable(UDS_Table *tableReference);
bool authentication(void);
// Function declarations
void send_can_message(int sockfd, uint32_t can_id, uint8_t *data);
FrameType identify_frame_type(struct can_frame *frame);
void extract_first_frame_data(struct can_frame *frame, uint16_t *total_length, uint8_t *SID, uint8_t *subfunction);
void extract_consecutive_frame_data(struct can_frame *frame, uint8_t *data_buffer, uint8_t sequence_number, int *data_index);
ProcessedFrame processCanFrame(struct can_frame *frame);
void sendPossRes(int sockfd, ServiceState sid, struct can_frame *processedFrame);//void findService(uint8_t SID);
int getSocket() ;
#endif // SERVER_H
