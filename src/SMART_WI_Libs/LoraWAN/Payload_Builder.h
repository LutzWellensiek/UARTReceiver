/**
 * @file Payload_Builder.h
 * @brief Simple LoRaWAN payload builder and decoder
 * @author Smart Wire Industries
 * @version 1.0.0
 * @date 2025-01-17
 */

#ifndef PAYLOAD_BUILDER_H
#define PAYLOAD_BUILDER_H

#include "Arduino.h"

// Special value to indicate "not used"
#define NO_VALUE NAN

// TLV Tags for different sensor types
#define TAG_TEMPERATURE  0x01
#define TAG_DEFLECTION   0x02
#define TAG_PRESSURE     0x03
#define TAG_MISC         0x04

/**
 * Builds a payload from individual sensor values
 * Only adds values that are not NAN to the payload
 * @param buffer Output buffer for payload
 * @param bufferSize Size of output buffer
 * @param temp1 Temperature value 1 (°C) - use NO_VALUE to skip
 * @param temp2 Temperature value 2 (°C) - use NO_VALUE to skip
 * @param temp3 Temperature value 3 (°C) - use NO_VALUE to skip
 * @param temp4 Temperature value 4 (°C) - use NO_VALUE to skip
 * @param defl1 Deflection value 1 (mm) - use NO_VALUE to skip
 * @param defl2 Deflection value 2 (mm) - use NO_VALUE to skip
 * @param defl3 Deflection value 3 (mm) - use NO_VALUE to skip
 * @param press1 Pressure value 1 (hPa) - use NO_VALUE to skip
 * @param press2 Pressure value 2 (hPa) - use NO_VALUE to skip
 * @param misc1 Miscellaneous value 1 - use NO_VALUE to skip
 * @param misc2 Miscellaneous value 2 - use NO_VALUE to skip
 * @return Size of created payload in bytes
 */
struct PayloadResult {
    uint8_t* buffer;
    size_t size;
};

PayloadResult buildPayload(uint8_t* buffer, size_t bufferSize,
                   float temp1 = NO_VALUE, float temp2 = NO_VALUE, 
                   float temp3 = NO_VALUE, float temp4 = NO_VALUE,
                   float defl1 = NO_VALUE, float defl2 = NO_VALUE, float defl3 = NO_VALUE,
                   float press1 = NO_VALUE, float press2 = NO_VALUE,
                   float misc1 = NO_VALUE, float misc2 = NO_VALUE);

/**
 * Decodes a payload and prints values to Serial
 * @param payload Payload bytes to decode
 * @param size Size of payload in bytes
 */
void decodePayload(const uint8_t* payload, size_t size);

/**
 * Prints payload as hex string for debugging
 * @param payload Payload bytes
 * @param size Size of payload in bytes
 */
void printPayloadHex(const uint8_t* payload, size_t size);

#endif // PAYLOAD_BUILDER_H
