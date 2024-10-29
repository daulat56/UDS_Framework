#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if.h>
#include <linux/can.h>
#include <linux/can/raw.h>
#include "../../server.h"
#include "digSessControl.h"


void defaultSession(void) {
    printf("Entering Default Session\n");
    // Implement the logic for entering default session
    //updateSession(DEFAULT_SESSION);
    // You might want to reset certain parameters or states here
}

void programmingSession(void) {
    printf("Entering Programming Session\n");
    // Implement the logic for entering programming session
    //updateSession(PROGRAMMING_SESSION);
    // You might want to prepare the system for programming here
    // For example, unlocking certain memory areas
}

void extendedSession(void) {
    printf("Entering Extended Session\n");
    // Implement the logic for entering extended session
    //updateSession(EXTENDED_SESSION);
    // You might want to enable certain extended features here
}
/* Diagnostic Session Control service function */
struct can_frame diagnosticControl(void *frame)
{
    ProcessedFrame *processedFrame = (ProcessedFrame *)frame;
    ServiceState sid = processedFrame->sid;
    SubFunctionType subfunction = processedFrame->subfunction;
    uint8_t data_length = processedFrame->data_length;
    printf("the data length is %d\n",data_length);
    printf("DIAGNOSTIC SESSION CONTROL: 0x%02X\n",sid);
    printf("the subfubction type : 0x%02x\n",subfunction);
    return handleUDSRequest(DIAGNOSTIC_SESSION_CONTROL,subfunction,data_length);
    /*
    UDS_Table udsServiceTable = {};
    getUDSTable(&udsServiceTable);
    // Define the response frame
    struct can_frame response_frame;
    memset(&response_frame, 0, sizeof(response_frame));

    response_frame.can_id = 0x123;  // Replace with your CAN ID
    response_frame.can_dlc = RESPONSE_LENGTH;

    // Set the appropriate response code
    uint32_t response_code = 0x00;  // Default positive response

    printf("DIAGNOSIS_CONTROL : 0x%02X\n", sid);
    printf("subfunction extracted :%x\n", subfunction);

    if (data_length < 0x02) {
        response_code = 0x13;  // NRC: Incorrect message length or invalid format
    }else if (!isSessionAllowed(DIAGNOSTIC_SESSION_CONTROL, subfunction)) {
        response_code = 0x7F;  // NRC: Service not supported in current session
    }
    else {
        switch (subfunction) {
            case DEFAULT_SESSION:
                printf("DEFAULT SESSION: 0x02%X\n",DEFAULT_SESSION);
              
                break;
                
            case PROGRAMMING_SESSION:
                printf("PROGRAMMING SESSION: 0x02%X\n", PROGRAMMING_SESSION);
        
                break;
            case EXTENDED_SESSION:
                // Implement the reset logic here
                printf("EXTENDED SESSION: 0x02%X\n", EXTENDED_SESSION);
                // For demonstration, we'll just simulate a successful reset
                break;
            default:
                printf("SESSION IS NOT THERE:0x12");
                response_code = 0x12;  // NRC: Subfunction not supported
                break;
        }
    }

    // Prepare the response
    if (response_code == 0x00) {
        response_frame.data[0] = 0x03;  // Response length
        response_frame.data[1] = sid + 0x40;  // Positive response SID
        response_frame.data[2] = subfunction;  // Echo the reset type
        response_frame.data[3] = (g_session_info.p2_server_max >> 8) & 0xFF;
        response_frame.data[4] = g_session_info.p2_server_max & 0xFF;
        response_frame.data[5] = (g_session_info.p2_star_server_max >> 8) & 0xFF;
        response_frame.data[6] = g_session_info.p2_star_server_max & 0xFF;
        response_frame.can_dlc = 7;
        // Update the session if the response is positive
        updateSession(subfunction);
    } else {
        response_frame.data[0] = 0x03;  // Response length
        response_frame.data[1] = 0x7F;  // Negative response
        response_frame.data[2] = sid;   // Service ID
        response_frame.data[3] = response_code;  // NRC
        response_frame.can_dlc = 4;  // Update length for negative response
    }
*/

   /*
    if (data_length < 0x02)
    {
        response_frame.data[0] = data_length;  // Negative response indication
        response_frame.data[1] = 0x7F;    // SID
        response_frame.data[2] = response_code; // NRC
        response_code = 0x13; // NRC: Incorrect message length or invalid format
    }
    else if (processedFrame->subfunction == udsServiceTable.row[0].allowed_sessions)
    //else if (isSessionAllowed(DIAGNOSTIC_SESSION_CONTROL, subfunction))
    {
        if (authentication())
        {
            printf("the subfunction i have added is :%d\n", subfunction);
            switch (subfunction)
            {
            case DEFAULT_SESSION:
                printf("came into the dwfault session");
                break;
            
            case PROGRAMMING_SESSION:
                printf("Came to programming session");
                break;
            
            case EXTENDED_SESSION:
                printf("came to extended session");
                break;
            
            default:
                break;
            }
            // Update the session
            //updateSession((SubFunctionType)subfunction);
            // Set positive response in CAN fram
            if(response_code==0x00){
                response_frame.data[0] = data_length;  // Positive response code for Diagnostic Session Control
                response_frame.data[1] = sid+0x40;   // SID
                response_frame.data[2] = subfunction; // Subfunction
                response_frame.data[3] = (g_session_info.p2_server_max >> 8) & 0xFF;
                response_frame.data[4] = g_session_info.p2_server_max & 0xFF;
                response_frame.data[5] = (g_session_info.p2_star_server_max >> 8) & 0xFF;
                response_frame.data[6] = g_session_info.p2_star_server_max & 0xFF;
                response_frame.can_dlc = 7;
                printf("Ready to send positive response:\n");
            }
        }
        else
        {
            response_frame.data[0] = data_length;  // Negative response indication
            response_frame.data[1] = 0x7F;    // SID
            response_frame.data[2] = response_code; // NRC
            response_code = 0x33; // NRC: Security access denied
        }
    }
    else
    {
        response_frame.data[0] = data_length;  // Negative response indication
        response_frame.data[1] = 0x7F;    // SID
        response_frame.data[2] = response_code; // NRC
        response_code = 0x12; // NRC: Subfunction not supported
    }

    // Set response frame data based on response code
    if (response_code != 0x00)
    {
        response_frame.data[0] = data_length;  // Negative response indication
        response_frame.data[1] = 0x7F;    // SID
        response_frame.data[2] = response_code; // NRC
    }

    */
/*
    // Print CAN frame for debugging
    printf("Sending CAN Frame:\n");
    printf("Can ID: 0x%03X\n", response_frame.can_id);
    printf("Can Data Length: %d\n", response_frame.can_dlc);
    printf("Can Data: ");
    for (int i = 0; i < response_frame.can_dlc; i++)
    {
        printf("%02X ", response_frame.data[i]);
    }
    printf("\n");

    return response_frame;
*/
}






