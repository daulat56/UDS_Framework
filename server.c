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

// Structure to hold parsed information
typedef struct {
    uint8_t data_length;
    ServiceState sid;
    SubFunctionType subfunction;
    uint8_t data[255];
} UDSFrameInfo;

//will have to write the sending possitive response to sender 

// Function to send CAN message
void send_can_message(int sockfd, uint32_t can_id, uint8_t *data) {
    struct can_frame frame;
    frame.can_id = can_id;
    frame.can_dlc = 8;
    memcpy(frame.data, data, 8);

    if (write(sockfd, &frame, sizeof(frame)) != sizeof(frame)) {
        perror("write");
        close(sockfd);
        exit(1);
    }

    printf("Sent CAN message: ");
    for (int i = 0; i < 8; i++) {
        printf("0x%02X ", frame.data[i]);
    }
    printf("\n");
}

FrameType identify_frame_type(struct can_frame *frame) {
    uint8_t pci_type = (frame->data[0] & 0xF0) >> 4;

    switch (pci_type) {
        case 0x00:
            //printf("Single frame received\n");
            return SINGLE_FRAME;
        case 0x01:
            //printf("First frame of a multi-frame received\n");
            return MULTI_FRAME;
        default:
            //printf("Unknown frame type received\n");
            return -1; // Unknown frame type
    }
}


// Function to extract data from the first frame of a multi-frame message
void extract_first_frame_data(struct can_frame *frame, uint16_t *total_length, uint8_t *SID, uint8_t *subfunction) {
    *total_length = ((frame->data[0] & 0x0F) << 8) | frame->data[1];  // Extract total length
    *SID = frame->data[2];                                            // Extract SID
    *subfunction = frame->data[3];                                    // Extract subfunction
}

// Function to handle consecutive frames
void extract_consecutive_frame_data(struct can_frame *frame, uint8_t *data_buffer, uint8_t sequence_number, int *data_index) {
    // Copy the data from the consecutive frame into the data buffer
    memcpy(&data_buffer[*data_index], &frame->data[1], 7);
    *data_index += 7;
}

// Main processing function
void process_can_frame(int sockfd,struct can_frame *frame) {
    FrameType frame_type = identify_frame_type(frame);
    
    if (frame_type == SINGLE_FRAME) {
        uint8_t SID = frame->data[1];
        uint8_t subfunction = frame->data[2];
        uint8_t data_length = frame->data[0] & 0x0F;

        printf("Single Frame detected\n");
        printf("SID: 0x%02X, SubFunction: 0x%02X, Data Length: %d\n", SID, subfunction, data_length);
    } else if (frame_type == MULTI_FRAME) {
        
        printf("Multi-Frame detected\n");
        uint16_t total_length;
        uint8_t SID, subfunction;
        uint8_t data_buffer[4096];  // Adjust size as necessary
        int data_index = 0;

        if ((frame->data[0] & 0xF0) >> 4 == 0x01) {
            extract_first_frame_data(frame, &total_length, &SID, &subfunction);
            printf("First Frame detected\n");
            printf("Total Length: %d, SID: 0x%02X, SubFunction: 0x%02X\n", total_length, SID, subfunction);
            data_index = 4;  // First 6 bytes are part of the first frame's data
        } else if ((frame->data[0] & 0xF0) >> 4 == 0x02) {
            uint8_t sequence_number = frame->data[0] & 0x0F;
            extract_consecutive_frame_data(frame, data_buffer, sequence_number, &data_index);
            printf("Consecutive Frame detected with sequence number: %d\n", sequence_number);
        }
        
    }
}

void findService(uint8_t SID)
{
    struct can_frame *frame;
    if(SID==0x10)
    {
        diagnosticControl(&frame);
    }
}

int main() {
    int sockfd;
    struct sockaddr_can addr;
    struct ifreq ifr;
    struct can_frame frame;

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

    // Listen for CAN messages
    while (1) {
        int nbytes = read(sockfd, &frame, sizeof(frame));
        if (nbytes > 0) {
            printf("Received CAN message with ID: 0x%X\n", frame.can_id);
            printf("Data: ");
            for (int i = 0; i < frame.can_dlc; i++) {
                printf("0x%02X ", frame.data[i]);
            }
            printf("\n");

            FrameType type = identify_frame_type(&frame);

            //to check the processed frame
            process_can_frame(sockfd, &frame);
        } else {
            perror("read");
        }
    }

    close(sockfd);
    return 0;
}
