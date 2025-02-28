#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include "../../server.h"
#include "ecuReset.h"

#define ECU_RESET_MIN_LENGTH 2

void hardReset(void) {
    printf("Performing Hard Reset\n");
    // Implement the hard reset logic here
    // This might involve restarting the entire system
    // For example:
    // system("reboot");  // This is just an example, actual implementation may vary
}

void keyOffOnReset(void) {
    printf("Performing Key Off On Reset\n");
    // Implement the key off on reset logic here
    // This might involve simulating a key cycle
    // For example:
    // turnOffSystems();
    // sleep(1);  // Wait for 1 second
    // turnOnSystems();
}

void softReset(void) {
    printf("Performing Soft Reset\n");
    // Implement the soft reset logic here
    // This might involve restarting certain processes or reinitializing some systems
    // For example:
    // reinitializeSubsystems();
}

struct can_frame ecuReset(void *frame) {
    printf("ecu reset is called");
    ProcessedFrame *processedFrame = (ProcessedFrame *)frame;
    ServiceState sid = processedFrame->sid;
    ECUResetType resetType = (ECUResetType)processedFrame->subfunction;
    uint8_t data_length = processedFrame->data_length;

    printf("ECU_RESET : 0x%02X\n", sid);
    printf("Reset type: 0x%02X\n", resetType);

    return handleUDSRequest(ECU_RESET, resetType, data_length);

    /*
    UDS_Table udsServiceTable = {};
    getUDSTable(&udsServiceTable);
    // Define the response frame
    struct can_frame response_frame;
    memset(&response_frame, 0, sizeof(response_frame));

    response_frame.can_id = 0x123;  // Replace with your response CAN ID
    response_frame.can_dlc = RESPONSE_LENGTH;  // Minimum response length

    uint32_t response_code = 0x00;  // Default positive response

    printf("ECU_RESET : 0x%02X\n", sid);
    printf("Reset type: 0x%02X\n", resetType);

    if (data_length < ECU_RESET_MIN_LENGTH) {
        response_code = 0x13;  // NRC: Incorrect message length or invalid format
    }
    else if (!isSessionAllowed(ECU_RESET, resetType)) {
        response_code = 0x7F;  // NRC: Service not supported in current session
    } else {
        switch (resetType) {
            case HARD_RESET:
                printf("HARD RESET: %x\n", HARD_RESET);
                
                break;
                
            case KEY_OFF_ON_RESET:
                printf("KEY_OFF_ON: %x\n",KEY_OFF_ON_RESET);
               
                break;
            case SOFT_RESET:
                // Implement the reset logic here
                printf("SOFT_RESET: %X\n", SOFT_RESET);
                // For demonstration, we'll just simulate a successful reset
                break;
            default:
                response_code = 0x12;  // NRC: Subfunction not supported
                break;
        }
    }

    // Prepare the response
    if (response_code == 0x00) {
        if(authentication)
        {
            
            printf("the current session is :%X\n", g_session_info.current_session);
            //bool isServiceAllowedInCurrSess=udsServiceTable.row[i].allowed_sessions && (1 << (session - 1))) != 0
            //if(g_session_info.current_session==udsServiceTable.row[1].allowed_sessions)
            if(isSessionAllowedInService(ECU_RESET,g_session_info.current_session))
            {
                response_frame.data[0] = 0x03;  // Response length
                response_frame.data[1] = sid + 0x40;  // Positive response SID
                response_frame.data[2] = resetType;  // Echo the reset type
            }
            else{
                response_frame.data[0] = 0x03;                  
                response_frame.data[1] = sid;  
                response_frame.data[2] = 0x7F;  
            }
        }
        else{
            response_frame.data[0] = 0x03;  // Response length
            response_frame.data[1] = 0x7F;  // Negative response
            response_frame.data[2] = sid;   // Service ID
            response_frame.data[3] = 0x34;  // NRC
        }
    } else {
        response_frame.data[0] = 0x03;  // Response length
        response_frame.data[1] = 0x7F;  // Negative response
        response_frame.data[2] = sid;   // Service ID
        response_frame.data[3] = response_code;  // NRC
        response_frame.can_dlc = 4;  // Update length for negative response
    }

    // Print CAN frame for debugging
    printf("Sending CAN Frame:\n");
    printf("Can ID: 0x%03X\n", response_frame.can_id);
    printf("Can Data Length: %d\n", response_frame.can_dlc);
    printf("Can Data: ");
    for (int i = 0; i < response_frame.can_dlc; i++) {
        printf("%02X ", response_frame.data[i]);
    }
    printf("\n");

    
    //eturn response_frame;
    */
}