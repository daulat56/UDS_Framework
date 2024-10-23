#ifndef ECURESET_H
#define ECURESET_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
// Add this new enum for ECU_RESET subfunctions
typedef enum {
    HARD_RESET = 0x01,
    KEY_OFF_ON_RESET = 0x02,
    SOFT_RESET = 0x03,
    // Add other reset types as needed
} ECUResetType;

uint32_t ecuReset(void* frame);

#endif