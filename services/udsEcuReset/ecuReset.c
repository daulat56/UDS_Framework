#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include "../../server.h"
#include "ecuReset.h"

#define ECU_RESET_MIN_LENGTH 2

uint32_t ecuReset(void *frame) {
    printf("ecu reset is called");
    ProcessedFrame *processedFrame = (ProcessedFrame *)frame;
    ServiceState sid = processedFrame->sid;
    ECUResetType resetType = (ECUResetType)processedFrame->subfunction;
    uint8_t data_length = processedFrame->data_length;
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
    }else if (!isSessionAllowed(ECU_RESET, g_session_info.current_session)) {
        response_code = 0x7F;  // NRC: Service not supported in current session
    }
    else {
        switch (resetType) {
            case HARD_RESET:
                printf("Hard reset requested\n");
                /*call the users api*/
                break;
                
            case KEY_OFF_ON_RESET:
                printf("key off on is implemented");
                /*call users api*/
                break;
            case SOFT_RESET:
                // Implement the reset logic here
                printf("Performing ECU reset type: 0x%02X\n", resetType);
                // For demonstration, we'll just simulate a successful reset
                break;
            default:
                response_code = 0x12;  // NRC: Subfunction not supported
                break;
        }
    }

    // Prepare the response
    if (response_code == 0x00) {
        response_frame.data[0] = 0x03;  // Response length
        response_frame.data[1] = sid + 0x40;  // Positive response SID
        response_frame.data[2] = resetType;  // Echo the reset type
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

    // Send the response frame
    int sockfd = getSocket();
    if (write(sockfd, &response_frame, sizeof(response_frame)) < 0) {
        perror("write");
        return 0xFF;  // Return error code if unable to send response
    } else {
        printf("ECU Reset response sent successfully\n");
    }

    return response_code;
}