#include <stdio.h>
#include <stdlib.h>
#include<stdint.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if.h>
#include <linux/can.h>
#include <linux/can/raw.h>

#define CAN_INTERFACE "vcan0"  // Replace with your CAN interface name

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

int main() {
    int sockfd;
    struct sockaddr_can addr;
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

    // Example data frame (change as needed)
    //uint8_t data[8] = {0x02, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00};  // {Length, Session, subfunction}/
   //uint8_t data[8] = {0x10, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07};  // Total data length = 0x14 (20 bytes)/
   uint8_t data[8]={0x02,0x11,0x02,0x02,0x00,0x00,0x00,0x00};


    // Send the message
    send_can_message(sockfd, 0x123, data);

    // Wait for response
    struct can_frame response;
    int nbytes = read(sockfd, &response, sizeof(response));
    if (nbytes > 0) {
        printf("Received response: ");
        for (int i = 0; i < response.can_dlc; i++) {
            printf("0x%02X ", response.data[i]);
        }
        printf("\n");
    } else {
        perror("read");
    }

    close(sockfd);
    return 0;
}
