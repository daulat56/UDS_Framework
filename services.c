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
#include "services.h"
#include "server.h"

/* Authentication function */
bool authentication()
{
    /* Need to implement authentication logic here */
    return true;
}

#define MIN_DIA_SESSION_LENGTH 2
#define MIN_ECURE_SESSION_LENGTH 2

/*Define the UDS Service Table */
UDS_Table udsTable = {.row = {
                          {.service_id = DIAGNOSTIC_SESSION_CONTROL, .min_data_length = MIN_DIA_SESSION_LENGTH, .service_handler = &diagnosticControl, .allowed_sessions = (DEFAULT_SESSION || PROGRAMMING_SESSION || EXTENDED_SESSION), .auth = true, .secSupp = 1, .security_level = {UnlockedL1}}},
                      .size = 1};

size_t getUDSTable(UDS_Table *tableReference)
{
    printf("the size is %d and sid is : %x\n", udsTable.size, udsTable.row->service_id);
    memcpy(tableReference, &udsTable, sizeof(UDS_Table));
}

#define RESPONSE_LENGTH 8

/* Diagnostic Session Control service function */
uint32_t diagnosticControl(void *frame)
{
    ProcessedFrame *processedFrame = (ProcessedFrame *)frame;
    ServiceState sid = processedFrame->sid;
    SubFunctionType subfunction = processedFrame->subfunction;
    uint8_t data_length = processedFrame->data_length;
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

    if (data_length < 0x02)
    {
        response_frame.data[0] = data_length;  // Negative response indication
        response_frame.data[1] = 0x7F;    // SID
        response_frame.data[2] = response_code; // NRC
        response_code = 0x13; // NRC: Incorrect message length or invalid format
    }
    else if (subfunction == udsServiceTable.row[0].allowed_sessions)
    {
        if (authentication())
        {
            // Set positive response in CAN frame
            response_frame.data[0] = data_length;  // Positive response code for Diagnostic Session Control
            response_frame.data[1] = sid+0x40;   // SID
            response_frame.data[2] = subfunction; // Subfunction
            printf("Ready to send positive response:\n");
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

    // Send the response frame
    int sockfd = getSocket();  // Replace with your function to get the socket descriptor
    if (write(sockfd, &response_frame, sizeof(response_frame)) < 0)
    {
        perror("write");
        return 0xFF; // Return error code if unable to send response
    }
    else
    {
        printf("Positive response sent successfully\n");
    }

    return response_code;
}






