#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if.h>
#include <linux/can.h>
#include <linux/can/raw.h>
// #include "services.h"
#include "server.h"
// #include "digSessControl.h"
#include "services/udsDigCon/digSessControl.h"
#include "services/udsEcuReset/ecuReset.h"

#define CAN_INTERFACE "vcan0" // Replace with your CAN interface name

// Declare subfunction handlers
void hardReset(void);
void keyOffOnReset(void);
void softReset(void);
void defaultSession(void);
void programmingSession(void);
void extendedSession(void);

// Define subfunction tables for each service
SubFunctionEntry ecuResetSubfunctions[] = {
    {HARD_RESET, {DEFAULT_SESSION, EXTENDED_SESSION}, true, 1, {UnlockedL1}, hardReset},
    {KEY_OFF_ON_RESET, {EXTENDED_SESSION}, true, 1, {UnlockedL1}, keyOffOnReset},
    {SOFT_RESET, {DEFAULT_SESSION, EXTENDED_SESSION}, true, 1, {UnlockedL1}, softReset}};

SubFunctionEntry diagSessionControlSubfunctions[] = {
    {DEFAULT_SESSION, {DEFAULT_SESSION, PROGRAMMING_SESSION, EXTENDED_SESSION}, true, 1, {UnlockedL1}, defaultSession},
    {PROGRAMMING_SESSION, {DEFAULT_SESSION, PROGRAMMING_SESSION}, true, 1, {UnlockedL1}, programmingSession},
    {EXTENDED_SESSION, {DEFAULT_SESSION, EXTENDED_SESSION}, true, 1, {UnlockedL1}, extendedSession}};

/* Function to send a negative response */
struct can_frame sendNegativeResponse(uint8_t original_sid, uint8_t error_code)
{
    struct can_frame response_frame;

    // Prepare the response CAN frame (using standard UDS negative response format)
    response_frame.can_id = 0x7DF;         // Change to appropriate ID if needed
    response_frame.can_dlc = 3;            // Response length
    response_frame.data[0] = 0x7F;         // Negative response identifier
    response_frame.data[1] = original_sid; // Original SID that caused the error
    response_frame.data[2] = error_code;   // Error code, e.g., 0x11 for "Service Not Supported"

    // Send the frame (assuming socket is already set up)
    int sockfd = getSocket();
    if (sockfd >= 0)
    {
        if (write(sockfd, &response_frame, sizeof(struct can_frame)) != sizeof(struct can_frame))
        {
            perror("Error sending negative response frame");
        }
        else
        {
            printf("Negative response sent: 0x7F 0x%02X 0x%02X\n", original_sid, error_code);
        }
    }
    return response_frame;
}

/* Authentication function */
bool authentication()
{
    /* Need to implement authentication logic here */
    return true;
}
/*Define the UDS Service Table */
UDS_Table udsTable = {.row = {
                          {.service_id = DIAGNOSTIC_SESSION_CONTROL,
                           .min_data_length = MIN_DIA_SESSION_LENGTH,
                           .service_handler = &diagnosticControl,
                           .allowed_sessions = {DEFAULT_SESSION, PROGRAMMING_SESSION, EXTENDED_SESSION},
                           .auth = true,
                           .secSupp = 1,
                           .security_level = {UnlockedL1},
                           .subfunctions = diagSessionControlSubfunctions,
                           .num_subfunctions = sizeof(diagSessionControlSubfunctions) / sizeof(SubFunctionEntry)},
                          {.service_id = ECU_RESET,
                           .min_data_length = ECU_RESET_MIN_LENGTH,
                           .service_handler = &ecuReset,
                           .allowed_sessions = {DEFAULT_SESSION, EXTENDED_SESSION},
                           .auth = true,
                           .secSupp = 1,
                           .security_level = {UnlockedL1},
                           .subfunctions = ecuResetSubfunctions,
                           .num_subfunctions = sizeof(ecuResetSubfunctions) / sizeof(SubFunctionEntry)}},
                      .size = 2};

size_t getUDSTable(UDS_Table *tableReference)
{
    // printf("the size is %d and sid is : %x\n", udsTable.size, udsTable.row->service_id);
    memcpy(tableReference, &udsTable, sizeof(UDS_Table));
}

FrameType identifyFrameType(struct can_frame *frame)
{
    // Print the first byte from the data array
    printf("identify frame type is called with first byte as 0x%02X\n", frame->data[0]);

    // Extract PCI type
    uint8_t pci_type = (frame->data[0] & 0xF0) >> 4;

    // Print the PCI type
    printf("PCI Type: %X\n", pci_type);

    if (pci_type == 0)
    {
        // Single frame
        return SINGLE_FRAME;
    }
    else if (pci_type == 1)
    {
        // First frame of multi-frame message
        return MULTI_FRAME;
    }
    else
    {
        // Handle other cases (not implemented in this example)
        return -1; // Default case for unknown types
    }
}

/* Function to extract data from the first frame of a multi-frame message */
void extract_first_frame_data(struct can_frame *frame, uint16_t *total_length, uint8_t *SID, uint8_t *subfunction)
{
    *total_length = ((frame->data[0] & 0x0F) << 8) | frame->data[1]; // Extract total length
    *SID = frame->data[2];                                           // Extract SID
    *subfunction = frame->data[3];                                   // Extract subfunction
}

/* Function to handle consecutive frames */
void extract_consecutive_frame_data(struct can_frame *frame, uint8_t *data_buffer, uint8_t sequence_number, int *data_index)
{

    /*Copy the data from the consecutive frame into the data buffer */
    memcpy(&data_buffer[*data_index], &frame->data[1], 7);
    *data_index += 7;
}

/* Main processing function */
ProcessedFrame processCanFrame(struct can_frame *frame)
{
    ProcessedFrame result = {0}; // Initialize all fields to 0
    FrameType frame_type = identifyFrameType(frame);

    if (frame_type == SINGLE_FRAME)
    {
        result.sid = (ServiceState)frame->data[1];
        result.subfunction = (SubFunctionType)frame->data[2];
        result.data_length = frame->data[0] & 0x0F;
        result.sequence_number = 0;
        printf("Single Frame detected\n");
        printf("SID: 0x%02X, SubFunction: 0x%02X, Data Length: %d\n", result.sid, result.subfunction, result.data_length);
    }
    else if (frame_type == MULTI_FRAME)
    {

        printf("Multi-Frame detected\n");
        uint16_t total_length;
        uint8_t SID, subfunction;
        uint8_t data_buffer[4096]; // Adjust size as necessary
        int data_index = 0;

        if ((frame->data[0] & 0xF0) >> 4 == 0x01)
        {
            extract_first_frame_data(frame, &total_length, &SID, &subfunction);
            result.sid = (ServiceState)SID;
            result.subfunction = (SubFunctionType)subfunction;
            printf("First Frame detected\n");
            printf("Total Length: %d, SID: 0x%02X, SubFunction: 0x%02X\n", total_length, SID, subfunction);
            data_index = 6; // First 6 bytes are part of the first frame's data
        }
        else if ((frame->data[0] & 0xF0) >> 4 == 0x02)
        {
            result.sequence_number = frame->data[0] & 0x0F;
            extract_consecutive_frame_data(frame, data_buffer, result.sequence_number, &data_index);
            printf("Consecutive Frame detected with sequence number: %d\n", result.sequence_number);
        }
    }

    return result;
}

struct can_frame handleService(ProcessedFrame *processedFrame)
{
    struct can_frame response_code;
    UDS_Table udsServiceTable = {};
    getUDSTable(&udsServiceTable);
    // printf("size of table : %d\n",udsServiceTable.size);
    for (int i = 0; i < udsServiceTable.size; i++)
    {
        // printf("the SID is : %d \n",udsServiceTable.row[i].service_id );
        if (udsServiceTable.row[i].service_id == processedFrame->sid)
        {
            response_code = udsServiceTable.row[i].service_handler(processedFrame);
            // printf("Response Code: 0x%02X\n", response_code);
            return response_code;
        }
    }
    printf("Unknown or unsupported SID: 0x%02X\n", processedFrame->sid);
    sendNegativeResponse(processedFrame->sid, 0x11); // Send "Service Not Supported" response
}

struct can_frame handleUDSRequest(ServiceState sid, SubFunctionType subfunction, uint8_t data_length)
{
    struct can_frame response_frame = {0};
    ResponseCode response_code = RESPONSE_POSITIVE;
    UDS_Table udsServiceTable = {};
    getUDSTable(&udsServiceTable);

    // Check if SID is supported
    UdsServiceMap *service_row = NULL;
    for (int i = 0; i < udsServiceTable.size; i++)
    {
        if (udsServiceTable.row[i].service_id == sid)
        {
            service_row = &udsServiceTable.row[i];
            break;
        }
    }
    printf("the current session : %d\n", g_session_info.current_session);

    if (!service_row)
    {
        response_code = RESPONSE_NEGATIVE_SERVICE_NOT_SUPPORTED;
    }
    else if (!authentication())
    {
        response_code = RESPONSE_NEGATIVE_SECURITY_ACCESS_DENIED;
    }
    else if (!isSessionAllowed(sid, g_session_info.current_session))
    {
        response_code = RESPONSE_NEGATIVE_SERVICE_NOT_SUPPORTED_IN_ACTIVE_SESSION;
    }
    else if (data_length < service_row->min_data_length)
    {
        response_code = RESPONSE_NEGATIVE_INCORRECT_LENGTH;
    }
    else
    {
        // Check if subfunction is supported
        SubFunctionEntry *subfunction_entry = NULL;
        for (int i = 0; i < service_row->num_subfunctions; i++)
        {
            if (service_row->subfunctions[i].subfunction == subfunction)
            {
                subfunction_entry = &service_row->subfunctions[i];
                break;
            }
        }
        if (!subfunction_entry)
        {
            response_code = RESPONSE_NEGATIVE_SUBFUNCTION_NOT_SUPPORTED;
        }
        else if (!subfunction_entry->auth)
        {
            response_code = RESPONSE_NEGATIVE_SECURITY_ACCESS_DENIED;
        }
        else if (!isSubfunctionSupportedInSession(subfunction_entry, g_session_info.current_session))
        {
            response_code = RESPONSE_NEGATIVE_SUBFUNCTION_NOT_SUPPORTED_IN_ACTIVE_SESSION;
        }
        else
        {
            // Call the subfunction handler
            subfunction_entry->subfunction_handler();
        }
    }

    // Prepare the response frame
    response_frame.can_id = 0x123; // Replace with your response CAN ID

    if (response_code == RESPONSE_POSITIVE)
    {

        // For diagnostic session control, we need 7 bytes total:
        // 1 byte length + 1 byte SID + 1 byte subfunction + 4 bytes timing parameters
        response_frame.can_dlc = 7;
        response_frame.data[0] = response_frame.can_dlc - 1; // Dynamic length
        response_frame.data[1] = sid + 0x40;                 // Positive response SID
        response_frame.data[2] = subfunction;                // Echo the subfunction
        response_frame.data[3] = (g_session_info.p2_server_max >> 8) & 0xFF;
        response_frame.data[4] = g_session_info.p2_server_max & 0xFF;
        response_frame.data[5] = (g_session_info.p2_star_server_max >> 8) & 0xFF;
        response_frame.data[6] = g_session_info.p2_star_server_max & 0xFF;
        // Update the session if the response is positive
        if (sid == DIAGNOSTIC_SESSION_CONTROL)
        {
            updateSession(subfunction);
        }
    }
    else
    {
        response_frame.can_dlc = 4;
        response_frame.data[0] = response_frame.can_dlc - 1; // Response length
        response_frame.data[1] = 0x7F;                       // Negative response
        response_frame.data[2] = sid;                        // Service ID
        response_frame.data[3] = response_code;              // NRC
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

    return response_frame;
}

struct can_frame callProcess(struct can_frame *frame)
{ // identifyFrameType(frame);
    struct can_frame response_code;
    ProcessedFrame processedFrame = processCanFrame(frame);
    response_code = handleService(&processedFrame);
    return response_code;
}

int getSocket()
{
    // Assuming `sockfd` is the file descriptor for the CAN socket
    static int sockfd = -1; // Initialize to -1 to indicate that it's not yet set

    if (sockfd == -1)
    {
        // Create a socket
        sockfd = socket(PF_CAN, SOCK_RAW, CAN_RAW);
        if (sockfd < 0)
        {
            perror("socket");
            return -1;
        }

        // Specify the CAN interface
        struct ifreq ifr;
        strcpy(ifr.ifr_name, CAN_INTERFACE);
        if (ioctl(sockfd, SIOCGIFINDEX, &ifr) < 0)
        {
            perror("ioctl");
            close(sockfd);
            return -1;
        }

        struct sockaddr_can addr;
        addr.can_family = AF_CAN;
        addr.can_ifindex = ifr.ifr_ifindex;

        // Bind the socket to the CAN interface
        if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
        {
            perror("bind");
            close(sockfd);
            return -1;
        }
    }

    return sockfd;
}

int main()
{
    int sockfd;
    struct sockaddr_can addr;
    struct can_frame frame;
    struct ifreq ifr;
    // Set the socket to receive only messages from other nodes
    int recv_own_msgs = 0; // 0 = disabled (don't receive own messages)
    setsockopt(sockfd, SOL_CAN_RAW, CAN_RAW_RECV_OWN_MSGS, &recv_own_msgs, sizeof(recv_own_msgs));

    // Create a socket
    if ((sockfd = socket(PF_CAN, SOCK_RAW, CAN_RAW)) < 0)
    {
        perror("socket");
        return 1;
    }

    // Specify the CAN interface
    strcpy(ifr.ifr_name, CAN_INTERFACE);
    if (ioctl(sockfd, SIOCGIFINDEX, &ifr) < 0)
    {
        perror("ioctl");
        close(sockfd);
        return 1;
    }

    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;

    // Bind the socket to the CAN interface
    if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        perror("bind");
        close(sockfd);
        return 1;
    }

    printf("Server is running. Waiting for client messages...\n");
    initializeSession();
    while (1)
    {
        int nbytes = read(sockfd, &frame, sizeof(struct can_frame));
        if (nbytes > 0)
        {
            printf("Received frame from client. Processing...\n");

            // Call process and get the response frame
            struct can_frame response_frame = callProcess(&frame);
            // Send the response frame directly from main
            if (write(sockfd, &response_frame, sizeof(struct can_frame)) != sizeof(struct can_frame))
            {
                perror("Error sending response frame");
            }
            else
            {
                printf("Response frame sent: CAN ID: 0x%03X, Data: ", response_frame.can_id);
                for (int i = 0; i < response_frame.can_dlc; i++)
                {
                    printf("0x%02X ", response_frame.data[i]);
                }
                printf("\n");
            }
        }
        else if (nbytes == 0)
        {
            printf("End of file reached\n");
            break;
        }
        else
        {
            perror("read");
            break;
        }
        // sleep(10);
    }
    close(sockfd);
    return 0;
}
