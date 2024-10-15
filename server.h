#ifndef SERVER_H
#define SERVER_H

#include <stdint.h>
#include <linux/can.h>
#include "services.h"

// Define CAN_INTERFACE as a preprocessor macro if needed across files
#define CAN_INTERFACE "vcan0"  // Replace with your CAN interface name


// Function declarations
void send_can_message(int sockfd, uint32_t can_id, uint8_t *data);
FrameType identify_frame_type(struct can_frame *frame);
void extract_first_frame_data(struct can_frame *frame, uint16_t *total_length, uint8_t *SID, uint8_t *subfunction);
void extract_consecutive_frame_data(struct can_frame *frame, uint8_t *data_buffer, uint8_t sequence_number, int *data_index);
ProcessedFrame processCanFrame(struct can_frame *frame);
void sendPossRes(int sockfd, ServiceState sid, struct can_frame *processedFrame);//void findService(uint8_t SID);
int getSocket() ;
#endif // SERVER_H
