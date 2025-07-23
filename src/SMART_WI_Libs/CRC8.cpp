/**
 * @file CRC8.cpp
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2024-10-11
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#include "CRC8.h"

CRC8_Class::CRC8_Class(){

}

CRC8_Class CRC8 = CRC8_Class::instance();

uint8_t CRC8_Class::Compute_CRC8(uint8_t bytes[], uint8_t numberOfBytes)
{
  //Serial3.println(F("CRC8 (uint8_t)"));
  uint8_t crc = {0};
  for (uint8_t i = {0}; i < numberOfBytes; i++)
  {
    /* XOR-in next input byte */
    uint8_t data = (uint8_t)(bytes[i] ^ crc);
    /* get current CRC value = remainder */
    crc = (uint8_t)(crctable[data]);
  }
  return crc;
}


template <typename T>
uint8_t CRC8_Class::Compute_CRC8(const T &data, uint8_t numberOfBytes)
{
  //Serial3.println(F("CRC8 T &data"));
  //uint8_t crc = {0};
  uint8_t bytes[numberOfBytes] = {0};
  memcpy(bytes, &data, numberOfBytes);
  // for (uint8_t i= 0; i < numberOfBytes; i++)
  // {
  //   Serial3.println(bytes[i], BIN);
  // }
  Serial3.println();
  
  return Compute_CRC8(bytes, numberOfBytes);
}

// Explicit template instantiations
template uint8_t CRC8_Class::Compute_CRC8<EEPROM_address>(const EEPROM_address&, unsigned char);
template uint8_t CRC8_Class::Compute_CRC8<unsigned char>(const unsigned char&, unsigned char);
template uint8_t CRC8_Class::Compute_CRC8<TelemetryData>(const TelemetryData&, unsigned char);
template uint8_t CRC8_Class::Compute_CRC8<unsigned long>(const unsigned long&, unsigned char);


void CRC8_Class::CalculateTable_CRC8()
{
  //const uint8_t generator = 0x1D;
  const uint8_t generator = 0x07;
  for (uint16_t i={0};i<=255;i++)
    crctable[i] = 0;
  /* iterate over all byte values 0 - 255 */
  for (uint16_t divident = 0; divident < 256; divident++)
  {
    uint8_t currByte = (uint8_t)divident;
    /* calculate the CRC-8 value for current byte */
    for (uint8_t j = {0}; j < 8; j++)
    {
      if ((currByte & 0x80) != 0)
      {
        currByte <<= 1;
        currByte ^= generator;
      }
      else
      {
        currByte <<= 1;
      }
    }
    /* store CRC value in lookup table */
    crctable[divident] = currByte;
  }
}