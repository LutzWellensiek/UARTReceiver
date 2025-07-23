/**
 * @file ecc608.h
 * @brief Stub for ECC608 crypto chip library
 * This is a minimal stub to allow compilation when ECC608 library is not available
 */

#ifndef ECC608_H
#define ECC608_H

#include <stdint.h>

// ATCA Status codes
typedef uint8_t ATCA_STATUS;
#define ATCA_SUCCESS 0x00

// Function declarations (stubs)
static inline ATCA_STATUS atcab_read_serial_number(uint8_t* serial_number) {
    // Return dummy serial number
    for(int i = 0; i < 9; i++) {
        serial_number[i] = i;
    }
    return ATCA_SUCCESS;
}

// ECC608 class stub
class ECC608Class {
public:
    ATCA_STATUS begin() { return ATCA_SUCCESS; }
};

extern ECC608Class ECC608;

#endif // ECC608_H
