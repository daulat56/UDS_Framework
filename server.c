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
#include "services.h"
#include "server.h"

#define CAN_INTERFACE "vcan0"  // Replace with your CAN interface name

FrameType identifyFrameType(struct can_frame *frame) {
    uint8_t pci_type = (frame->data[0] & 0xF0) >> 4;

    switch (pci_type) {
        case 0x00:
            return SINGLE_FRAME;
        case 0x01:
            return MULTI_FRAME;
        default:
            return -1; 
    }
}


/* Function to extract data from the first frame of a multi-frame message */
void extract_first_frame_data(struct can_frame *frame, uint16_t *total_length, uint8_t *SID, uint8_t *subfunction) {
    *total_length = ((frame->data[0] & 0x0F) << 8) | frame->data[1];  // Extract total length
    *SID = frame->data[2];                                            // Extract SID
    *subfunction = frame->data[3];                                    // Extract subfunction
}

/* Function to handle consecutive frames */
void extract_consecutive_frame_data(struct can_frame *frame, uint8_t *data_buffer, uint8_t sequence_number, int *data_index) {

    /*Copy the data from the consecutive frame into the data buffer */
    memcpy(&data_buffer[*data_index], &frame->data[1], 7);
    *data_index += 7;
}


/* Main processing function */
ProcessedFrame processCanFrame(struct can_frame *frame) {
    ProcessedFrame result = {0};  // Initialize all fields to 0
    FrameType frame_type = identifyFrameType(frame);
    
    if (frame_type == SINGLE_FRAME) {
        result.sid = (ServiceState)frame->data[1];
        result.subfunction = (SubFunctionType)frame->data[2];
        result.data_length = frame->data[0] & 0x0F;
        result.sequence_number = 0;
        printf("Single Frame detected\n");
        printf("SID: 0x%02X, SubFunction: 0x%02X, Data Length: %d\n", result.sid, result.subfunction, result.data_length);
    } else if (frame_type == MULTI_FRAME) {
        
        printf("Multi-Frame detected\n");
        uint16_t total_length;
        uint8_t SID, subfunction;
        uint8_t data_buffer[4096];  // Adjust size as necessary
        int data_index = 0;

        if ((frame->data[0] & 0xF0) >> 4 == 0x01) {
            extract_first_frame_data(frame, &total_length, &SID, &subfunction);
            result.sid = (ServiceState)SID;
            result.subfunction = (SubFunctionType)subfunction;
            printf("First Frame detected\n");
            printf("Total Length: %d, SID: 0x%02X, SubFunction: 0x%02X\n", total_length, SID, subfunction);
            data_index = 6;  // First 6 bytes are part of the first frame's data
        } else if ((frame->data[0] & 0xF0) >> 4 == 0x02) {
            result.sequence_number = frame->data[0] & 0x0F;
            extract_consecutive_frame_data(frame, data_buffer, result.sequence_number, &data_index);
            printf("Consecutive Frame detected with sequence number: %d\n", result.sequence_number);
        }
    }

    return result;
}

void handleService(ProcessedFrame *processedFrame) {
    UDS_Table udsServiceTable={};
    getUDSTable(&udsServiceTable);
    printf("size of table : %d\n",udsServiceTable.size);
    for (int i = 0; i < udsServiceTable.size; i++) {
        printf("the SID is : %d \n",udsServiceTable.row[i].service_id );
       if (udsServiceTable.row[i].service_id == processedFrame->sid) {
            uint32_t response_code = udsServiceTable.row[i].service_handler(processedFrame);
            printf("Response Code: 0x%02X\n", response_code);
            return;
        }
        
    }
    printf("Unknown or unsupported SID: 0x%02X\n", processedFrame->sid);
}

void callProcess(struct can_frame *frame)
{
    identifyFrameType(frame);
    ProcessedFrame processedFrame = processCanFrame(frame);
    handleService(&processedFrame);
}



int getSocket() {
    // Assuming `sockfd` is the file descriptor for the CAN socket
    static int sockfd = -1; // Initialize to -1 to indicate that it's not yet set

    if (sockfd == -1) {
        // Create a socket
        sockfd = socket(PF_CAN, SOCK_RAW, CAN_RAW);
        if (sockfd < 0) {
            perror("socket");
            return -1;
        }

        // Specify the CAN interface
        struct ifreq ifr;
        strcpy(ifr.ifr_name, CAN_INTERFACE);
        if (ioctl(sockfd, SIOCGIFINDEX, &ifr) < 0) {
            perror("ioctl");
            close(sockfd);
            return -1;
        }

        struct sockaddr_can addr;
        addr.can_family = AF_CAN;
        addr.can_ifindex = ifr.ifr_ifindex;

        // Bind the socket to the CAN interface
        if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
            perror("bind");
            close(sockfd);
            return -1;
        }
    }

    return sockfd;
}



int main() {
    int sockfd;
    struct sockaddr_can addr;
    struct can_frame frame;
    struct ifreq ifr;

    // Create a socket
    if ((sockfd = socket(PF_CAN, SOCK_RAW, CAN_RAW)) < 0) {
        perror("socket");
        return 1;
    }

    // Specify the CAN interface
    strcpy(ifr.ifr_name, CAN_INTERFACE);
    if (ioctl(sockfd, SIOCGIFINDEX, &ifr) < 0) {
        perror("ioctl");
        close(sockfd);
        return 1;
    }

    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;

    // Bind the socket to the CAN interface
    if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind");
        close(sockfd);
        return 1;
    }

    while (1) {
        // Read the CAN frame
        int nbytes = read(sockfd, &frame, sizeof(struct can_frame));
        if (nbytes > 0) {
            // Process the incoming CAN frame
            callProcess(&frame);
        } else {
            perror("read");
        }
    }

    close(sockfd);
    return 0;
}
