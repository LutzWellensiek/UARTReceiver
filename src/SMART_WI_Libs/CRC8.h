/**
 * @file CRC8.h
 * @author Csaba Freiberger (csaba.freiberger@witzenmann.com)
 * @brief 
 * @version 0.1
 * @date 2024-10-09
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#ifndef CRC8_h
#define CRC8_h

  #include <Arduino.h>

  struct EEPROM_address;
  struct TelemetryData;


  class CRC8_Class
  {
  private:
    /* data */
    CRC8_Class(/* args */);
    //array for crc table
    int crctable[256];
  public:
    static CRC8_Class& instance(void){
      static CRC8_Class instance;
      return instance;
    }
    //~CRC8();
    
  /**
   * @brief Function for generating the CRC table (has to be executed only once at the start of the program)
   * 
  */
  void CalculateTable_CRC8();

  /**
   * @brief Function for calculate the checksum from the RS485/EEPROM
   * 
   * @param bytes received data
   * @param numberOfBytes Number of Bytes per frame
   * @return uint8_t CRC checksum
  */
  uint8_t Compute_CRC8(uint8_t bytes[], uint8_t numberOfBytes);

  /**
  * @brief Function for calculating the checksum from the RS485/EEPROM
  * 
  * @tparam T arbitary data Type
  * @param data received data
  * @param numberOfBytes Number of Bytes per frame
  * @return uint8_t CRC checksum
  */
  template <typename T>
  uint8_t Compute_CRC8(const T &data, uint8_t numberOfBytes);
  };
  
  extern CRC8_Class CRC8;

#endif