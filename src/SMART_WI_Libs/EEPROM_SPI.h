/**
 * @file EEPROM_SPI.h
 * @author Csaba Freiberger
 * @brief This class is for writing and reading on the external 25CSM04 EEPROM via SPI
 * @version 0.1
 * @date 2024-05-24
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#ifndef EEPROM_SPI_h
#define EEPROM_SPI_h

#include <Arduino.h>
#include <SPI.h>

const uint32_t MAX_25CSM04_ADDRESS = {524287};       //Maximum possible address of the whole EEPROM for an 8Bit variable
const uint8_t ADDRESS_PAGE_FLAG = {0};             //Address of flag, inlc. crc, which indicates in which page we are currently in
const uint8_t ADDRESS_SAVED_TELEM_FLAG = {2};   //Address of flag, inlc crc, which indicates if there are new saved tleemetry data on the EEPROM
const uint32_t ADDRESS_LAST_TELEM_ADDRESS_01 = {4}; //Address for saving the last telemetry address on the frist page
const uint32_t ADDRESS_FIRST_TELEM_01 = {10};         //First possible free address for telemetry data on the first page;



const uint16_t PAGE_SIZE = {52428};     //Max number of possible addresses in one page
const uint8_t MAX_PAGE_NUMBER = {10};   //Number of pages of the whole EEPROM


class EEPROM_SPI_Class
{
private:
    const uint8_t chipSelectE = PIN_PE3;


    const byte RDSR = 0x05; //Read STATUS Register
    const byte WREN = 0x06; //Set Write Enable
    const byte READ = 0x03; // Read from EEPROM Array
    const byte WRITE =  0x02; //Write to EEPROM Array (1 to 256 bytes)
    const byte WRBP = 0x08; //Write Ready/Busy Poll
    const byte WEL = 0x02;  //Write Enable Latch Bit
    const uint16_t pageSize = 256;  //Size of one page for writing

    /**
     * @brief Hide constructor in order to enforce a single instance of the
     * class.
     * 
     */
    EEPROM_SPI_Class(/* args */){};

public:
    /**
     * @brief  Singleton instance.
     * 
     * @return EEPPROM_SPI_Class& 
     */
    static EEPROM_SPI_Class& instance(void){
        static EEPROM_SPI_Class instance;
        return instance;
    }

    /**
     * @brief Destroy the eeprom spi class object
     * 
     */
    ~EEPROM_SPI_Class();

    /**
     * @brief Begins SPI communication between AVR128DB48 and the 25CSM04 EEPROM
     * 
     */
    void begin();

    /**
     * @return True if begin has already been called. 
     */
    bool isInitialized();

    /**
     * @brief Ends SPI communication between AVR128DB48 and the 25CSM04 EEPROM
     * 
     */
    void end();

    /**
    * @brief Reads the status register of the external EEPROM
    * 
    * @return uint8_t 
    */
    uint16_t readEEPROMStatusRegister();

    // uint8_t readBusyStatusLatch()();

    /**                                                                             ___
    * @brief Checks if EEPROM is busy with an internal write cycle, by checking the RDY/BSY status register
     * 
   * @return true if busy
    * @return false if ready to get new instructions
    */
    bool EEPROMisBusy();

    /**
     * @brief Chekst if EEPROM is enabled for writing, by ckecking the WEL status register
     * 
     * @return true 
     * @return false 
     */
    bool isWriteEnabled();

    /**
    * @brief Write data of arbitary variable type on the external EEPROM
    * 
    * @param address where the date should be written
    * @param buf pointer of data
    * @param sizeBuf Size of data, MAX 256Bytes
    */
    void writeExternEEPROM(uint32_t address, const uint8_t *buf, uint16_t sizeBuf);

    /**
    * @brief Read data of arbitary variable type from external EEPROM
    * 
    * @param address from the data should be read
    * @param buf pointer of buffer for the copied data
    * @param sizeBuf Size of buffer where data should be copied
    */
    void readExternEEPROM(uint32_t address, uint8_t *buf, uint8_t sizeBuf);

    template <typename T>
    const T &putEEPROMData(uint32_t address, const T &t){
        const uint8_t *data = (const uint8_t *)&t;
        writeExternEEPROM(address, data, sizeof(T)); 
        delay(5);
        return t;
    }

    template <typename T>
    T &getEEPROMData(uint32_t address, T &t){
        uint8_t *ptrValue = (uint8_t *)&t;
        readExternEEPROM(address, ptrValue, sizeof(T));
        delay(5);
        return t;
    }

};

extern EEPROM_SPI_Class EEPROM_SPI;

#endif
